/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2001 Werner Trobin <trobin@kde.org>
    SPDX-FileCopyrightText: 2002 Werner Trobin <trobin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisImportExportFilter.h"

#include <QFile>
#include <QFileInfo>
#include <kis_debug.h>
#include <QStack>
#include "KisImportExportManager.h"
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KisExportCheckBase.h>
#include <KisExportCheckRegistry.h>
#include "KoUpdater.h"
#include <klocalizedstring.h>
#include "kis_config.h"
#include <KoStore.h>
#include <KisDocument.h>

const QString KisImportExportFilter::ImageContainsTransparencyTag = "ImageContainsTransparency";
const QString KisImportExportFilter::ColorModelIDTag = "ColorModelID";
const QString KisImportExportFilter::ColorDepthIDTag = "ColorDepthID";
const QString KisImportExportFilter::sRGBTag = "sRGB";
const QString KisImportExportFilter::CICPPrimariesTag = "CICPCompatiblePrimaries";
const QString KisImportExportFilter::CICPTransferCharacteristicsTag = "CICPCompatibleTransferFunction";
const QString KisImportExportFilter::HDRTag = "HDRSupported";

class Q_DECL_HIDDEN KisImportExportFilter::Private
{
public:
    QPointer<KoUpdater> updater;
    QByteArray mime;
    QString filename;
    QString realFilename;
    bool batchmode;

    QMap<QString, KisExportCheckBase*> capabilities;

    Private()
        : updater(0), mime("")
        , batchmode(false)
    {}

    ~Private()
    {
        qDeleteAll(capabilities);
    }

};


KisImportExportFilter::KisImportExportFilter(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

KisImportExportFilter::~KisImportExportFilter()
{
    if (d->updater) {
        d->updater->setProgress(100);
    }
    delete d;
}

QString KisImportExportFilter::filename() const
{
    return d->filename;
}

QString KisImportExportFilter::realFilename() const
{
    return d->realFilename;
}

bool KisImportExportFilter::batchMode() const
{
    return d->batchmode;
}


void KisImportExportFilter::setBatchMode(bool batchmode)
{
    d->batchmode = batchmode;
}

void KisImportExportFilter::setFilename(const QString &filename)
{
    d->filename = filename;
}

void KisImportExportFilter::setRealFilename(const QString &filename)
{
    d->realFilename = filename;
}


void KisImportExportFilter::setMimeType(const QString &mime)
{
    d->mime = mime.toLatin1();
}

QByteArray KisImportExportFilter::mimeType() const
{
    return d->mime;
}

KisPropertiesConfigurationSP KisImportExportFilter::defaultConfiguration(const QByteArray &from, const QByteArray &to) const
{
    Q_UNUSED(from);
    Q_UNUSED(to);
    return 0;
}

KisPropertiesConfigurationSP KisImportExportFilter::lastSavedConfiguration(const QByteArray &from, const QByteArray &to) const
{
    KisPropertiesConfigurationSP cfg = defaultConfiguration(from, to);
    const QString filterConfig = KisConfig(true).exportConfigurationXML(to);
    if (cfg && !filterConfig.isEmpty()) {
        cfg->fromXML(filterConfig, false);
    }
    return cfg;
}

KisConfigWidget *KisImportExportFilter::createConfigurationWidget(QWidget *, const QByteArray &from, const QByteArray &to) const
{
    Q_UNUSED(from);
    Q_UNUSED(to);
    return 0;
}

QMap<QString, KisExportCheckBase *> KisImportExportFilter::exportChecks()
{
    qDeleteAll(d->capabilities);
    initializeCapabilities();
    return d->capabilities;
}

QString KisImportExportFilter::verify(const QString &fileName) const
{
    QFileInfo fi(fileName);

    if (!fi.exists()) {
        return i18n("%1 does not exist after writing. Try saving again under a different name, in another location.", fileName);
    }

    if (!fi.isReadable()) {
        return i18n("%1 is not readable", fileName);
    }

    if (fi.size() < 10)  {
        return i18n("%1 is smaller than 10 bytes, it must be corrupt. Try saving again under a different name, in another location.", fileName);
    }

    QFile f(fileName);
    f.open(QFile::ReadOnly);
    QByteArray ba = f.read(std::min(f.size(), (qint64)1000));
    bool found = false;
    for(int i = 0; i < ba.size(); ++i) {
        if (ba.at(i) > 0) {
            found = true;
            break;
        }
    }

    if (!found) {
        return i18n("%1 has only zero bytes in the first 1000 bytes, it's probably corrupt. Try saving again under a different name, in another location.", fileName);
    }

    return QString();
}

void KisImportExportFilter::setUpdater(QPointer<KoUpdater> updater)
{
    d->updater = updater;
}

QPointer<KoUpdater> KisImportExportFilter::updater()
{
    return d->updater;
}

void KisImportExportFilter::setProgress(int value)
{
    if (d->updater) {
        d->updater->setValue(value);
    }
}

void KisImportExportFilter::initializeCapabilities()
{
    // XXX: Initialize everything to fully supported?
}

void KisImportExportFilter::addCapability(KisExportCheckBase *capability)
{
    d->capabilities[capability->id()] = capability;
}



void KisImportExportFilter::addSupportedColorModels(QList<QPair<KoID, KoID> > supportedColorModels, const QString &name, KisExportCheckBase::Level level)
{
    Q_ASSERT(level != KisExportCheckBase::SUPPORTED);
    QString layerMessage;
    QString imageMessage;
    QList<KoID> allColorModels = KoColorSpaceRegistry::instance()->colorModelsList(KoColorSpaceRegistry::AllColorSpaces);
    Q_FOREACH(const KoID &colorModelID, allColorModels) {
        QList<KoID> allColorDepths = KoColorSpaceRegistry::instance()->colorDepthList(colorModelID.id(), KoColorSpaceRegistry::AllColorSpaces);
        Q_FOREACH(const KoID &colorDepthID, allColorDepths) {

            KisExportCheckFactory *colorModelCheckFactory =
                    KisExportCheckRegistry::instance()->get("ColorModelCheck/" + colorModelID.id() + "/" + colorDepthID.id());
            KisExportCheckFactory *colorModelPerLayerCheckFactory =
                    KisExportCheckRegistry::instance()->get("ColorModelPerLayerCheck/" + colorModelID.id() + "/" + colorDepthID.id());

            if(!colorModelCheckFactory || !colorModelPerLayerCheckFactory) {
                qWarning() << "No factory for" << colorModelID << colorDepthID;
                continue;
            }

            if (supportedColorModels.contains(QPair<KoID, KoID>(colorModelID, colorDepthID))) {
                addCapability(colorModelCheckFactory->create(KisExportCheckBase::SUPPORTED));
                addCapability(colorModelPerLayerCheckFactory->create(KisExportCheckBase::SUPPORTED));
            }
            else {


                if (level == KisExportCheckBase::PARTIALLY) {
                    imageMessage = i18nc("image conversion warning",
                                         "%1 cannot save images with color model <b>%2</b> and depth <b>%3</b>. The image will be converted."
                                         ,name, colorModelID.name(), colorDepthID.name());

                    layerMessage =
                            i18nc("image conversion warning",
                                  "%1 cannot save layers with color model <b>%2</b> and depth <b>%3</b>. The layers will be converted or skipped."
                                  ,name, colorModelID.name(), colorDepthID.name());
                }
                else {
                    imageMessage = i18nc("image conversion warning",
                                         "%1 cannot save images with color model <b>%2</b> and depth <b>%3</b>. The image will not be saved."
                                         ,name, colorModelID.name(), colorDepthID.name());

                    layerMessage =
                            i18nc("image conversion warning",
                                  "%1 cannot save layers with color model <b>%2</b> and depth <b>%3</b>. The layers will be skipped."
                                  , name, colorModelID.name(), colorDepthID.name());
                 }



                addCapability(colorModelCheckFactory->create(level, imageMessage));
                addCapability(colorModelPerLayerCheckFactory->create(level, layerMessage));
            }
        }
    }
}

QString KisImportExportFilter::verifyZiPBasedFiles(const QString &fileName, const QStringList &filesToCheck) const
{
    QScopedPointer<KoStore> store(KoStore::createStore(fileName, KoStore::Read, KIS_MIME_TYPE, KoStore::Zip));

    if (!store || store->bad()) {
        return i18n("Could not open the saved file %1. Please try to save again in a different location.", fileName);
    }

    Q_FOREACH(const QString &file, filesToCheck) {
        if (!store->hasFile(file)) {
            return i18n("File %1 is missing in %2 and is broken. Please try to save again in a different location.", file, fileName);
        }
    }

    return QString();

}
