/*
 *
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "panorama.h"

#include <stdlib.h>
#include <algorithm>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kurl.h>

#include <KoColorSpaceRegistry.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_doc2.h"
#include "kis_filter_strategy.h"
#include "kis_global.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_device.h"
#include "kis_types.h"
#include "kis_view2.h"
#include "imageviewer.h"

#include "imagoptim_functions.h"
#include "harris_detector.h"
#include "kis_images_blender.h"
#include "kis_image_alignment.h"
#include "models/kis_image_alignment_projective_model.h"

#include "ui_wdgpanoramacreation.h"

typedef KGenericFactory<PanoramaPlugin> PanoramaPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritapanorama, PanoramaPluginFactory( "krita" ) )


PanoramaPlugin::PanoramaPlugin(QObject *parent, const QStringList &)
    : KParts::Plugin(parent), m_wdgPanoramaCreation(0)
{
    if ( parent->inherits("KisView2") )
    {
        m_view = (KisView2*) parent;

        setComponentData(PanoramaPluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data","kritaplugins/panorama.rc"), true);

        KAction *action  = new KAction(i18n("New Panorama Layer"), this);
        actionCollection()->addAction("PanoramaLayer", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotCreatePanoramaLayer()));
    }
    KisInterestPointsDetector::setInterestPointDetector(0, new HarrisPointDetector);
}

PanoramaPlugin::~PanoramaPlugin()
{
    m_view = 0;
    delete m_wdgPanoramaCreation;
}

void PanoramaPlugin::addImage(const QString& filename)
{
    new QListWidgetItem(filename, m_wdgPanoramaCreation->listImages);
}

void PanoramaPlugin::slotAddImages()
{
//     kDebug(41006) <<"Add image";
    QStringList openfiles = KFileDialog::getOpenFileNames(KUrl(),"*", m_view);
//     kDebug(41006) << openfiles.size() <<" files selected for inclusion";
    QString filename;
    foreach(filename, openfiles)
    {
        addImage( filename );
    }
}

void PanoramaPlugin::slotRemoveImage()
{
    delete m_wdgPanoramaCreation->listImages->takeItem(m_wdgPanoramaCreation->listImages->currentRow());
}

void PanoramaPlugin::slotImageUp()
{
    if (m_wdgPanoramaCreation->listImages->currentRow() == 0) return; 
    QListWidgetItem * w = m_wdgPanoramaCreation->listImages->takeItem(m_wdgPanoramaCreation->listImages->currentRow());
    m_wdgPanoramaCreation->listImages->insertItem(m_wdgPanoramaCreation->listImages->currentRow(), w);
    m_wdgPanoramaCreation->listImages->setCurrentItem(w);
}

void PanoramaPlugin::slotImageDown()
{
    if (m_wdgPanoramaCreation->listImages->currentRow() == m_wdgPanoramaCreation->listImages->count() - 1) return;

    int i = m_wdgPanoramaCreation->listImages->currentRow();
        
    QListWidgetItem * w = m_wdgPanoramaCreation->listImages->takeItem(m_wdgPanoramaCreation->listImages->currentRow());

    if (i == 0) 
        m_wdgPanoramaCreation->listImages->insertItem(m_wdgPanoramaCreation->listImages->currentRow() + 1, w);
    else
        m_wdgPanoramaCreation->listImages->insertItem(m_wdgPanoramaCreation->listImages->currentRow() + 2, w);
    m_wdgPanoramaCreation->listImages->setCurrentItem(w);
}

void PanoramaPlugin::slotCreatePanoramaLayer()
{
    kDebug(41006) <<"Create a panorama layer";
    delete m_wdgPanoramaCreation;
    m_wdgPanoramaCreation = new Ui_WdgPanoramaCreation();
    
    QDialog* dialog = new QDialog(m_view);
    dialog->setWindowTitle(i18n("Create Panorama Layer"));
    dialog->setModal(true);
    m_wdgPanoramaCreation->setupUi(dialog);

//     addImage("/home/cyrille/H0010632.JPG");
//     addImage("/home/cyrille/H0010633.JPG");
//     addImage("/home/cyrille/0.roma.png");
//     addImage("/home/cyrille/1.roma.png");
    addImage("/home/cyrille/0.roma2.png");
    addImage("/home/cyrille/1.roma2.png");
    addImage("/home/cyrille/2.roma2.png");
    addImage("/home/cyrille/3.roma2.png");
//     addImage("/home/cyrille/0.montreal.2.png");
//     addImage("/home/cyrille/1.montreal.2.png");
//     addImage("/home/cyrille/2.montreal.2.png");
/*    addImage("/home/cyrille/0.montreal.png");
    addImage("/home/cyrille/1.montreal.png");
    addImage("/home/cyrille/2.montreal.png");*/
/*   addImage("/home/cyrille/0.graffitis.png");
   addImage("/home/cyrille/1.graffitis.png");
   addImage("/home/cyrille/2.graffitis.png");
   addImage("/home/cyrille/3.graffitis.png");*/
//     addImage("/home/cyrille/0.a.png");
//     addImage("/home/cyrille/1.a.png");

    connect(m_wdgPanoramaCreation->pushButtonCancel, SIGNAL(released()), dialog, SLOT(reject ()));
    connect(m_wdgPanoramaCreation->pushButtonCreatePanorama, SIGNAL(released()), dialog, SLOT(accept ()));
    connect(m_wdgPanoramaCreation->bnAdd, SIGNAL(released()), this, SLOT(slotAddImages()));
    connect(m_wdgPanoramaCreation->bnRemove, SIGNAL(released()), this, SLOT(slotRemoveImage()));
    connect(m_wdgPanoramaCreation->bnUp, SIGNAL(released()), this, SLOT(slotImageUp()));
    connect(m_wdgPanoramaCreation->bnDown, SIGNAL(released()), this, SLOT(slotImageDown()));
    connect(m_wdgPanoramaCreation->pushButtonPreview, SIGNAL(released()), this, SLOT(slotPreview()));


    m_wdgPanoramaCreation->bnAdd->setIcon(SmallIcon("list-add"));
    m_wdgPanoramaCreation->bnRemove->setIcon(SmallIcon("list-remove"));
    m_wdgPanoramaCreation->bnUp->setIcon(SmallIcon("go-up"));
    m_wdgPanoramaCreation->bnDown->setIcon(SmallIcon("go-down"));
    
    if(dialog->exec()==QDialog::Accepted)
    {

        QList<KisImageAlignment::ImageInfo> images;
        for(int i = 0; i < m_wdgPanoramaCreation->listImages->count(); i++)
        {
            QString fileName = m_wdgPanoramaCreation->listImages->item(i)->text();
            kDebug(41006) <<"Loading fileName" << fileName;
            KisDoc2 d;
            d.import(fileName);
            KisImageAlignment::ImageInfo pi;
            KisImageSP img = d.image();
            if(not img) break;
//             img->scale(1000.0 / img->width(), 1000.0 / img->width(), 0, new KisBoxFilterStrategy);
            pi.device = img->projection();
            pi.rect = QRect(0,0, img->width(), img->height());
            images.push_back(pi);
        }

        KisPaintLayerSP layer = new KisPaintLayer(m_view->image(), i18n("Panorama Layer"), OPACITY_OPAQUE, images[0].device->colorSpace());
        Q_ASSERT(layer);
        KisGroupLayerSP parent;
        KisLayerSP above;
        if (KisGroupLayer* pactive = qobject_cast<KisGroupLayer*>(m_view->activeLayer().data()))
        {
            parent = pactive;
        }
        if(!parent)
            parent = m_view->image()->rootLayer();
        above = qobject_cast<KisLayer*>( parent->firstChild().data() );
        m_view->image()->addNode(layer.data(), parent.data(), above.data());


        QRect dstArea;
        createPanorama(images, layer->paintDevice(), dstArea);
        m_view->image()->rootLayer()->setDirty();
    }
    delete dialog;
}

void PanoramaPlugin::slotPreview()
{
    QList<KisImageAlignment::ImageInfo> images;
    for(int i = 0; i < m_wdgPanoramaCreation->listImages->count(); i++)
    {
        QString fileName = m_wdgPanoramaCreation->listImages->item(i)->text();
        kDebug(41006) <<"Loading fileName" << fileName;
        KisDoc2 d;
        d.import(fileName);
        KisImageAlignment::ImageInfo pi;
        KisImageSP img = d.image();
        if(not img) break;
//         img->scale(1000.0 / img->width(), 1000.0 / img->width(), 0, new KisBoxFilterStrategy);
//         img->scale(1000.0 / img->width(), 1000.0 / img->width(), 0, new KisMitchellFilterStrategy );
        pi.device = img->projection();
        pi.rect = QRect(0,0, img->width(), img->height());
        images.push_back(pi);
    }
    KisPaintDeviceSP dst = new KisPaintDevice( images[0].device->colorSpace(), "panorama preview" );
    QRect dstArea;
    createPanorama(images, dst, dstArea);
    QImage img = dst->convertToQImage(0);
    img = img.scaledToHeight(500);
    m_wdgPanoramaCreation->wdgPreview->setImage(img);
    //m_wdgPanoramaCreation->labelPreview->setPixmap(QPixmap::fromImage(img));
}

void PanoramaPlugin::createPanorama(QList<KisImageAlignment::ImageInfo>& images, KisPaintDeviceSP dstdevice, QRect& )
{
    KisImageAlignment ia( new KisImageAlignmentProjectiveModel, KisInterestPointsDetector::interestPointDetector());
    std::vector< KisImageAlignment::Result > p = ia.align(images);
    std::cout << "Number of results = " << p.size() << std::endl;
    // blend
    QList<KisImagesBlender::LayerSource> sources;
    for(int i = 0; i < (int)p.size(); i++)
    {
        KisImagesBlender::LayerSource layerSource;
        layerSource.layer = images[ i ].device;
        layerSource.a = p[ i ].a;
        layerSource.b = p[ i ].b;
        layerSource.c = p[ i ].c;
        layerSource.xc1 = images[ i ].rect.width()  * 0.5;
        layerSource.xc2 = images[ i ].rect.width()  * 0.5;
        layerSource.yc1 = images[ i ].rect.height() * 0.5;
        layerSource.yc2 = images[ i ].rect.height() * 0.5;
        layerSource.norm = (4.0 / ( images[ i ].rect.width() * images[ i ].rect.width() + images[ i ].rect.height() * images[ i ].rect.height() ) );
        layerSource.homography = p[ i ].homography;
        layerSource.rect = images[ i ].rect;
        sources.push_back(layerSource);
    }
#if 0
    // First layer
    KisImagesBlender::LayerSource firstLayer;
    firstLayer.layer = images[0].device;
    firstLayer.a = p[0].a;
    firstLayer.b = p[0].b;
    firstLayer.c = p[0].c;
    firstLayer.xc1 = images[0].rect.width()  * 0.5;
    firstLayer.xc2 = images[0].rect.width()  * 0.5;
    firstLayer.yc1 = images[0].rect.height() * 0.5;
    firstLayer.yc2 = images[0].rect.height() * 0.5;
    firstLayer.norm = (4.0 / ( images[0].rect.width() * images[0].rect.width() + images[0].rect.height() * images[0].rect.height() ) );
    firstLayer.homography = p[0].homography;
    firstLayer.rect = images[0].rect;
    sources.push_back(firstLayer);
    // Second Layer
    KisImagesBlender::LayerSource secondLayer;
    secondLayer.layer = images[1].device;
    secondLayer.a = p[1].a;
    secondLayer.b = p[1].b;
    secondLayer.c = p[1].c;
    secondLayer.xc1 = images[1].rect.width()  * 0.5;
    secondLayer.xc2 = images[1].rect.width()  * 0.5;
    secondLayer.yc1 = images[1].rect.height() * 0.5;
    secondLayer.yc2 = images[1].rect.height() * 0.5;
    secondLayer.norm = (4.0 / ( images[1].rect.width() * images[1].rect.width() + images[1].rect.height() * images[1].rect.height() ) );
    secondLayer.homography = p[1].homography;
    secondLayer.rect = images[1].rect;
    sources.push_back(secondLayer);
#endif
    KisImagesBlender::blend(sources, dstdevice);
}

#include "panorama.moc"
