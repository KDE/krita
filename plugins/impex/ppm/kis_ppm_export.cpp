/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_ppm_export.h"

#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KoColorSpaceConstants.h>

#include <KisImportExportManager.h>
#include <KisExportCheckRegistry.h>

#include <kis_debug.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_properties_configuration.h>
#include <kis_config.h>

#include <qendian.h>
#include <KoColorSpaceTraits.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include "kis_iterator_ng.h"

#include <QApplication>


K_PLUGIN_FACTORY_WITH_JSON(KisPPMExportFactory, "krita_ppm_export.json", registerPlugin<KisPPMExport>();)

KisPPMExport::KisPPMExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisPPMExport::~KisPPMExport()
{
}

class KisPPMFlow
{
public:
    KisPPMFlow() {
    }
    virtual ~KisPPMFlow() {
    }
    virtual void writeBool(quint8 v) = 0;
    virtual void writeBool(quint16 v) = 0;
    virtual void writeNumber(quint8 v) = 0;
    virtual void writeNumber(quint16 v) = 0;
    virtual void flush() = 0;
private:
};

class KisPPMAsciiFlow : public KisPPMFlow
{
public:
    KisPPMAsciiFlow(QIODevice* device) : m_device(device) {
    }
    ~KisPPMAsciiFlow() override {
    }
    void writeBool(quint8 v) override {
        if (v > 127) {
            m_device->write("1 ");
        } else {
            m_device->write("0 ");
        }
    }
    void writeBool(quint16 v) override {
        writeBool(quint8(v >> 8));
    }
    void writeNumber(quint8 v) override {
        m_device->write(QByteArray::number(v));
        m_device->write(" ");
    }
    void writeNumber(quint16 v) override {
        m_device->write(QByteArray::number(v));
        m_device->write(" ");
    }
    void flush() override {
    }
private:
    QIODevice* m_device;
};

class KisPPMBinaryFlow : public KisPPMFlow
{
public:
    KisPPMBinaryFlow(QIODevice* device) : m_device(device), m_pos(0), m_current(0) {
    }
    ~KisPPMBinaryFlow() override {
    }
    void writeBool(quint8 v) override {
        m_current = m_current << 1;
        m_current |= (v > 127);
        ++m_pos;
        if (m_pos >= 8) {
            m_current = 0;
            m_pos = 0;
            flush();
        }
    }
    void writeBool(quint16 v) override {
        writeBool(quint8(v >> 8));
    }
    void writeNumber(quint8 v) override {
        m_device->write((char*)&v, 1);
    }
    void writeNumber(quint16 v) override {
        quint16 vo = qToBigEndian(v);
        m_device->write((char*)&vo, 2);
    }
    void flush() override {
        m_device->write((char*)&m_current, 1);
    }
private:
    QIODevice* m_device;
    int m_pos;
    quint8 m_current;
};

KisImportExportErrorCode KisPPMExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{
    bool rgb = (mimeType() == "image/x-portable-pixmap");
    bool binary = (configuration->getInt("type") == 0);

    bool bitmap = (mimeType() == "image/x-portable-bitmap");

    KisImageSP image = document->savingImage();
    KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());

    // Test color space
    if (((rgb && (pd->colorSpace()->id() != "RGBA" && pd->colorSpace()->id() != "RGBA16"))
            || (!rgb && (pd->colorSpace()->id() != "GRAYA" && pd->colorSpace()->id() != "GRAYA16" && pd->colorSpace()->id() != "GRAYAU16")))) {
        if (rgb) {
            pd->convertTo(KoColorSpaceRegistry::instance()->rgb8(0), KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
        }
        else {
            pd->convertTo(KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), 0), KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
        }
    }

    bool is16bit = pd->colorSpace()->id() == "RGBA16" || pd->colorSpace()->id() == "GRAYAU16";

    // Write the magic
    QString toWrite = "";
    int written = 0;
    if (rgb) {
        if (binary) {
            toWrite = "P6";
        }
        else {
            toWrite = "P3";
        }
    } else if (bitmap) {
        if (binary) {
            toWrite = "P4";
        }
        else {
            toWrite = "P1";
        }
    } else {
        if (binary) {
            toWrite = "P5";
        }
        else {
            toWrite = "P2";
        }
    }
    written = io->write(toWrite.toUtf8());
    if (written != toWrite.toUtf8().length()) {
        return ImportExportCodes::ErrorWhileWriting;
    }

    written = io->write("\n");

    // Write the header
    QByteArray width = QByteArray::number(image->width());
    QByteArray height = QByteArray::number(image->height());

    written += io->write(width);
    written += io->write(" ");
    written += io->write(height);
    if (written != QString(" ").length() + QString("\n").length() + width.length() + height.length()) {
        return ImportExportCodes::ErrorWhileWriting;
    }
    if (!bitmap) {
        if (is16bit) {
            toWrite = " 65535\n";
        }
        else {
            toWrite = " 255\n";
        }
    } else {
        toWrite = "\n";
    }
    written = io->write(toWrite.toUtf8());
    if (written != toWrite.toUtf8().length()) {
        return ImportExportCodes::ErrorWhileWriting;
    }

    // Write the data
    KisPPMFlow* flow = 0;
    if (binary) flow = new KisPPMBinaryFlow(io);
    else flow = new KisPPMAsciiFlow(io);

    for (int y = 0; y < image->height(); ++y) {
        KisHLineIteratorSP it = pd->createHLineIteratorNG(0, y, image->width());
        if (is16bit) {
            if (rgb) {
                do {
                    flow->writeNumber(KoBgrU16Traits::red(it->rawData()));
                    flow->writeNumber(KoBgrU16Traits::green(it->rawData()));
                    flow->writeNumber(KoBgrU16Traits::blue(it->rawData()));

                } while (it->nextPixel());
            } else if (bitmap) {
                do {
                    flow->writeBool(*reinterpret_cast<quint16*>(it->rawData()));

                } while (it->nextPixel());
            } else {
                do {
                    flow->writeNumber(*reinterpret_cast<quint16*>(it->rawData()));
                } while (it->nextPixel());
            }
        } else {
            if (rgb) {
                do {
                    flow->writeNumber(KoBgrTraits<quint8>::red(it->rawData()));
                    flow->writeNumber(KoBgrTraits<quint8>::green(it->rawData()));
                    flow->writeNumber(KoBgrTraits<quint8>::blue(it->rawData()));

                } while (it->nextPixel());
            } else if (bitmap) {
                do {
                    flow->writeBool(*reinterpret_cast<quint8*>(it->rawData()));

                } while (it->nextPixel());
            } else {
                do {
                    flow->writeNumber(*reinterpret_cast<quint8*>(it->rawData()));

                } while (it->nextPixel());
            }
        }
    }
    if (bitmap) {
        flow->flush();
    }
    delete flow;
    return ImportExportCodes::OK;
}

KisPropertiesConfigurationSP KisPPMExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("type", 0);
    return cfg;
}

KisConfigWidget *KisPPMExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    return new KisWdgOptionsPPM(parent);
}


void KisWdgOptionsPPM::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    cmbType->setCurrentIndex(cfg->getInt("type", 0));
}

KisPropertiesConfigurationSP KisWdgOptionsPPM::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("type", cmbType->currentIndex());
    return cfg;

}

void KisPPMExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()->get("ColorModelCheck/" + RGBAColorModelID.id() + "/" + Integer8BitsColorDepthID.id())->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("ColorModelCheck/" + RGBAColorModelID.id() + "/" + Integer16BitsColorDepthID.id())->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("ColorModelCheck/" + GrayAColorModelID.id() + "/" + Integer8BitsColorDepthID.id())->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("ColorModelCheck/" + GrayAColorModelID.id() + "/" + Integer16BitsColorDepthID.id())->create(KisExportCheckBase::SUPPORTED));

    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(RGBAColorModelID, Integer16BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer16BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "PPM");

}



#include "kis_ppm_export.moc"

