/* This file is part of the KDE libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>
                 2002 Werner Trobin <trobin@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "KisImportExportFilter.h"

#include <QFile>
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

const QString KisImportExportFilter::ImageContainsTransparencyTag = "ImageContainsTransparency";
const QString KisImportExportFilter::ColorModelIDTag = "ColorModelID";
const QString KisImportExportFilter::ColorDepthIDTag = "ColorDepthID";
const QString KisImportExportFilter::sRGBTag = "sRGB";

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

QString KisImportExportFilter::conversionStatusString(ConversionStatus status)
{
    QString msg;
    switch (status) {
    case OK: break;

    case FilterCreationError:
        msg = i18n("Krita does not support this file format"); break;

    case CreationError:
        msg = i18n("Could not create the output document"); break;

    case FileNotFound:
        msg = i18n("File not found"); break;

    case StorageCreationError:
        msg = i18n("Cannot create storage"); break;

    case BadMimeType:
        msg = i18n("Bad MIME type"); break;

    case WrongFormat:
        msg = i18n("Format not recognized"); break;

    case NotImplemented:
        msg = i18n("Not implemented"); break;

    case ParsingError:
        msg = i18n("Parsing error"); break;

    case InvalidFormat:
        msg = i18n("Invalid file format"); break;

    case InternalError:
    case UsageError:
        msg = i18n("Internal error"); break;

    case ProgressCancelled:
        msg = i18n("Cancelled by user"); break;

    case BadConversionGraph:

        msg = i18n("Unknown file type"); break;

    case UnsupportedVersion:

        msg = i18n("Unsupported file version"); break;

    case UserCancelled:

        // intentionally we do not prompt the error message here
        break;


    default: msg = i18n("Unknown error"); break;
    }
    return msg;
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

void KisImportExportFilter::setUpdater(QPointer<KoUpdater> updater)
{
    d->updater = updater;
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
