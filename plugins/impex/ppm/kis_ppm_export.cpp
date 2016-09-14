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
#include <KisFilterChain.h>
#include <KisImportExportManager.h>

#include <KoDialog.h>

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
    ~KisPPMAsciiFlow() {
    }
    virtual void writeBool(quint8 v) {
        if (v > 127) {
            m_device->write("1 ");
        } else {
            m_device->write("0 ");
        }
    }
    virtual void writeBool(quint16 v) {
        writeBool(quint8(v >> 8));
    }
    virtual void writeNumber(quint8 v) {
        m_device->write(QByteArray::number(v));
        m_device->write(" ");
    }
    virtual void writeNumber(quint16 v) {
        m_device->write(QByteArray::number(v));
        m_device->write(" ");
    }
    virtual void flush() {
    }
private:
    QIODevice* m_device;
};

class KisPPMBinaryFlow : public KisPPMFlow
{
public:
    KisPPMBinaryFlow(QIODevice* device) : m_device(device), m_pos(0), m_current(0) {
    }
    virtual ~KisPPMBinaryFlow() {
    }
    virtual void writeBool(quint8 v) {
        m_current = m_current << 1;
        m_current |= (v > 127);
        ++m_pos;
        if (m_pos >= 8) {
            m_current = 0;
            m_pos = 0;
            flush();
        }
    }
    virtual void writeBool(quint16 v) {
        writeBool(quint8(v >> 8));
    }
    virtual void writeNumber(quint8 v) {
        m_device->write((char*)&v, 1);
    }
    virtual void writeNumber(quint16 v) {
        quint16 vo = qToBigEndian(v);
        m_device->write((char*)&vo, 2);
    }
    virtual void flush() {
        m_device->write((char*)&m_current, 1);
    }
private:
    QIODevice* m_device;
    int m_pos;
    quint8 m_current;
};

KisImportExportFilter::ConversionStatus KisPPMExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{

    KoDialog kdb;
    kdb.setWindowTitle(i18n("PPM Export Options"));
    kdb.setButtons(KoDialog::Ok | KoDialog::Cancel);
    KisConfigWidget *wdg = createConfigurationWidget(&kdb, KisDocument::nativeFormatMimeType(), mimeType());
    kdb.setMainWidget(wdg);
    QApplication::restoreOverrideCursor();

    // If a configuration object was passed to the convert method, we use that, otherwise we load from the settings
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());
    if (configuration) {
        cfg->fromXML(configuration->toXML());
    }
    else {
        cfg = lastSavedConfiguration(KisDocument::nativeFormatMimeType(), mimeType());
    }
    wdg->setConfiguration(cfg);

    if (!getBatchMode()) {
        if (kdb.exec() == QDialog::Rejected) {
            return KisImportExportFilter::UserCancelled;
        }
        cfg = wdg->configuration();
        KisConfig().setExportConfiguration("PPM", *cfg.data());
    }

    bool rgb = (mimeType() == "image/x-portable-pixmap");
    bool binary = (cfg->getInt("type") == 0);

    bool bitmap = (mimeType() == "image/x-portable-bitmap");

    KisImageWSP image = document->image();
    Q_CHECK_PTR(image);
    // the image must be locked at the higher levels
    KIS_SAFE_ASSERT_RECOVER_NOOP(document->image()->locked());
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
    if (rgb) {
        if (binary) io->write("P6");
        else io->write("P3");
    } else if (bitmap) {
        if (binary) io->write("P4");
        else io->write("P1");
    } else {
        if (binary) io->write("P5");
        else io->write("P2");
    }
    io->write("\n");

    // Write the header
    io->write(QByteArray::number(image->width()));
    io->write(" ");
    io->write(QByteArray::number(image->height()));
    if (!bitmap) {
        if (is16bit) io->write(" 65535\n");
        else io->write(" 255\n");
    } else {
        io->write("\n");
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
    return KisImportExportFilter::OK;
}

KisPropertiesConfigurationSP KisPPMExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("type", 0);
    return cfg;
}

KisPropertiesConfigurationSP KisPPMExport::lastSavedConfiguration(const QByteArray &from, const QByteArray &to) const
{
    KisPropertiesConfigurationSP cfg = defaultConfiguration(from, to);
    QString filterConfig = KisConfig().exportConfiguration("PPM");
    cfg->fromXML(filterConfig, false);
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
#include "kis_ppm_export.moc"

