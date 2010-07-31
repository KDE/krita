/*
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "phongbumpmap.h"

#include <stdlib.h>

#include <QButtonGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QPoint>
#include <QPushButton>
#include <QString>

#include <kcomponentdata.h>
#include <kpluginfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <KoColorTransformation.h>
#include <KoIntegerMaths.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoDocumentSectionView.h>

#include <kis_debug.h>
#include <kis_doc2.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <kis_node_model.h>

#include <QPixmap>
#include <QImage>
#include <QDebug>
#include <QTime>
#include <QVector3D>
#include <QVector>
#include <cmath>
#include <iostream>

#include <time.h>

#include <KoColorSpaceRegistry.h>
#include <colorprofiles/KoDummyColorProfile.h>

#include <QColor>

/*
#include <../../extensions/impasto/kis_fresh_start_color_space.h>
#include <../../extensions/impasto/kis_fresh_start_color_space.cpp>
*/

#include "kdebug.h"


K_PLUGIN_FACTORY(KritaPhongBumpmapFactory, registerPlugin<KritaPhongBumpmap>();)
K_EXPORT_PLUGIN(KritaPhongBumpmapFactory("krita"))


KritaPhongBumpmap::KritaPhongBumpmap(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    //setComponentData(KritaPhongBumpmapFactory::componentData());
    KisFilterRegistry::instance()->add(new KisFilterPhongBumpmap());

}

KritaPhongBumpmap::~KritaPhongBumpmap()
{
}

void KisFilterPhongBumpmap::process(KisConstProcessingInformation srcInfo,
                                    KisProcessingInformation dstInfo,
                                    const QSize& size,
                                    const KisFilterConfiguration* config,
                                    KoUpdater* progressUpdater
                                    ) const
{
    
    QTime timer, timerE;
    QPoint pos;
    QVector<quint8*> Channels;
    KisPaintDeviceSP src;
    KisPaintDeviceSP dst;
    QRect lim;
    QString userChosenHeightChannel = config->getString("heightChannel", "FAIL");
    KoChannelInfo* m_heightChannel;
    
    if (userChosenHeightChannel == "FAIL") {
        qDebug("FIX YOUR FILTER");
        return;
    }
        
    timer.start();
    
    quint32 demora_proceso[] = {0, 0, 0};
    
    src = srcInfo.paintDevice();
    dst = dstInfo.paintDevice();
    
    lim = src->exactBounds();
    
    Channels = src->readPlanarBytes(lim.x(), lim.y(), lim.width(), lim.height());
    
    QImage bumpmap(lim.width(), lim.height(), QImage::Format_RGB32);
    bumpmap.fill(0);
    
    foreach (KoChannelInfo* channel, src->colorSpace()->channels()) {
        if (userChosenHeightChannel == channel->name())
            m_heightChannel = channel;
    }
    
    quint8* heightmap = Channels.data()[m_heightChannel->index()];
    
    qDebug("Tiempo de preparacion: %d ms", timer.restart());
    
    quint32 posup;
    quint32 posdown;
    quint32 posleft;
    quint32 posright;
    quint32 x, y;
    
    QColor I; //Reflected light
    
    quint8** fastHeightmap = new quint8*[lim.height()];
    
    for (int yIndex = 0; yIndex < lim.height(); yIndex++) {
        
        quint8* fastLine = new quint8[lim.width()];
        
        for (int xIndex = 0; xIndex < lim.width(); xIndex++) {
            fastLine[xIndex] = heightmap[xIndex + yIndex * lim.width()];
        }
        
        fastHeightmap[yIndex] = fastLine;
    }
    
    PhongPixelProcessor *pixelProcessor = new PhongPixelProcessor(heightmap);
    PhongPixelProcessor *fastPixelProcessor = new PhongPixelProcessor(fastHeightmap);
    
    //======================================
    //======Preparation paraphlenalia=======
    //======================================

    //int pixelsPerLine = bumpmap.bytesPerLine() / 4;
    
    QRgb** lines = new QRgb*[bumpmap.height()];

    for (int yIndex = 0; yIndex < bumpmap.height(); yIndex++)
        lines[yIndex] = (QRgb *) bumpmap.scanLine(yIndex);
        
    qDebug("Tiempo de pointeracion: %d ms", timer.restart());

    //=======================================
    //================Fill===================
    //=======================================
    
    if ((lim.width() == 1) && (lim.height() == 1))
        return;
    quint32 width_minus_1 = lim.width() - 1;
    quint32 height_minus_1 = lim.height() - 1;
    
    // calculate corner (0,0)
    posup = lim.width();
    posdown = 0;
    posleft = 0;
    posright = 1;
    
    I = pixelProcessor->illuminatePixel(posup, posdown, posleft, posright);
    lines[0][0] = I.rgb();
    //dst->setPixel(0, 0, I);  // TODO: Inefficient, hacky, silly, will fix
    
    // calculate corner (width_minus_1,0)
    posup = lim.width() + width_minus_1;
    posdown = width_minus_1;
    posleft = width_minus_1 - 1;
    posright = width_minus_1;
    
    I = pixelProcessor->illuminatePixel(posup, posdown, posleft, posright);
    lines[0][width_minus_1] = I.rgb();
    //dst->setPixel(width_minus_1, 0, I);  // TODO: Inefficient, hacky, silly, will fix
    
    // calculate corner (0,height_minus_1)
    posup = (height_minus_1) * lim.width();
    posdown = (height_minus_1 - 1) * lim.width();
    posleft =  height_minus_1 * lim.width();
    posright = height_minus_1  * lim.width() + 1;
    
    I = pixelProcessor->illuminatePixel(posup, posdown, posleft, posright);
    lines[height_minus_1][0] = I.rgb();
    //dst->setPixel(0, height_minus_1, I);  // TODO: Inefficient, hacky, silly, will fix
    
    // calculate corner (width_minus_1,height_minus_1)
    posup = (height_minus_1) * lim.width() + width_minus_1;
    posdown = (height_minus_1 - 1) * lim.width() + width_minus_1;
    posleft =  height_minus_1 * lim.width() + width_minus_1 - 1;
    posright = height_minus_1  * lim.width() + width_minus_1;
    
    I = pixelProcessor->illuminatePixel(posup, posdown, posleft, posright);
    lines[height_minus_1][width_minus_1] = I.rgb();
    //dst->setPixel(width_minus_1, height_minus_1, I);  // TODO: Inefficient, hacky, silly, will fix
    
    
    for (y = 1; y < height_minus_1; y++) {
        // calculate edge (0, y)
        posup = (y + 1) * lim.width();
        posdown = (y - 1) * lim.width();
        posleft =  y * lim.width();
        posright = y * lim.width() + 1;
        
        I = pixelProcessor->illuminatePixel(posup, posdown, posleft, posright);
        lines[y][0] = I.rgb();
        //dst->setPixel(0, y, I);  // TODO: Inefficient, hacky, silly, will fix
        
        // calculate edge (width_minus_1, y)
        posup = (y + 1) * lim.width() + width_minus_1;
        posdown = (y - 1) * lim.width() + width_minus_1;
        posleft =  y * lim.width() + width_minus_1 - 1;
        posright = y * lim.width() + width_minus_1;
        
        I = pixelProcessor->illuminatePixel(posup, posdown, posleft, posright);
        lines[y][width_minus_1] = I.rgb();
        //dst->setPixel(width_minus_1, y, I);  // TODO: Inefficient, hacky, silly, will fix
    }
    for (x = 1; x < width_minus_1; x++) {
        // calculate edge (x, 0)
        posup = lim.width() + x;
        posdown = x;
        posleft = x - 1;
        posright = x + 1;
        
        I = pixelProcessor->illuminatePixel(posup, posdown, posleft, posright);
        lines[0][x] = I.rgb();
        //dst->setPixel(x, 0, I);  // TODO: Inefficient, hacky, silly, will fix
        
        // calculate edge (x, height_minus_1)
        posup = (height_minus_1) * lim.width() + x;
        posdown = (height_minus_1 - 1) * lim.width() + x;
        posleft =  height_minus_1 * lim.width() + x - 1;
        posright = height_minus_1 * lim.width() + x + 1;
        
        I = pixelProcessor->illuminatePixel(posup, posdown, posleft, posright);
        lines[height_minus_1][x] = I.rgb();
        //dst->setPixel(x, height_minus_1, I);  // TODO: Inefficient, hacky, silly, will fix
        
        for (y = 1; y < height_minus_1; y++) {
            //timerE.start();
            // calculate interior (x, y)
            //demora_proceso[0] += timerE.restart();
            
            //QRgb yay = fastPixelProcessor->fastIlluminatePixel(QPoint(x, y+1), QPoint(x, y-1), QPoint(x-1, y), QPoint(x+1, y));
            //demora_proceso[1] += timerE.restart();
            //lines[y][x] = yay;
            //dst->setPixel(x, y, I);  // TODO: Inefficient, hacky, silly, will fix
            //demora_proceso[2] += timerE.elapsed();
            
            /*
            posup = (y + 1) * lim.width() + x;
            posdown = (y - 1) * lim.width() + x;
            posleft =  y * lim.width() + x - 1;
            posright = y  * lim.width() + x + 1;
            
            I = pixelProcessor->illuminatePixel(posup, posdown, posleft, posright);
            lines[y][x] = I.rgb();
            */
            lines[y][x] = fastPixelProcessor->reallyFastIlluminatePixel(x, y+1, x, y-1, x-1, y, x+1, y);
        }
    }
    qDebug("Tiempo de calculo: %d ms", timer.restart());
    dst->convertFromQImage(bumpmap, "");
    
    qDebug("Tiempo deconversion: %d ms", timer.elapsed());
    
    //qDebug("Tiempo gastado en calcular indices: %d ms", demora_proceso[0]);
    //qDebug("Tiempo gastado en procesar pixeles: %d ms", demora_proceso[1]);
    //qDebug("Tiempo gastado en asignar pixeles: %d ms", demora_proceso[2]);
    //bumpmap.save("/home/pentalis/GSoC/relieve_filtrado.bmp");
    
    delete pixelProcessor;
    delete lines;
}



KisFilterPhongBumpmap::KisFilterPhongBumpmap()
                      : KisFilter(KoID("phongbumpmap", i18n("PhongBumpmap")), KisFilter::categoryMap(), i18n("&PhongBumpmap..."))
{
    setColorSpaceIndependence(TO_LAB16);
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(true);
}


KisFilterConfiguration* KisFilterPhongBumpmap::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id(), 0);
    return config;
}













KisConfigWidget * KisFilterPhongBumpmap::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const
{
    KisPhongBumpmapConfigWidget * w = new KisPhongBumpmapConfigWidget(dev, image, parent);

    return w;
}










KisPhongBumpmapConfigWidget::KisPhongBumpmapConfigWidget(const KisPaintDeviceSP dev, const KisImageWSP image, QWidget * parent, Qt::WFlags f)
                            : KisConfigWidget(parent, f)
                            , m_device(dev)
                            , m_image(image)
{
    Q_ASSERT(m_device);

    m_page = new PhongBumpmapWidget(this);

    QHBoxLayout * l = new QHBoxLayout(this);
    Q_CHECK_PTR(l);

    l->addWidget(m_page);

    m_model = new KisNodeModel(this);
    m_model->setImage(image);

    /* fill in the channel chooser */
    QList<KoChannelInfo *> channels = m_device->colorSpace()->channels();
    for (quint8 ch = 0; ch < m_device->colorSpace()->colorChannelCount(); ch++)
        m_page->heightChannelComboBox->addItem(channels.at(ch)->name());
}

void KisPhongBumpmapConfigWidget::setConfiguration(const KisPropertiesConfiguration * cfg)
{
    if (!cfg) return;

    //KisNodeSP node = cfg->getProperty("source_layer").value<KisNodeSP>();
    //if (node)
    //    m_page->listLayers->setCurrentIndex(m_model->indexFromNode(node));
    
    //m_page->heightChannelComboBox->setValue(cfg->getString("heightChannel", "FAIL"));
    
}

KisPropertiesConfiguration* KisPhongBumpmapConfigWidget::configuration() const
{
    KisFilterConfiguration * cfg = new KisFilterConfiguration("phongbumpmap", 2);
    
    //QVariant v;
    //v.fromValue(m_model->nodeFromIndex(m_page->listLayers->currentIndex()));
    
    //cfg->setProperty("source_layer", v);
    cfg->setProperty("heightChannel", m_page->heightChannelComboBox->currentText());
    
    return cfg;
}

#include "phongbumpmap.moc"
