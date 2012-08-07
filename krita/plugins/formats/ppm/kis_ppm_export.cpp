/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_ppm_export.h"

#include <KPluginFactory>
#include <KApplication>

#include <KoColorSpace.h>
#include <KoColorSpaceConstants.h>
#include <KoFilterChain.h>
#include <KoFilterManager.h>

#include <KDialog>
#include <KMessageBox>

#include <kis_debug.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_properties_configuration.h>
#include <kis_config.h>

#include "ui_kis_wdg_options_ppm.h"
#include <qendian.h>
#include <KoColorSpaceTraits.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include "kis_iterator_ng.h"

K_PLUGIN_FACTORY(KisPPMExportFactory, registerPlugin<KisPPMExport>();)
K_EXPORT_PLUGIN(KisPPMExportFactory("krita"))

KisPPMExport::KisPPMExport(QObject *parent, const QVariantList &) : KoFilter(parent)
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

KoFilter::ConversionStatus KisPPMExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "PPM export! From:" << from << ", To:" << to << "";

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    KisDoc2 *output = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    if (!output)
        return KoFilter::CreationError;

    if (filename.isEmpty()) return KoFilter::FileNotFound;

    KDialog* kdb = new KDialog(0);
    kdb->setWindowTitle(i18n("PPM Export Options"));
    kdb->setButtons(KDialog::Ok | KDialog::Cancel);

    Ui::WdgOptionsPPM optionsPPM;

    QWidget* wdg = new QWidget(kdb);
    optionsPPM.setupUi(wdg);

    kdb->setMainWidget(wdg);
    kapp->restoreOverrideCursor();

    QString filterConfig = KisConfig().exportConfiguration("PPM");
    KisPropertiesConfiguration cfg;
    cfg.fromXML(filterConfig);

    optionsPPM.type->setCurrentIndex(cfg.getInt("type", 0));

    if (!m_chain->manager()->getBatchMode()) {
        if (kdb->exec() == QDialog::Rejected) {
            return KoFilter::OK; // FIXME Cancel doesn't exist :(
        }
    }

    bool rgb = (to == "image/x-portable-pixmap");
    bool binary = optionsPPM.type->currentIndex() == 0;
    cfg.setProperty("type", optionsPPM.type->currentIndex());
    KisConfig().setExportConfiguration("PPM", cfg);

    bool bitmap = (to == "image/x-portable-bitmap");

    KisImageWSP image = output->image();
    Q_CHECK_PTR(image);
    image->refreshGraph();
    image->lock();
    KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());
    image->unlock();

    // Test color space
    if (((rgb && (pd->colorSpace()->id() != "RGBA" && pd->colorSpace()->id() != "RGBA16"))
            || (!rgb && (pd->colorSpace()->id() != "GRAYA" && pd->colorSpace()->id() != "GRAYA16")))) {
        if (rgb) {
            pd->convertTo(KoColorSpaceRegistry::instance()->rgb8(0), KoColorConversionTransformation::IntentPerceptual, KoColorConversionTransformation::BlackpointCompensation);
        }
        else {
            pd->convertTo(KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), 0), KoColorConversionTransformation::IntentPerceptual, KoColorConversionTransformation::BlackpointCompensation);
        }
    }

    bool is16bit = pd->colorSpace()->id() == "RGBA16" || pd->colorSpace()->id() == "GRAYA16";

    // Open the file for writing
    QFile fp(filename);
    fp.open(QIODevice::WriteOnly);

    // Write the magic
    if (rgb) {
        if (binary) fp.write("P6");
        else fp.write("P3");
    } else if (bitmap) {
        if (binary) fp.write("P4");
        else fp.write("P1");
    } else {
        if (binary) fp.write("P5");
        else fp.write("P2");
    }
    fp.write("\n");

    // Write the header
    fp.write(QByteArray::number(image->width()));
    fp.write(" ");
    fp.write(QByteArray::number(image->height()));
    if (!bitmap) {
        if (is16bit) fp.write(" 65535\n");
        else fp.write(" 255\n");
    } else {
        fp.write("\n");
    }

    // Write the data
    KisPPMFlow* flow = 0;
    if (binary) flow = new KisPPMBinaryFlow(&fp);
    else flow = new KisPPMAsciiFlow(&fp);

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
    fp.close();
    return KoFilter::OK;
}
