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
#include <QGroupBox>

#include <klocalizedstring.h>
#include <ksqueezedtextlabel.h>
#include <kpluginfactory.h>

#include <kis_icon_utils.h>
#include <KoDialog.h>
#include <KoProgressUpdater.h>
#include <KoJsonTrader.h>
#include <KisMimeDatabase.h>
#include <kis_config_widget.h>
#include <kis_debug.h>
#include <KisMimeDatabase.h>
#include <KisPreExportChecker.h>
#include <KisPart.h>
#include "kis_config.h"
#include "KisImportExportFilter.h"
#include "KisDocument.h"
#include "kis_image.h"
#include "kis_guides_config.h"
#include "kis_grid_config.h"
#include "kis_popup_button.h"

// static cache for import and export mimetypes
QStringList KisImportExportManager::m_importMimeTypes;
QStringList KisImportExportManager::m_exportMimeTypes;

class Q_DECL_HIDDEN KisImportExportManager::Private
{
public:
    bool batchMode {false};
    QPointer<KoProgressUpdater> progressUpdater {0};
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

KisImportExportFilter::ConversionStatus KisImportExportManager::importDocument(const QString& location, const QString& mimeType)
{
    return convert(Import, location, mimeType, 0);
}

KisImportExportFilter::ConversionStatus KisImportExportManager::exportDocument(const QString& location, QByteArray& mimeType, KisPropertiesConfigurationSP exportConfiguration)
{
    return convert(Export, location, mimeType, exportConfiguration);
}

// The static method to figure out to which parts of the
// graph this mimetype has a connection to.
QStringList KisImportExportManager::mimeFilter(Direction direction)
{
    // Find the right mimetype by the extension
    QSet<QString> mimeTypes;
    //    mimeTypes << KisDocument::nativeFormatMimeType() << "application/x-krita-paintoppreset" << "image/openraster";

    if (direction == KisImportExportManager::Import) {
        if (m_importMimeTypes.isEmpty()) {
            KoJsonTrader trader;
            QList<QPluginLoader *>list = trader.query("Krita/FileFilter", "");
            Q_FOREACH(QPluginLoader *loader, list) {
                QJsonObject json = loader->metaData().value("MetaData").toObject();
                Q_FOREACH(const QString &mimetype, json.value("X-KDE-Import").toString().split(",")) {
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
            KoJsonTrader trader;
            QList<QPluginLoader *>list = trader.query("Krita/FileFilter", "");
            Q_FOREACH(QPluginLoader *loader, list) {
                QJsonObject json = loader->metaData().value("MetaData").toObject();
                Q_FOREACH(const QString &mimetype, json.value("X-KDE-Export").toString().split(",")) {
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
    KoJsonTrader trader;
    QList<QPluginLoader *>list = trader.query("Krita/FileFilter", "");
    Q_FOREACH(QPluginLoader *loader, list) {
        QJsonObject json = loader->metaData().value("MetaData").toObject();
        QString directionKey = direction == Export ? "X-KDE-Export" : "X-KDE-Import";
        if (json.value(directionKey).toString().split(",").contains(mimetype)) {
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
    filter->setMimeType(mimetype);
    return filter;
}

void KisImportExportManager::setBatchMode(const bool batch)
{
    d->batchMode = batch;
}

bool KisImportExportManager::batchMode(void) const
{
    return d->batchMode;
}

void KisImportExportManager::setProgresUpdater(KoProgressUpdater *updater)
{
    d->progressUpdater = updater;
}

KisImportExportFilter::ConversionStatus KisImportExportManager::convert(KisImportExportManager::Direction direction, const QString &location, const QString &mimeType, KisPropertiesConfigurationSP exportConfiguration)
{

    QString typeName = mimeType;
    if (typeName.isEmpty()) {
        typeName = KisMimeDatabase::mimeTypeForFile(location);
    }

    QSharedPointer<KisImportExportFilter> filter(filterForMimeType(typeName, direction));
    if (!filter) {
        return KisImportExportFilter::FilterCreationError;
    }

    filter->setFilename(location);
    filter->setBatchMode(batchMode());
    filter->setMimeType(typeName);

    if (d->progressUpdater) {
        filter->setUpdater(d->progressUpdater->startSubtask());
    }

    QByteArray from, to;
    if (direction == Export) {
        from = m_document->nativeFormatMimeType();
        to = mimeType.toLatin1();
    }
    else {
        from = mimeType.toLatin1();
        to = m_document->nativeFormatMimeType();
    }

    if (!exportConfiguration) {
        exportConfiguration = filter->lastSavedConfiguration(from, to);
    }

//    KisImageSP unconvertedImage = m_document->image();

    KisPreExportChecker checker;
    if (direction == Export) {
        if (!checker.check(m_document->image(), filter->exportChecks())) {
//        KisImageSP convertedImage = checker.convertedImage(m_document->image());
//        m_document->setCurrentImage(convertedImage);
        }
    }

    KisConfigWidget *wdg = filter->createConfigurationWidget(0, from, to);
    QCheckBox *alsoAsKra = 0;

    QStringList warnings = checker.warnings();
    QStringList errors = checker.errors();

    // Extra checks that cannot be done by the checker, because the checker only has access to the image.
    if (!m_document->assistants().isEmpty() && typeName != m_document->nativeFormatMimeType()) {
        warnings.append(i18nc("image conversion warning", "The image contains <b>assistants</b>. The assistants will not be saved."));
    }
    if (m_document->guidesConfig().hasGuides() && typeName != m_document->nativeFormatMimeType()) {
        warnings.append(i18nc("image conversion warning", "The image contains <b>guides</b>. The guides will not be saved."));
    }
    if (!m_document->gridConfig().isDefault() && typeName != m_document->nativeFormatMimeType()) {
        warnings.append(i18nc("image conversion warning", "The image contains a <b>custom grid configuration</b>. The configuration will not be saved."));
    }

    if (!batchMode() && !errors.isEmpty()) {
        QString error =  "<html><body><p><b>"
                + i18n("Error: cannot save this image as a %1.", KisMimeDatabase::descriptionForMimeType(typeName))
                + "</b> "
                + i18n("Reasons:</p>")
                + "</p><ul>";
        Q_FOREACH(const QString &w, errors) {
            error += "\n<li>" + w + "</li>";
        }

        error += "</ul>";

        QMessageBox::critical(KisPart::instance()->currentMainwindow(), i18nc("@title:window", "Krita: Export Error"), error);
        return KisImportExportFilter::UsageError;
    }

    if (!batchMode() && (wdg || !warnings.isEmpty())) {

        KoDialog dlg;

        dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
        dlg.setWindowTitle(KisMimeDatabase::descriptionForMimeType(mimeType));

        QWidget *page = new QWidget(&dlg);
        QVBoxLayout *layout = new QVBoxLayout(page);

        if (!checker.warnings().isEmpty()) {

            QHBoxLayout *hLayout = new QHBoxLayout();

            QLabel *labelWarning = new QLabel();
            labelWarning->setPixmap(KisIconUtils::loadIcon("dialog-warning").pixmap(32, 32));
            hLayout->addWidget(labelWarning);

            KisPopupButton *bn = new KisPopupButton(0);
            //bn->setIcon(KisIconUtils::loadIcon("dialog-"));
            bn->setText(i18nc("Keep the extra space at the end of the sentencte, please", "Warning: saving as %1 will lose information from your image.    ", KisMimeDatabase::descriptionForMimeType(mimeType)));
            hLayout->addWidget(bn);

            layout->addLayout(hLayout);

            QTextBrowser *browser = new QTextBrowser();
            browser->setMinimumWidth(bn->width());
            bn->setPopupWidget(browser);

            QString warning = "<html><body><p><b>"
                    + i18n("You will lose information when saving this image as a %1.", KisMimeDatabase::descriptionForMimeType(typeName))
                    + "</b> "
                    + i18n("Reasons:</p>")
                    + "</p><ul>";

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

        if (!checker.warnings().isEmpty()) {
            alsoAsKra = new QCheckBox(i18n("Also save your image as a Krita file."));
            alsoAsKra->setChecked(KisConfig().readEntry<bool>("AlsoSaveAsKra", false));
            layout->addWidget(alsoAsKra);
        }

        dlg.setMainWidget(page);
        dlg.resize(dlg.minimumSize());

        if (!dlg.exec()) {
            return KisImportExportFilter::UserCancelled;
        }

        if (alsoAsKra) {
            KisConfig().writeEntry<bool>("AlsoSaveAsKra", alsoAsKra->isChecked());
        }

        if (wdg) {
            exportConfiguration = wdg->configuration();
        }

        QApplication::setOverrideCursor(Qt::WaitCursor);
    }

    QFile io(location);

    if (direction == Import) {
        if (!io.exists()) {
            return KisImportExportFilter::FileNotFound;
        }

        if (!io.open(QFile::ReadOnly)) {
            return KisImportExportFilter::FileNotFound;
        }
    }
    else if (direction == Export) {
        if (!io.open(QFile::WriteOnly)) {
            return KisImportExportFilter::CreationError;
        }
    }
    else {
        return KisImportExportFilter::BadConversionGraph;
    }

    KisImportExportFilter::ConversionStatus status = filter->convert(m_document, &io, exportConfiguration);
    io.close();

    if (exportConfiguration) {
       KisConfig().setExportConfiguration(typeName, exportConfiguration);
    }

    if (alsoAsKra && alsoAsKra->isChecked()) {
        QString l = location + ".kra";
        QByteArray ba = m_document->nativeFormatMimeType();
        KisImportExportFilter *filter = filterForMimeType(QString::fromLatin1(ba), Export);
        QFile f(l);
        f.open(QIODevice::WriteOnly);
        if (filter) {
            filter->setFilename(l);
            filter->convert(m_document, &f);
        }
        f.close();
        delete filter;

    }

    if (!batchMode()) {
        QApplication::restoreOverrideCursor();
    }

//    m_document->setCurrentImage(unconvertedImage);

    return status;

}


#include <KisMimeDatabase.h>
