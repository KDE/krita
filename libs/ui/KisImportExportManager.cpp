/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisImportExportManager.h"

#include <QDir>
#include <QFile>
#include <QLabel>
#include <QVBoxLayout>
#include <QList>
#include <QApplication>
#include <QByteArray>
#include <QPluginLoader>
#include <QFileInfo>
#include <QMessageBox>
#include <QJsonObject>
#include <QTextBrowser>
#include <QCheckBox>
#include <QSaveFile>
#include <QGroupBox>
#include <QFuture>
#include <QtConcurrent>

#include <klocalizedstring.h>
#include <ksqueezedtextlabel.h>
#include <kpluginfactory.h>

#include <KisMimeDatabase.h>
#include <KisPart.h>
#include <KisPopupButton.h>
#include <KisPreExportChecker.h>
#include <KisUsageLogger.h>
#include <KoColorProfile.h>
#include <KoColorProfileConstants.h>
#include <KoDialog.h>
#include <KoFileDialog.h>
#include <KoJsonTrader.h>
#include <KoProgressUpdater.h>
#include <kis_assert.h>
#include <kis_config_widget.h>
#include <kis_debug.h>
#include <kis_icon_utils.h>
#include <kis_image.h>
#include <kis_iterator_ng.h>
#include <kis_layer_utils.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>

#include "KisDocument.h"
#include "KisImportExportErrorCode.h"
#include "KisImportExportFilter.h"
#include "KisMainWindow.h"
#include "KisReferenceImagesLayer.h"
#include "imagesize/dlg_imagesize.h"
#include "kis_async_action_feedback.h"
#include "kis_config.h"
#include "kis_grid_config.h"
#include "kis_guides_config.h"
#include <kis_adjustment_layer.h>
#include <kis_filter_mask.h>

#include <KisImportUserFeedbackInterface.h>

// static cache for import and export mimetypes
QStringList KisImportExportManager::m_importMimeTypes;
QStringList KisImportExportManager::m_exportMimeTypes;

namespace {
struct SynchronousUserFeedbackInterface : KisImportUserFeedbackInterface
{
    SynchronousUserFeedbackInterface(QWidget *parent, bool batchMode)
        : m_parent(parent)
        , m_batchMode(batchMode)
    {
    }

    Result askUser(AskCallback callback) override
    {
        if (m_batchMode) return SuppressedByBatchMode;
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_parent, SuppressedByBatchMode);

        return callback(m_parent) ? Success : UserCancelled;
    }

    QWidget *m_parent {nullptr};
    bool m_batchMode {false};
};

}


class Q_DECL_HIDDEN KisImportExportManager::Private
{
public:
    KoUpdaterPtr updater;

    QString cachedExportFilterMimeType;
    QSharedPointer<KisImportExportFilter> cachedExportFilter;
};

struct KisImportExportManager::ConversionResult {
    ConversionResult()
    {
    }

    ConversionResult(const QFuture<KisImportExportErrorCode> &futureStatus)
        : m_isAsync(true),
          m_futureStatus(futureStatus)
    {
    }

    ConversionResult(KisImportExportErrorCode status)
        : m_isAsync(false),
          m_status(status)
    {
    }

    bool isAsync() const {
        return m_isAsync;
    }

    QFuture<KisImportExportErrorCode> futureStatus() const {
        // if the result is not async, then it means some failure happened,
        // just return a cancelled future
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_isAsync || !m_status.isOk());

        return m_futureStatus;
    }

    KisImportExportErrorCode status() const {
        return m_status;
    }

    void setStatus(KisImportExportErrorCode value) {
        m_status = value;
    }
private:
    bool m_isAsync = false;
    QFuture<KisImportExportErrorCode> m_futureStatus;
    KisImportExportErrorCode m_status = ImportExportCodes::InternalError;
};


KisImportExportManager::KisImportExportManager(KisDocument* document)
    : m_document(document)
    , d(new Private)
{
}

KisImportExportManager::~KisImportExportManager()
{
    delete d;
}

KisImportExportErrorCode KisImportExportManager::importDocument(const QString& location, const QString& mimeType)
{
    ConversionResult result = convert(Import, location, location, mimeType, false, 0, false);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!result.isAsync(), ImportExportCodes::InternalError);

    return result.status();
}

KisImportExportErrorCode KisImportExportManager::exportDocument(const QString& location, const QString& realLocation, const QByteArray& mimeType, bool showWarnings, KisPropertiesConfigurationSP exportConfiguration, bool isAdvancedExporting)
{
    ConversionResult result = convert(Export, location, realLocation, mimeType, showWarnings, exportConfiguration, false, isAdvancedExporting);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!result.isAsync(), ImportExportCodes::InternalError);

    return result.status();
}

QFuture<KisImportExportErrorCode> KisImportExportManager::exportDocumentAsync(const QString &location, const QString &realLocation, const QByteArray &mimeType,
                                                                            KisImportExportErrorCode &status, bool showWarnings, KisPropertiesConfigurationSP exportConfiguration, bool isAdvancedExporting)
{
    ConversionResult result = convert(Export, location, realLocation, mimeType, showWarnings, exportConfiguration, true, isAdvancedExporting);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(result.isAsync() ||
                                         !result.status().isOk(), QFuture<KisImportExportErrorCode>());

    status = result.status();
    return result.futureStatus();
}

// The static method to figure out to which parts of the
// graph this mimetype has a connection to.
QStringList KisImportExportManager::supportedMimeTypes(Direction direction)
{
    // Find the right mimetype by the extension
    QSet<QString> mimeTypes;
    //    mimeTypes << KisDocument::nativeFormatMimeType() << "application/x-krita-paintoppreset" << "image/openraster";

    if (direction == KisImportExportManager::Import) {
        if (m_importMimeTypes.isEmpty()) {
            QList<KoJsonTrader::Plugin> list = KoJsonTrader::instance()->query("Krita/FileFilter", "");
            Q_FOREACH(const KoJsonTrader::Plugin &loader, list) {
                QJsonObject json = loader.metaData().value("MetaData").toObject();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
                Q_FOREACH(const QString &mimetype, json.value("X-KDE-Import").toString().split(",", Qt::SkipEmptyParts)) {
#else
                Q_FOREACH(const QString &mimetype, json.value("X-KDE-Import").toString().split(",", QString::SkipEmptyParts)) {
#endif

                    //qDebug() << "Adding  import mimetype" << mimetype << KisMimeDatabase::descriptionForMimeType(mimetype) << "from plugin" << loader;
                    mimeTypes << mimetype;
                }
            }
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
            m_importMimeTypes = QList<QString>(mimeTypes.begin(), mimeTypes.end());
#else
            m_importMimeTypes = QList<QString>::fromSet(mimeTypes);
#endif
        }
        return m_importMimeTypes;
    }
    else if (direction == KisImportExportManager::Export) {
        if (m_exportMimeTypes.isEmpty()) {
            QList<KoJsonTrader::Plugin> list = KoJsonTrader::instance()->query("Krita/FileFilter", "");
            Q_FOREACH(const KoJsonTrader::Plugin &loader, list) {
                QJsonObject json = loader.metaData().value("MetaData").toObject();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
                Q_FOREACH(const QString &mimetype, json.value("X-KDE-Export").toString().split(",", Qt::SkipEmptyParts)) {
#else
                Q_FOREACH(const QString &mimetype, json.value("X-KDE-Export").toString().split(",", QString::SkipEmptyParts)) {
#endif

                    //qDebug() << "Adding  export mimetype" << mimetype << KisMimeDatabase::descriptionForMimeType(mimetype) << "from plugin" << loader;
                    mimeTypes << mimetype;
                }
            }
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
            m_exportMimeTypes = QList<QString>(mimeTypes.begin(), mimeTypes.end());
#else
           m_exportMimeTypes = QList<QString>::fromSet(mimeTypes);
#endif

        }
        return m_exportMimeTypes;
    }
    return QStringList();
}

KisImportExportFilter *KisImportExportManager::filterForMimeType(const QString &mimetype, KisImportExportManager::Direction direction)
{
    int weight = -1;
    KisImportExportFilter *filter = 0;
    QList<KoJsonTrader::Plugin>list = KoJsonTrader::instance()->query("Krita/FileFilter", "");

    Q_FOREACH(const KoJsonTrader::Plugin &loader, list) {
        QJsonObject json = loader.metaData().value("MetaData").toObject();
        QString directionKey = direction == Export ? "X-KDE-Export" : "X-KDE-Import";

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        if (json.value(directionKey).toString().split(",", Qt::SkipEmptyParts).contains(mimetype)) {
#else
        if (json.value(directionKey).toString().split(",", QString::SkipEmptyParts).contains(mimetype)) {
#endif

            KLibFactory *factory = qobject_cast<KLibFactory *>(loader.instance());

            if (!factory) {
                warnUI << loader.errorString();
                continue;
            }

            QObject* obj = factory->create<KisImportExportFilter>(0);
            if (!obj || !obj->inherits("KisImportExportFilter")) {
                delete obj;
                continue;
            }

            KisImportExportFilter *f = qobject_cast<KisImportExportFilter*>(obj);
            if (!f) {
                delete obj;
                continue;
            }

            KIS_ASSERT_RECOVER_NOOP(json.value("X-KDE-Weight").isDouble());

            int w = json.value("X-KDE-Weight").toInt();

            if (w > weight) {
                delete filter;
                filter = f;
                f->setObjectName(loader.fileName());
                weight = w;
            }
        }
    }

    if (filter) {
        filter->setMimeType(mimetype);
    }
    return filter;
}

bool KisImportExportManager::batchMode(void) const
{
    return m_document->fileBatchMode();
}

void KisImportExportManager::setUpdater(KoUpdaterPtr updater)
{
    d->updater = updater;
}

QString KisImportExportManager::askForAudioFileName(const QString &defaultDir, QWidget *parent)
{
    KoFileDialog dialog(parent, KoFileDialog::ImportFiles, "ImportAudio");

    if (!defaultDir.isEmpty()) {
        dialog.setDefaultDir(defaultDir);
    }

    QStringList mimeTypes;
    mimeTypes << "audio/mpeg";
    mimeTypes << "audio/ogg";
    mimeTypes << "audio/vorbis";
    mimeTypes << "audio/vnd.wave";
    mimeTypes << "audio/flac";

    dialog.setMimeTypeFilters(mimeTypes);
    dialog.setCaption(i18nc("@title:window", "Open Audio"));

    return dialog.filename();
}

QString KisImportExportManager::getUriForAdditionalFile(const QString &defaultUri, QWidget *parent)
{
    KoFileDialog dialog(parent, KoFileDialog::SaveFile, "Save Kra");

    KIS_SAFE_ASSERT_RECOVER_NOOP(!defaultUri.isEmpty());

    dialog.setDirectoryUrl(QUrl(defaultUri));
    dialog.setMimeTypeFilters(QStringList("application/x-krita"));

    return dialog.filename();
}

KisImportExportManager::ConversionResult KisImportExportManager::convert(KisImportExportManager::Direction direction, const QString &location, const QString& realLocation, const QString &mimeType, bool showWarnings, KisPropertiesConfigurationSP exportConfiguration, bool isAsync, bool isAdvancedExporting)
{
    // export configuration is supported for export only
    KIS_SAFE_ASSERT_RECOVER_NOOP(direction == Export || !bool(exportConfiguration));

    QString typeName = mimeType;
    if (typeName.isEmpty()) {
        typeName = KisMimeDatabase::mimeTypeForFile(location, direction == KisImportExportManager::Export ? false : true);
    }

    QSharedPointer<KisImportExportFilter> filter;

    /**
     * Fetching a filter from the registry is a really expensive operation,
     * because it blocks all the threads. Cache the filter if possible.
     */
    if (direction == KisImportExportManager::Export &&
            d->cachedExportFilter &&
            d->cachedExportFilterMimeType == typeName) {

        filter = d->cachedExportFilter;
    } else {

        filter = toQShared(filterForMimeType(typeName, direction));

        if (direction == Export) {
            d->cachedExportFilter = filter;
            d->cachedExportFilterMimeType = typeName;
        }
    }

    if (!filter) {
        return KisImportExportErrorCode(ImportExportCodes::FileFormatNotSupported);
    }

    filter->setFilename(location);
    filter->setRealFilename(realLocation);
    filter->setBatchMode(batchMode());
    filter->setMimeType(typeName);

    if (direction == Import) {
        KisMainWindow *kisMain = KisPart::instance()->currentMainwindow();
        filter->setImportUserFeedBackInterface(new SynchronousUserFeedbackInterface(kisMain, batchMode()));
    }

    if (!d->updater.isNull()) {
        // WARNING: The updater is not guaranteed to be persistent! If you ever want
        // to add progress reporting to "Save also as .kra", make sure you create
        // a separate KoProgressUpdater for that!

        // WARNING2: the failsafe completion of the updater happens in the destructor
        // the filter.

        filter->setUpdater(d->updater);
    }

    QByteArray from, to;
    if (direction == Export) {
        from = m_document->nativeFormatMimeType();
        to = typeName.toLatin1();
    }
    else {
        from = typeName.toLatin1();
        to = m_document->nativeFormatMimeType();
    }

    KIS_ASSERT_RECOVER_RETURN_VALUE(
                direction == Import || direction == Export,
                KisImportExportErrorCode(ImportExportCodes::InternalError)); // "bad conversion graph"

    ConversionResult result = KisImportExportErrorCode(ImportExportCodes::OK);
    if (direction == Import) {

        KisUsageLogger::log(QString("Importing %1 to %2. Location: %3. Real location: %4. Batchmode: %5")
                                .arg(QString::fromLatin1(from), QString::fromLatin1(to), location,
                                     realLocation, QString::number(batchMode())));

        // async importing is not yet supported!
        KIS_SAFE_ASSERT_RECOVER_NOOP(!isAsync);

        // FIXME: Dmitry says "this progress reporting code never worked. Initial idea was to implement it his way, but I stopped and didn't finish it"
        if (0 && !batchMode()) {
            KisAsyncActionFeedback f(i18n("Opening document..."), 0);
            result = f.runAction(std::bind(&KisImportExportManager::doImport, this, location, filter));
        } else {
            result = doImport(location, filter);
        }
        if (result.status().isOk()) {
            KisImageSP image = m_document->image().toStrongRef();
            if (image) {
                KIS_SAFE_ASSERT_RECOVER(image->colorSpace() != nullptr && image->colorSpace()->profile() != nullptr)
                {
                    qWarning() << "Loaded a profile-less file without a fallback. Rejecting image "
                                  "opening";
                    return KisImportExportErrorCode(ImportExportCodes::InternalError);
                }
                KisUsageLogger::log(QString("Loaded image from %1. Size: %2 * %3 pixels, %4 dpi. Color "
                                            "model: %6 %5 (%7). Layers: %8")
                                        .arg(QString::fromLatin1(from), QString::number(image->width()),
                                             QString::number(image->height()), QString::number(image->xRes()),
                                             image->colorSpace()->colorModelId().name(),
                                             image->colorSpace()->colorDepthId().name(),
                                             image->colorSpace()->profile()->name(),
                                             QString::number(image->nlayers())));
            } else {
                qWarning() << "The filter returned OK, but there is no image";
            }

        }
        else {
            KisUsageLogger::log(QString("Failed to load image from %1").arg(QString::fromLatin1(from)));
        }

    }
    else /* if (direction == Export) */ {
        if (!exportConfiguration) {
            exportConfiguration = filter->lastSavedConfiguration(from, to);
        }

        if (exportConfiguration) {
            fillStaticExportConfigurationProperties(exportConfiguration);
        }

        bool alsoAsKra = false;
        bool askUser = askUserAboutExportConfiguration(filter, exportConfiguration,
                                                       from, to,
                                                       batchMode(), showWarnings,
                                                       &alsoAsKra, isAdvancedExporting);


        if (!batchMode() && !askUser) {
            return KisImportExportErrorCode(ImportExportCodes::Cancelled);
        }

        KisUsageLogger::log(
            QString(
                "Converting from %1 to %2. Location: %3. Real location: %4. Batchmode: %5. Configuration: %6")
                .arg(QString::fromLatin1(from), QString::fromLatin1(to), location, realLocation,
                     QString::number(batchMode()),
                     (exportConfiguration ? exportConfiguration->toXML() : "none")));

        const QString alsoAsKraLocation = alsoAsKra ? getAlsoAsKraLocation(location) : QString();
        if (isAsync) {
            result = QtConcurrent::run(std::bind(&KisImportExportManager::doExport, this, location, filter,
                                                 exportConfiguration, alsoAsKraLocation));

            // we should explicitly report that the exporting has been initiated
            result.setStatus(ImportExportCodes::OK);

        } else if (!batchMode()) {
            KisAsyncActionFeedback f(i18n("Saving document..."), 0);
            result = f.runAction(std::bind(&KisImportExportManager::doExport, this, location, filter,
                                           exportConfiguration, alsoAsKraLocation));
        } else {
            result = doExport(location, filter, exportConfiguration, alsoAsKraLocation);
        }

        if (exportConfiguration && !batchMode()) {
            KisConfig(false).setExportConfiguration(typeName, exportConfiguration);
        }
    }
    return result;
}

void KisImportExportManager::fillStaticExportConfigurationProperties(KisPropertiesConfigurationSP exportConfiguration, KisImageSP image)
{
    KisPaintDeviceSP dev = image->projection();
    const KoColorSpace* cs = dev->colorSpace();
    const bool isThereAlpha =
            KisPainter::checkDeviceHasTransparency(image->projection());

    exportConfiguration->setProperty(KisImportExportFilter::ImageContainsTransparencyTag, isThereAlpha);
    exportConfiguration->setProperty(KisImportExportFilter::ColorModelIDTag, cs->colorModelId().id());
    exportConfiguration->setProperty(KisImportExportFilter::ColorDepthIDTag, cs->colorDepthId().id());

    const bool sRGB =
            (cs->profile()->name().contains(QLatin1String("srgb"), Qt::CaseInsensitive) &&
             !cs->profile()->name().contains(QLatin1String("g10")));
    exportConfiguration->setProperty(KisImportExportFilter::sRGBTag, sRGB);

    ColorPrimaries primaries = cs->profile()->getColorPrimaries();
    if (primaries >= PRIMARIES_ADOBE_RGB_1998) {
        primaries = PRIMARIES_UNSPECIFIED;
    }
    TransferCharacteristics transferFunction =
        cs->profile()->getTransferCharacteristics();
    if (transferFunction >= TRC_GAMMA_1_8) {
        transferFunction = TRC_UNSPECIFIED;
    }
    exportConfiguration->setProperty(KisImportExportFilter::CICPPrimariesTag,
                                     static_cast<int>(primaries));
    exportConfiguration->setProperty(
        KisImportExportFilter::CICPTransferCharacteristicsTag,
        static_cast<int>(transferFunction));
    exportConfiguration->setProperty(KisImportExportFilter::HDRTag, cs->hasHighDynamicRange());
}


void KisImportExportManager::fillStaticExportConfigurationProperties(KisPropertiesConfigurationSP exportConfiguration)
{
    return fillStaticExportConfigurationProperties(exportConfiguration, m_document->image());
}

bool KisImportExportManager::askUserAboutExportConfiguration(
        QSharedPointer<KisImportExportFilter> filter,
        KisPropertiesConfigurationSP exportConfiguration,
        const QByteArray &from,
        const QByteArray &to,
        const bool batchMode,
        const bool showWarnings,
        bool *alsoAsKra,
        bool isAdvancedExporting)
{

    // prevents the animation renderer from running this code


    const QString mimeUserDescription = KisMimeDatabase::descriptionForMimeType(to);

    QStringList warnings;
    QStringList errors;

    {
        KisPreExportChecker checker;
        checker.check(m_document->image(), filter->exportChecks());

        warnings = checker.warnings();
        errors = checker.errors();
    }

    KisConfigWidget *wdg = 0;

    if (QThread::currentThread() == qApp->thread()) {
        wdg = filter->createConfigurationWidget(0, from, to);

        KisMainWindow *kisMain = KisPart::instance()->currentMainwindow();
        if (wdg && kisMain) {
            KisViewManager *manager = kisMain->viewManager();
            wdg->setView(manager);
        }
    }

    // Extra checks that cannot be done by the checker, because the checker only has access to the image.
    if (!m_document->assistants().isEmpty() && to != m_document->nativeFormatMimeType()) {
        warnings.append(i18nc("image conversion warning", "The image contains <b>assistants</b>. The assistants will not be saved."));
    }
    if (m_document->referenceImagesLayer() && m_document->referenceImagesLayer()->shapeCount() > 0 && to != m_document->nativeFormatMimeType()) {
        warnings.append(i18nc("image conversion warning", "The image contains <b>reference images</b>. The reference images will not be saved."));
    }
    if (m_document->guidesConfig().hasGuides() && to != m_document->nativeFormatMimeType()) {
        warnings.append(i18nc("image conversion warning", "The image contains <b>guides</b>. The guides will not be saved."));
    }
    if (!m_document->gridConfig().isDefault() && to != m_document->nativeFormatMimeType()) {
        warnings.append(i18nc("image conversion warning", "The image contains a <b>custom grid configuration</b>. The configuration will not be saved."));
    }

    bool shouldFlattenTheImageBeforeScaling = false;

    if (isAdvancedExporting) {
        QMap<QString, KisExportCheckBase *> exportChecks = filter->exportChecks();

        const bool filterSupportsMultilayerExport =
                exportChecks.contains("MultiLayerCheck") &&
                exportChecks["MultiLayerCheck"]->checkNeeded(m_document->image()) &&
                exportChecks["MultiLayerCheck"]->check(m_document->image()) == KisExportCheckBase::SUPPORTED;

        if (!filterSupportsMultilayerExport) {
            shouldFlattenTheImageBeforeScaling = true;
        } else {
            if (KisLayerUtils::findNodeByType<KisAdjustmentLayer>(m_document->image()->root())) {
                shouldFlattenTheImageBeforeScaling = true;
                warnings.append(i18nc("image conversion warning", "Trying to perform an Advanced Export with the image containing a <b>filter layer</b>. The image will be <b>flattened</b> before resizing."));
            }
            if (KisLayerUtils::findNodeByType<KisFilterMask>(m_document->image()->root())) {
                shouldFlattenTheImageBeforeScaling = true;
                warnings.append(i18nc("image conversion warning", "Trying to perform an Advanced Export with the image containing a <b>filter mask</b>. The image will be <b>flattened</b> before resizing."));
            }
            bool hasLayerStyles =
                    KisLayerUtils::recursiveFindNode(m_document->image()->root(),
                                                     [] (KisNodeSP node) {
                    KisLayer *layer = dynamic_cast<KisLayer*>(node.data());
                    return layer && layer->layerStyle();
        });

            if (hasLayerStyles) {
                shouldFlattenTheImageBeforeScaling = true;
                warnings.append(i18nc("image conversion warning", "Trying to perform an Advanced Export with the image containing a <b>layer style</b>. The image will be <b>flattened</b> before resizing."));
            }
        }
    }

    if (!batchMode && !errors.isEmpty()) {
        QString error =  "<html><body><p><b>"
                + i18n("Error: cannot save this image as a %1.", mimeUserDescription)
                + "</b> " + i18n("Reasons:") + "</p>"
                + "<p/><ul>";
        Q_FOREACH(const QString &w, errors) {
            error += "\n<li>" + w + "</li>";
        }

        error += "</ul>";

        QMessageBox::critical(KisPart::instance()->currentMainwindow(), i18nc("@title:window", "Krita: Export Error"), error);
        return false;
    }

    if (!batchMode && (wdg || !warnings.isEmpty() || isAdvancedExporting)) {
        QWidget *page = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(page);

        if (showWarnings && !warnings.isEmpty()) {
            QHBoxLayout *hLayout = new QHBoxLayout();

            QLabel *labelWarning = new QLabel();
            labelWarning->setPixmap(KisIconUtils::loadIcon("dialog-warning").pixmap(48, 48));
            hLayout->addWidget(labelWarning);

            KisPopupButton *bn = new KisPopupButton(0);

            bn->setText(i18nc("Keep the extra space at the end of the sentence, please", "Warning: saving as a %1 will lose information from your image.    ", mimeUserDescription));
            bn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

            hLayout->addWidget(bn);

            layout->addLayout(hLayout);

            QTextBrowser *browser = new QTextBrowser();
            browser->setMinimumWidth(bn->width());
            bn->setPopupWidget(browser);

            QString warning = "<html><body><p><b>"
                    + i18n("You will lose information when saving this image as a %1.", mimeUserDescription);

            if (warnings.size() == 1) {
                warning += "</b> " + i18n("Reason:") + "</p>";
            }
            else {
                warning += "</b> " + i18n("Reasons:") + "</p>";
            }
            warning += "<p/><ul>";

            Q_FOREACH(const QString &w, warnings) {
                warning += "\n<li>" + w + "</li>";
            }

            warning += "</ul>";
            browser->setHtml(warning);
        }

        QTabWidget *box = new QTabWidget;
        if (wdg) {
            wdg->setConfiguration(exportConfiguration);
            box->addTab(wdg,i18n("Options"));
        }

        DlgImageSize *dlgImageSize = nullptr;

        if (isAdvancedExporting) {
            dlgImageSize = new DlgImageSize(box, m_document->image()->width(), m_document->image()->height(), m_document->image()->yRes());
            dlgImageSize->setButtons(KoDialog::None);

            box->addTab(dlgImageSize,i18n("Resize"));
        }
        layout->addWidget(box);

        QCheckBox *chkAlsoAsKra = 0;
        if (showWarnings && !warnings.isEmpty()) {
            chkAlsoAsKra = new QCheckBox(i18n("Also save your image as a Krita file."));
            chkAlsoAsKra->setChecked(KisConfig(true).readEntry<bool>("AlsoSaveAsKra", false));
            layout->addWidget(chkAlsoAsKra);
        }

        KoDialog dlg(qApp->activeWindow());
        dlg.setMainWidget(page);
        page->setParent(&dlg);
        dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
        dlg.setWindowTitle(mimeUserDescription);

        if (showWarnings || wdg || isAdvancedExporting) {
            if (!dlg.exec()) {
                return false;
            }
        }

        *alsoAsKra = false;
        if (chkAlsoAsKra) {
            KisConfig(false).writeEntry<bool>("AlsoSaveAsKra", chkAlsoAsKra->isChecked());
            *alsoAsKra = chkAlsoAsKra->isChecked();
        }

        KIS_SAFE_ASSERT_RECOVER_NOOP(bool(isAdvancedExporting) == bool(dlgImageSize));

        if (isAdvancedExporting && dlgImageSize) {
            if (shouldFlattenTheImageBeforeScaling) {
                m_document->savingImage()->flatten(KisNodeSP());
                m_document->savingImage()->waitForDone();
            }

            const QSize desiredSize(dlgImageSize->desiredWidth(), dlgImageSize->desiredHeight());
            double res = dlgImageSize->desiredResolution();
            m_document->savingImage()->scaleImage(desiredSize,res,res,dlgImageSize->filterType());
            m_document->savingImage()->waitForDone();
            KisLayerUtils::forceAllDelayedNodesUpdate(m_document->savingImage()->root());
            m_document->savingImage()->waitForDone();
        }

        if (wdg) {
            *exportConfiguration = *wdg->configuration();
        }
    }

    return true;
}

KisImportExportErrorCode KisImportExportManager::doImport(const QString &location, QSharedPointer<KisImportExportFilter> filter)
{
    QFile file(location);
    if (!file.exists()) {
        return ImportExportCodes::FileNotExist;
    }

    if (filter->supportsIO() && !file.open(QFile::ReadOnly)) {
        return KisImportExportErrorCode(KisImportExportErrorCannotRead(file.error()));
    }

    KisImportExportErrorCode status = filter->convert(m_document, &file, KisPropertiesConfigurationSP());

    if (file.isOpen()) {
        file.close();
    }

    return status;
}

KisImportExportErrorCode KisImportExportManager::doExport(const QString &location,
                                                          QSharedPointer<KisImportExportFilter> filter,
                                                          KisPropertiesConfigurationSP exportConfiguration,
                                                          const QString alsoAsKraLocation)
{
    KisImportExportErrorCode status =
            doExportImpl(location, filter, exportConfiguration);

    if (!alsoAsKraLocation.isNull() && status.isOk()) {
        QByteArray mime = m_document->nativeFormatMimeType();
        QSharedPointer<KisImportExportFilter> filter(
                    filterForMimeType(QString::fromLatin1(mime), Export));

        KIS_SAFE_ASSERT_RECOVER_NOOP(filter);

        if (filter) {
            filter->setFilename(alsoAsKraLocation);

            KisPropertiesConfigurationSP kraExportConfiguration =
                    filter->lastSavedConfiguration(mime, mime);

            status = doExportImpl(alsoAsKraLocation, filter, kraExportConfiguration);
        } else {
            status = ImportExportCodes::FileFormatIncorrect;
        }
    }

    return status;
}

// Temporary workaround until QTBUG-57299 is fixed.
// 02-10-2019 update: the bug is closed, but we've still seen this issue.
//                    and without using QSaveFile the issue can still occur
//                    when QFile::copy fails because Dropbox/Google/OneDrive
//                    locks the target file.
// 02-24-2022 update: Added macOS since QSaveFile does not work on sandboxed krita
//                    It can work if user gives access to the container dir, but
//                    we cannot guarantee the user gave us permission.
#if !(defined(Q_OS_WIN) || defined(Q_OS_MACOS))
#define USE_QSAVEFILE
#endif

KisImportExportErrorCode KisImportExportManager::doExportImpl(const QString &location, QSharedPointer<KisImportExportFilter> filter, KisPropertiesConfigurationSP exportConfiguration)
{
#ifdef USE_QSAVEFILE
    QSaveFile file(location);
    file.setDirectWriteFallback(true);
    if (filter->supportsIO() && !file.open(QFile::WriteOnly)) {
#else
    QFileInfo fi(location);
    QTemporaryFile file(QDir::tempPath() + "/.XXXXXX.kra");
    if (filter->supportsIO() && !file.open()) {
#endif
        KisImportExportErrorCannotWrite result(file.error() == QFileDevice::NoError ? QFileDevice::WriteError : file.error());
#ifdef USE_QSAVEFILE
        file.cancelWriting();
#endif
        return result;
    }

    KisImportExportErrorCode status = filter->convert(m_document, &file, exportConfiguration);

    if (filter->supportsIO()) {
        if (!status.isOk()) {
#ifdef USE_QSAVEFILE
            file.cancelWriting();
#endif
        } else {
#ifdef USE_QSAVEFILE
            if (!file.commit()) {
                qWarning() << "Could not commit QSaveFile";
                status = KisImportExportErrorCannotWrite(file.error());
            }
#else
            file.flush();
            file.close();
            QFile target(location);
            if (target.exists()) {
                // There should already be a .kra~ backup
                target.remove();
            }
            if (!file.copy(location)) {
                file.setAutoRemove(false);
                return KisImportExportErrorCannotWrite(file.error());
            }
#endif
        }
    }

    if (status.isOk()) {
        // Do some minimal verification
        QString verificationResult = filter->verify(location);
        if (!verificationResult.isEmpty()) {
            status = KisImportExportErrorCode(ImportExportCodes::ErrorWhileWriting);
            m_document->setErrorMessage(verificationResult);
        }
    }

    return status;

}

QString KisImportExportManager::getAlsoAsKraLocation(const QString location) const
{
#ifdef Q_OS_ANDROID
    return getUriForAdditionalFile(location, nullptr);
#else
    return location + ".kra";
#endif
}

#include <KisMimeDatabase.h>
