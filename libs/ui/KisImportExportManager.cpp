/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KisImportExportManager.h"

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

#include <KisUsageLogger.h>
#include <KoFileDialog.h>
#include <kis_icon_utils.h>
#include <KoDialog.h>
#include <KoProgressUpdater.h>
#include <KoJsonTrader.h>
#include <KisMimeDatabase.h>
#include <kis_config_widget.h>
#include <kis_debug.h>
#include <KisPreExportChecker.h>
#include <KisPart.h>
#include "kis_config.h"
#include "KisImportExportFilter.h"
#include "KisDocument.h"
#include <kis_image.h>
#include <kis_paint_layer.h>
#include "kis_painter.h"
#include "kis_guides_config.h"
#include "kis_grid_config.h"
#include "kis_popup_button.h"
#include <kis_iterator_ng.h>
#include "kis_async_action_feedback.h"
#include "KisReferenceImagesLayer.h"

// static cache for import and export mimetypes
QStringList KisImportExportManager::m_importMimeTypes;
QStringList KisImportExportManager::m_exportMimeTypes;

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

KisImportExportErrorCode KisImportExportManager::exportDocument(const QString& location, const QString& realLocation, const QByteArray& mimeType, bool showWarnings, KisPropertiesConfigurationSP exportConfiguration)
{
    ConversionResult result = convert(Export, location, realLocation, mimeType, showWarnings, exportConfiguration, false);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!result.isAsync(), ImportExportCodes::InternalError);

    return result.status();
}

QFuture<KisImportExportErrorCode> KisImportExportManager::exportDocumentAsyc(const QString &location, const QString &realLocation, const QByteArray &mimeType,
                                                                            KisImportExportErrorCode &status, bool showWarnings, KisPropertiesConfigurationSP exportConfiguration)
{
    ConversionResult result = convert(Export, location, realLocation, mimeType, showWarnings, exportConfiguration, true);
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
            QList<QPluginLoader *>list = KoJsonTrader::instance()->query("Krita/FileFilter", "");
            Q_FOREACH(QPluginLoader *loader, list) {
                QJsonObject json = loader->metaData().value("MetaData").toObject();
                Q_FOREACH(const QString &mimetype, json.value("X-KDE-Import").toString().split(",", QString::SkipEmptyParts)) {
                    //qDebug() << "Adding  import mimetype" << mimetype << KisMimeDatabase::descriptionForMimeType(mimetype) << "from plugin" << loader;
                    mimeTypes << mimetype;
                }
            }
            qDeleteAll(list);
            m_importMimeTypes = mimeTypes.toList();
        }
        return m_importMimeTypes;
    }
    else if (direction == KisImportExportManager::Export) {
        if (m_exportMimeTypes.isEmpty()) {
            QList<QPluginLoader *>list = KoJsonTrader::instance()->query("Krita/FileFilter", "");
            Q_FOREACH(QPluginLoader *loader, list) {
                QJsonObject json = loader->metaData().value("MetaData").toObject();
                Q_FOREACH(const QString &mimetype, json.value("X-KDE-Export").toString().split(",", QString::SkipEmptyParts)) {
                    //qDebug() << "Adding  export mimetype" << mimetype << KisMimeDatabase::descriptionForMimeType(mimetype) << "from plugin" << loader;
                    mimeTypes << mimetype;
                }
            }
            qDeleteAll(list);
            m_exportMimeTypes = mimeTypes.toList();
        }
        return m_exportMimeTypes;
    }
    return QStringList();
}

KisImportExportFilter *KisImportExportManager::filterForMimeType(const QString &mimetype, KisImportExportManager::Direction direction)
{
    int weight = -1;
    KisImportExportFilter *filter = 0;
    QList<QPluginLoader *>list = KoJsonTrader::instance()->query("Krita/FileFilter", "");
    Q_FOREACH(QPluginLoader *loader, list) {
        QJsonObject json = loader->metaData().value("MetaData").toObject();
        QString directionKey = direction == Export ? "X-KDE-Export" : "X-KDE-Import";
        if (json.value(directionKey).toString().split(",", QString::SkipEmptyParts).contains(mimetype)) {
            KLibFactory *factory = qobject_cast<KLibFactory *>(loader->instance());

            if (!factory) {
                warnUI << loader->errorString();
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

            int w = json.value("X-KDE-Weight").toInt();
            if (w > weight) {
                delete filter;
                filter = f;
                f->setObjectName(loader->fileName());
                weight = w;
            }
        }
    }
    qDeleteAll(list);
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
    dialog.setCaption(i18nc("@titile:window", "Open Audio"));

    return dialog.filename();
}

KisImportExportManager::ConversionResult KisImportExportManager::convert(KisImportExportManager::Direction direction, const QString &location, const QString& realLocation, const QString &mimeType, bool showWarnings, KisPropertiesConfigurationSP exportConfiguration, bool isAsync)
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
        return KisImportExportErrorCode(ImportExportCodes::FileFormatIncorrect);
    }

    filter->setFilename(location);
    filter->setRealFilename(realLocation);
    filter->setBatchMode(batchMode());
    filter->setMimeType(typeName);

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
                            .arg(QString::fromLatin1(from))
                            .arg(QString::fromLatin1(to))
                            .arg(location)
                            .arg(realLocation)
                            .arg(batchMode()));



        // async importing is not yet supported!
        KIS_SAFE_ASSERT_RECOVER_NOOP(!isAsync);

        if (0 && !batchMode()) {
            KisAsyncActionFeedback f(i18n("Opening document..."), 0);
            result = f.runAction(std::bind(&KisImportExportManager::doImport, this, location, filter));
        } else {
            result = doImport(location, filter);
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
                                                       &alsoAsKra);


        if (!batchMode() && !askUser) {
            return KisImportExportErrorCode(ImportExportCodes::Cancelled);
        }

        KisUsageLogger::log(QString("Converting from %1 to %2. Location: %3. Real location: %4. Batchmode: %5. Configuration: %6")
                            .arg(QString::fromLatin1(from))
                            .arg(QString::fromLatin1(to))
                            .arg(location)
                            .arg(realLocation)
                            .arg(batchMode())
                            .arg(exportConfiguration ? exportConfiguration->toXML() : "none"));



        if (isAsync) {
            result = QtConcurrent::run(std::bind(&KisImportExportManager::doExport, this, location, filter, exportConfiguration, alsoAsKra));

            // we should explicitly report that the exporting has been initiated
            result.setStatus(ImportExportCodes::OK);

        } else if (!batchMode()) {
            KisAsyncActionFeedback f(i18n("Saving document..."), 0);
            result = f.runAction(std::bind(&KisImportExportManager::doExport, this, location, filter, exportConfiguration, alsoAsKra));
        } else {
            result = doExport(location, filter, exportConfiguration, alsoAsKra);
        }

        if (exportConfiguration && !batchMode() && showWarnings) {
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
        bool *alsoAsKra)
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

    if (!batchMode && (wdg || !warnings.isEmpty())) {

        KoDialog dlg;

        dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
        dlg.setWindowTitle(mimeUserDescription);

        QWidget *page = new QWidget(&dlg);
        QVBoxLayout *layout = new QVBoxLayout(page);

        if (showWarnings && !warnings.isEmpty()) {
            QHBoxLayout *hLayout = new QHBoxLayout();

            QLabel *labelWarning = new QLabel();
            labelWarning->setPixmap(KisIconUtils::loadIcon("warning").pixmap(32, 32));
            hLayout->addWidget(labelWarning);

            KisPopupButton *bn = new KisPopupButton(0);

            bn->setText(i18nc("Keep the extra space at the end of the sentence, please", "Warning: saving as %1 will lose information from your image.    ", mimeUserDescription));

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

        if (wdg) {
            QGroupBox *box = new QGroupBox(i18n("Options"));
            QVBoxLayout *boxLayout = new QVBoxLayout(box);
            wdg->setConfiguration(exportConfiguration);
            boxLayout->addWidget(wdg);
            layout->addWidget(box);
        }

        QCheckBox *chkAlsoAsKra = 0;
        if (showWarnings && !warnings.isEmpty()) {
            chkAlsoAsKra = new QCheckBox(i18n("Also save your image as a Krita file."));
            chkAlsoAsKra->setChecked(KisConfig(true).readEntry<bool>("AlsoSaveAsKra", false));
            layout->addWidget(chkAlsoAsKra);
        }

        dlg.setMainWidget(page);
        dlg.resize(dlg.minimumSize());

        if (showWarnings || wdg) {
            if (!dlg.exec()) {
                return false;
            }
        }

        *alsoAsKra = false;
        if (chkAlsoAsKra) {
            KisConfig(false).writeEntry<bool>("AlsoSaveAsKra", chkAlsoAsKra->isChecked());
            *alsoAsKra = chkAlsoAsKra->isChecked();
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

KisImportExportErrorCode KisImportExportManager::doExport(const QString &location, QSharedPointer<KisImportExportFilter> filter, KisPropertiesConfigurationSP exportConfiguration, bool alsoAsKra)
{
    KisImportExportErrorCode status =
            doExportImpl(location, filter, exportConfiguration);

    if (alsoAsKra && status.isOk()) {
        QString kraLocation = location + ".kra";
        QByteArray mime = m_document->nativeFormatMimeType();
        QSharedPointer<KisImportExportFilter> filter(
                    filterForMimeType(QString::fromLatin1(mime), Export));

        KIS_SAFE_ASSERT_RECOVER_NOOP(filter);

        if (filter) {
            filter->setFilename(kraLocation);

            KisPropertiesConfigurationSP kraExportConfiguration =
                    filter->lastSavedConfiguration(mime, mime);

            status = doExportImpl(kraLocation, filter, kraExportConfiguration);
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
#ifndef Q_OS_WIN
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
    QTemporaryFile file(fi.absolutePath() + ".XXXXXX.kra");
    if (filter->supportsIO() && !file.open()) {
#endif
        KisImportExportErrorCannotWrite result(file.error());
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

    // Do some minimal verification
    QString verificationResult = filter->verify(location);
    qDebug() << verificationResult;
    if (!verificationResult.isEmpty()) {
        status = KisImportExportErrorCode(ImportExportCodes::ErrorWhileWriting);
        m_document->setErrorMessage(verificationResult);
    }


    return status;

}

#include <KisMimeDatabase.h>
