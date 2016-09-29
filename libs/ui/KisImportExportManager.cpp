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
    if (!batchMode() && (wdg || !checker.warnings().isEmpty())) {

        KoDialog dlg;

        dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
        dlg.setWindowTitle(KisMimeDatabase::descriptionForMimeType(mimeType));

        QWidget *page = new QWidget(&dlg);
        QVBoxLayout *layout = new QVBoxLayout(page);

        if (!checker.warnings().isEmpty()) {
            QTextBrowser *browser = new QTextBrowser();
            layout->addWidget(browser);
            QString description = KisMimeDatabase::descriptionForMimeType(typeName);
            qDebug() << description;
            QString warning = "<html><body><p><b>"
                    + i18n("You will lose information when saving this image as a %1.", description)
                    + "</p></b><p>"
                    + i18n("Reasons:")
                    + "</p><ul>";
            Q_FOREACH(const QString &w, checker.warnings()) {
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

    if (!batchMode()) {
        QApplication::restoreOverrideCursor();
    }

    if (alsoAsKra && alsoAsKra->isChecked()) {
        QString l = location + ".kra";
        QByteArray ba = m_document->nativeFormatMimeType();
        exportDocument(l, ba);
    }

//    m_document->setCurrentImage(unconvertedImage);

    return status;

}


#include <KisMimeDatabase.h>
