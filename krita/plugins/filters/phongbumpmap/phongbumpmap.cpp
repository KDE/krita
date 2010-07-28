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

/*
#include <../../extensions/impasto/kis_fresh_start_color_space.h>
#include <../../extensions/impasto/kis_fresh_start_color_space.cpp>
*/

#include "kdebug.h"

#include <../../extensions/impasto/kis_rgbu8_height_color_space.h>
#include <../../extensions/impasto/kis_rgbu8_height_color_space.cpp>


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
    /*NOTE: THIS FILTER HASN'T BEEN EITHER OPTIMIZED, OR GIVEN MUCH THOUGHT,
    IT'S STILL PRE-ALPHA, ITS SPAGETTINESS IS EVIDENT AND ITS INEFFICIENCY TOO.
    NOT POINTING OUT THE OBVIOUS IS APPRECIATED, THANK YOU.*/
    
    //qDebug("---- LE FILTER, LE FILTER, LE FILTER, LE FILTER, LE FILTER, LE FILTER ----");
    
    QImage bumpmap;
    QTime timer;
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
    
    //qDebug() << srcInfo.paintDevice()->colorSpace()->colorModelId();
    /*  ------- Debugging tools
    QList<KoID> AVER = KoColorSpaceRegistry::instance()->listKeys();
    
    foreach (KoID mierda, AVER)
        qDebug() << mierda ;
    */
    
    src = srcInfo.paintDevice();
    dst = dstInfo.paintDevice();
    
    lim = src->exactBounds();
    
    Channels = src->readPlanarBytes(lim.x(), lim.y(), lim.width(), lim.height());
    
    foreach (KoChannelInfo* channel, src->colorSpace()->channels()) {
        if (userChosenHeightChannel == channel->name())
            m_heightChannel = channel;
    }
    
    quint8* heightmap = Channels.data()[m_heightChannel->index()];
    
    QVector3D reflection_vector(0, 0, 0);
    QVector3D normal_vector(0, 0, 1);
    QVector3D x_vector(1, 0, 0);
    QVector3D y_vector(0, 1, 0);
    QVector3D light_vector(-6, -6, 3);
    light_vector.normalize();
    QVector3D vision_vector(0, 0, 1);
    
    qreal Il, Ka, Kd, Ks, shiny_exp, I, Ia, Id, Is;
    
    Il = 0xFF;
    Ka = 0.0;
    Kd = 1;
    Ks = 1;
    shiny_exp = 40;
    
    I = Ia = Id = Is = 0;
    
    qDebug("Time elapsed: %d ms", timer.elapsed());
    
    quint32 posup, posdown, posleft, posright;
    quint32 x, y;
    x = y = 0;
    //=======================================
    //================Fill===================
    //=======================================
    
    qDebug("VEAMOS SI ESTO EXISTE REALMENTE");
    
    if ((lim.width() == 1) && (lim.height() == 1))
        return;
    quint32 width_minus_1 = lim.width() - 1;
    quint32 height_minus_1 = lim.height() - 1;
    
    // calculate corner (0,0)
    posup = lim.width();
    posdown = 0;
    posleft = 0;
    posright = 1;
    
    normal_vector.setX(- heightmap[posright] + heightmap[posleft]);
    normal_vector.setY(- heightmap[posup] + heightmap[posdown]);
    normal_vector.setZ(4);
    normal_vector.normalize();
    reflection_vector = (2 * pow(QVector3D::dotProduct(normal_vector, light_vector), shiny_exp)) * normal_vector - light_vector;
    Ia = Ka * Il;
    Id = Kd * Il * QVector3D::dotProduct(normal_vector, light_vector);
    if (Id < 0)     Id = 0;
    Is = Ks * Il * QVector3D::dotProduct(vision_vector, reflection_vector);
    if (Is < 0)     Is = 0;
    I = Ia + Id + Is;
    if (I > 0xFF)   I = 0xFF;
    if (I < 0)      I = 0;
    dst->setPixel(0, 0, QColor(I, I, I));  // TODO: Inefficient, hacky, silly, will fix
    
    // calculate corner (width_minus_1,0)
    posup = lim.width() + width_minus_1;
    posdown = width_minus_1;
    posleft = width_minus_1 - 1;
    posright = width_minus_1;
    
    normal_vector.setX(- heightmap[posright] + heightmap[posleft]);
    normal_vector.setY(- heightmap[posup] + heightmap[posdown]);
    normal_vector.setZ(4);
    normal_vector.normalize();
    reflection_vector = (2 * pow(QVector3D::dotProduct(normal_vector, light_vector), shiny_exp)) * normal_vector - light_vector;
    Ia = Ka * Il;
    Id = Kd * Il * QVector3D::dotProduct(normal_vector, light_vector);
    if (Id < 0)     Id = 0;
    Is = Ks * Il * QVector3D::dotProduct(vision_vector, reflection_vector);
    if (Is < 0)     Is = 0;
    I = Ia + Id + Is;
    if (I > 0xFF)   I = 0xFF;
    if (I < 0)      I = 0;
    dst->setPixel(width_minus_1, 0, QColor(I, I, I));  // TODO: Inefficient, hacky, silly, will fix
    
    // calculate corner (0,height_minus_1)
    posup = (height_minus_1) * lim.width();
    posdown = (height_minus_1 - 1) * lim.width();
    posleft =  height_minus_1 * lim.width();
    posright = height_minus_1  * lim.width() + 1;
    
    normal_vector.setX(- heightmap[posright] + heightmap[posleft]);
    normal_vector.setY(- heightmap[posup] + heightmap[posdown]);
    normal_vector.setZ(4);
    normal_vector.normalize();
    reflection_vector = (2 * pow(QVector3D::dotProduct(normal_vector, light_vector), shiny_exp)) * normal_vector - light_vector;
    Ia = Ka * Il;
    Id = Kd * Il * QVector3D::dotProduct(normal_vector, light_vector);
    if (Id < 0)     Id = 0;
    Is = Ks * Il * QVector3D::dotProduct(vision_vector, reflection_vector);
    if (Is < 0)     Is = 0;
    I = Ia + Id + Is;
    if (I > 0xFF)   I = 0xFF;
    if (I < 0)      I = 0;
    dst->setPixel(0, height_minus_1, QColor(I, I, I));  // TODO: Inefficient, hacky, silly, will fix
    
    // calculate corner (width_minus_1,height_minus_1)
    posup = (height_minus_1) * lim.width() + width_minus_1;
    posdown = (height_minus_1 - 1) * lim.width() + width_minus_1;
    posleft =  height_minus_1 * lim.width() + width_minus_1 - 1;
    posright = height_minus_1  * lim.width() + width_minus_1;
    
    normal_vector.setX(- heightmap[posright] + heightmap[posleft]);
    normal_vector.setY(- heightmap[posup] + heightmap[posdown]);
    normal_vector.setZ(4);
    normal_vector.normalize();
    reflection_vector = (2 * pow(QVector3D::dotProduct(normal_vector, light_vector), shiny_exp)) * normal_vector - light_vector;
    Ia = Ka * Il;
    Id = Kd * Il * QVector3D::dotProduct(normal_vector, light_vector);
    if (Id < 0)     Id = 0;
    Is = Ks * Il * QVector3D::dotProduct(vision_vector, reflection_vector);
    if (Is < 0)     Is = 0;
    I = Ia + Id + Is;
    if (I > 0xFF)   I = 0xFF;
    if (I < 0)      I = 0;
    dst->setPixel(width_minus_1, height_minus_1, QColor(I, I, I));  // TODO: Inefficient, hacky, silly, will fix
    
    
    for (y = 1; y < height_minus_1; y++) {
        // calculate edge (0, y)
        posup = (y + 1) * lim.width();
        posdown = (y - 1) * lim.width();
        posleft =  y * lim.width();
        posright = y * lim.width() + 1;
        
        normal_vector.setX(- heightmap[posright] + heightmap[posleft]);
        normal_vector.setY(- heightmap[posup] + heightmap[posdown]);
        normal_vector.setZ(4);
        normal_vector.normalize();
        reflection_vector = (2 * pow(QVector3D::dotProduct(normal_vector, light_vector), shiny_exp)) * normal_vector - light_vector;
        Ia = Ka * Il;
        Id = Kd * Il * QVector3D::dotProduct(normal_vector, light_vector);
        if (Id < 0)     Id = 0;
        Is = Ks * Il * QVector3D::dotProduct(vision_vector, reflection_vector);
        if (Is < 0)     Is = 0;
        I = Ia + Id + Is;
        if (I > 0xFF)   I = 0xFF;
        if (I < 0)      I = 0;
        dst->setPixel(0, y, QColor(I, I, I));  // TODO: Inefficient, hacky, silly, will fix
        
        // calculate edge (width_minus_1, y)
        posup = (y + 1) * lim.width() + width_minus_1;
        posdown = (y - 1) * lim.width() + width_minus_1;
        posleft =  y * lim.width() + width_minus_1 - 1;
        posright = y  * lim.width() + width_minus_1;
        
        normal_vector.setX(- heightmap[posright] + heightmap[posleft]);
        normal_vector.setY(- heightmap[posup] + heightmap[posdown]);
        normal_vector.setZ(4);
        normal_vector.normalize();
        reflection_vector = (2 * pow(QVector3D::dotProduct(normal_vector, light_vector), shiny_exp)) * normal_vector - light_vector;
        Ia = Ka * Il;
        Id = Kd * Il * QVector3D::dotProduct(normal_vector, light_vector);
        if (Id < 0)     Id = 0;
        Is = Ks * Il * QVector3D::dotProduct(vision_vector, reflection_vector);
        if (Is < 0)     Is = 0;
        I = Ia + Id + Is;
        if (I > 0xFF)   I = 0xFF;
        if (I < 0)      I = 0;
        dst->setPixel(width_minus_1, y, QColor(I, I, I));  // TODO: Inefficient, hacky, silly, will fix
    }
    for (x = 1; x < width_minus_1; x++) {
        // calculate edge (x, 0)
        posup = lim.width() + x;
        posdown = -lim.width() + x;
        posleft = x - 1;
        posright = x + 1;
        
        normal_vector.setX(- heightmap[posright] + heightmap[posleft]);
        normal_vector.setY(- heightmap[posup] + heightmap[posdown]);
        normal_vector.setZ(4);
        normal_vector.normalize();
        reflection_vector = (2 * pow(QVector3D::dotProduct(normal_vector, light_vector), shiny_exp)) * normal_vector - light_vector;
        Ia = Ka * Il;
        Id = Kd * Il * QVector3D::dotProduct(normal_vector, light_vector);
        if (Id < 0)     Id = 0;
        Is = Ks * Il * QVector3D::dotProduct(vision_vector, reflection_vector);
        if (Is < 0)     Is = 0;
        I = Ia + Id + Is;
        if (I > 0xFF)   I = 0xFF;
        if (I < 0)      I = 0;
        dst->setPixel(x, 0, QColor(I, I, I));  // TODO: Inefficient, hacky, silly, will fix
        
        // calculate edge (x, height_minus_1)
        posup = (height_minus_1) * lim.width() + x;
        posdown = (height_minus_1 - 1) * lim.width() + x;
        posleft =  height_minus_1 * lim.width() + x - 1;
        posright = height_minus_1 * lim.width() + x + 1;
        
        normal_vector.setX(- heightmap[posright] + heightmap[posleft]);
        normal_vector.setY(- heightmap[posup] + heightmap[posdown]);
        normal_vector.setZ(4);
        normal_vector.normalize();
        reflection_vector = (2 * pow(QVector3D::dotProduct(normal_vector, light_vector), shiny_exp)) * normal_vector - light_vector;
        Ia = Ka * Il;
        Id = Kd * Il * QVector3D::dotProduct(normal_vector, light_vector);
        if (Id < 0)     Id = 0;
        Is = Ks * Il * QVector3D::dotProduct(vision_vector, reflection_vector);
        if (Is < 0)     Is = 0;
        I = Ia + Id + Is;
        if (I > 0xFF)   I = 0xFF;
        if (I < 0)      I = 0;
        dst->setPixel(x, height_minus_1, QColor(I, I, I));  // TODO: Inefficient, hacky, silly, will fix
        
        for (y = 1; y < height_minus_1; y++) {
            // calculate interior (x, y)
            posup = (y + 1) * lim.width() + x;
            posdown = (y - 1) * lim.width() + x;
            posleft =  y * lim.width() + x - 1;
            posright = y  * lim.width() + x + 1;
            
            normal_vector.setX(- heightmap[posright] + heightmap[posleft]);
            normal_vector.setY(- heightmap[posup] + heightmap[posdown]);
            normal_vector.setZ(4);
            normal_vector.normalize();
            reflection_vector = (2 * pow(QVector3D::dotProduct(normal_vector, light_vector), shiny_exp)) * normal_vector - light_vector;
            Ia = Ka * Il;
            Id = Kd * Il * QVector3D::dotProduct(normal_vector, light_vector);
            if (Id < 0)     Id = 0;
            Is = Ks * Il * QVector3D::dotProduct(vision_vector, reflection_vector);
            if (Is < 0)     Is = 0;
            I = Ia + Id + Is;
            if (I > 0xFF)   I = 0xFF;
            if (I < 0)      I = 0;
            dst->setPixel(x, y, QColor(I, I, I));  // TODO: Inefficient, hacky, silly, will fix
        }
    }
    qDebug("Time elapsed: %d ms", timer.elapsed());
    
    //bumpmap.save("/home/pentalis/GSoC/relieve_filtrado.bmp");
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
