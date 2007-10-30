/*
 *
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

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

#include "eigen/vector.h"
#include "eigen/matrix.h"

#include "harris_detector.h"
#include "matching.h"
#include "ransac.h"
#include "panoptim_p.h"
#include "imagematchmodel_p.h"
#include "homographyimagematchmodel_p.h"
#include "kis_images_blender.h"
#include "hessiandetector/kis_hessian_detector.h"

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

        KAction *action  = new KAction(i18n("New panorama layer"), this);
        actionCollection()->addAction("PanoramaLayer", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotCreatePanoramaLayer()));
    }
    KisInterestPointsDetector::setInterestPointDetector(0, new HarrisPointDetector);
//     KisInterestPointsDetector::setInterestPointDetector(1, new KisHessianDetector());
}

PanoramaPlugin::~PanoramaPlugin()
{
    m_view = 0;
    delete m_wdgPanoramaCreation;
}

void PanoramaPlugin::addImage(const QString& filename)
{
    m_wdgPanoramaCreation->listImages->addItem(filename);
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

void PanoramaPlugin::slotCreatePanoramaLayer()
{
    kDebug(41006) <<"Create a panorama layer";
    delete m_wdgPanoramaCreation;
    m_wdgPanoramaCreation = new Ui_WdgPanoramaCreation();
    QDialog* dialog = new QDialog(m_view);
    dialog->setModal(true);
    m_wdgPanoramaCreation->setupUi(dialog);

/*    addImage("/home/cyrille/H0010632.JPG");
    addImage("/home/cyrille/H0010633.JPG");*/
    addImage("/home/cyrille/0.png");
    addImage("/home/cyrille/1.png");
/*    addImage("/home/cyrille/0.a.png");
    addImage("/home/cyrille/1.a.png");*/

    connect(m_wdgPanoramaCreation->pushButtonCancel, SIGNAL(released()), dialog, SLOT(reject ()));
    connect(m_wdgPanoramaCreation->pushButtonCreatePanorama, SIGNAL(released()), dialog, SLOT(accept ()));
    connect(m_wdgPanoramaCreation->pushButtonAddImages, SIGNAL(released()), this, SLOT(slotAddImages()));
    connect(m_wdgPanoramaCreation->pushButtonPreview, SIGNAL(released()), this, SLOT(slotPreview()));

    if(dialog->exec()==QDialog::Accepted)
    {

        QList<PanoramaImage> images;
        for(int i = 0; i < m_wdgPanoramaCreation->listImages->count(); i++)
        {
            QString fileName = m_wdgPanoramaCreation->listImages->item(i)->text();
            kDebug(41006) <<"Loading fileName" << fileName;
            KisDoc2 d;
            d.import(fileName);
            PanoramaImage pi;
            KisImageSP img = d.image();
            if(not img) break;
//             img->scale(1000.0 / img->width(), 1000.0 / img->width(), 0, new KisBoxFilterStrategy);
            pi.device = img->projection();
            pi.rect = QRect(0,0, img->width(), img->height());
            images.push_back(pi);
        }


        KisPaintLayerSP layer = new KisPaintLayer(m_view->image(), i18n("Panorama Layer"), 255, images[0].device->colorSpace());
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
        m_view->image()->addLayer(layer, parent, above);


        QRect dstArea;
        createPanorama(images, layer->paintDevice(), dstArea);

    }
    delete dialog;
}

#include <kurl.h>

void PanoramaPlugin::slotPreview()
{
    QList<PanoramaImage> images;
    for(int i = 0; i < m_wdgPanoramaCreation->listImages->count(); i++)
    {
        QString fileName = m_wdgPanoramaCreation->listImages->item(i)->text();
        kDebug(41006) <<"Loading fileName" << fileName;
        KisDoc2 d;
        d.import(fileName);
        PanoramaImage pi;
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
    QImage img = dst->convertToQImage(0,0.0);
    img = img.scaledToHeight(500);
    m_wdgPanoramaCreation->labelPreview->setPixmap(QPixmap::fromImage(img));
}

void PanoramaPlugin::createPanorama(QList<PanoramaImage>& images, KisPaintDeviceSP dstdevice, QRect& )
{
    int width = 1000; // TODO TMP variable
    int height = 1000; // TODO TMP variable
    kDebug(41006) <<"Creating panorama with" << images.size() <<" images";
    kDebug(41006) <<"Detecting interest points";
    KoColorSpace* graycs = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", 0);
    if(not graycs)
    {
        kDebug(41006) <<"Gray 8bit is not installed."; // TODO: message box
        return;
    }
    for(QList<PanoramaImage>::iterator it = images.begin(); it != images.end(); ++it)
    {
        KisPaintDeviceSP graydevice = new KisPaintDevice(*(it->device));
        graydevice->convertTo(graycs);
/*        QImage img = graydevice->convertToQImage(0,0.0);
        img.save("test.png", "PNG");*/
        it->points = KisInterestPointsDetector::interestPointDetector()->computeInterestPoints(graydevice, it->rect);
        width = it->rect.width();
        height = it->rect.height();
        for(lInterestPoints::const_iterator itp = it->points.begin(); itp != it->points.end(); ++itp)
        {
          kDebug(41006) <<"ip =" << (*itp)->x() <<"" << (*itp)->y() <<"" << (*itp)->toString();
        }
    }
    lMatches mp = matching(images[0].points, images[1].points);
/*    for(lMatches::const_iterator it = mp.begin(); it != mp.end(); ++it)
    {
        kDebug(41006) <<"init match =" << it->ref->x() <<"" << it->ref->y() <<"" << it->match->x() <<"" <<  it->match->y();
    }*/
    Ransac<ImageMatchModel, void, KisMatch> ransac(2,100,0);
    std::list<ImageMatchModel*> models = ransac.findModels( mp );
//     Ransac<HomographyImageMatchModel, HomographyImageMatchModelStaticParams, KisMatch> ransac(2,100, new HomographyImageMatchModelStaticParams(width, height));
//     std::list<HomographyImageMatchModel*> models = ransac.findModels( mp );
    if(not models.empty())
    {
        kDebug(41006) <<"Best model:" << (*models.begin())->fittingErrorSum() <<" with" << (*models.begin())->matches().size() <<" points";
#if 0
        std::vector<double> p = (*models.begin())->parameters();
#endif
#if 1
        Eigen::Matrix3d transfo = (*models.begin())->transfo();
        kDebug(41006) <<"Translation :" << transfo(0,2) <<"" << transfo(1,2);

        for(std::list<ImageMatchModel*>::iterator it = models.begin(); it != models.end(); it++)
        {
            kDebug(41006) <<" Error:" << (*it)->fittingErrorSum() <<" with" << (*it)->matches().size();
        }
//
//         for(lMatches::const_iterator it = (*models.begin())->matches().begin(); it != (*models.begin())->matches().end(); ++it)
//         {
//             kDebug(41006) <<"selected match =" << it->ref->x() <<"" << it->ref->y() <<"" << it->match->x() <<"" <<  it->match->y();
//         }

        PanoptimFunction<HomographySameDistortionFunction, HomographySameDistortionFunction::SIZEINDEXES> f( (*models.begin())->matches(), width * 0.5, height * 0.5, width, height );


        std::vector<double> p(HomographySameDistortionFunction::SIZEINDEXES);
        p[HomographySameDistortionFunction::INDX_a] = 0.0;
        p[HomographySameDistortionFunction::INDX_b] = 0.0;
        p[HomographySameDistortionFunction::INDX_c] = 0.0;
        p[HomographySameDistortionFunction::INDX_h11] = 1.0; // << TODO we compute a rotation in the ransac model, use that
        p[HomographySameDistortionFunction::INDX_h21] = 0.0;
        p[HomographySameDistortionFunction::INDX_h31] = transfo(0,2);
        p[HomographySameDistortionFunction::INDX_h12] = 0.0;
        p[HomographySameDistortionFunction::INDX_h22] = 1.0; // << TODO we compute a rotation in the ransac model, use that
        p[HomographySameDistortionFunction::INDX_h32] = transfo(1,2);
        p[HomographySameDistortionFunction::INDX_h13] = 0.0;
        p[HomographySameDistortionFunction::INDX_h23] = 0.0;
    //       double r = Optimization::Algorithms::gaussNewton< Optimization::Methods::GaussNewton< PanoptimFunction, double> >(&f, p, 100, 1e-12);
        double r = Optimization::Algorithms::levenbergMarquardt(&f, p, 100, 1e-12, 0.01, 10.0);
        kDebug(41006) <<"Remain =" << r;
    //       kDebug(41006) <<"Remain =" << Optimization::gradientDescent(&f, p, 10000, 1e-3, 1e-6);
        for(uint i = 0; i < p.size(); i++)
        {
            kDebug(41006) <<"p["<< i <<"]=" << p[i];
        }
        #if 0
        // Attempt a second optimization by completing the match base
        kDebug(41006) <<"Complete match base";
        lMatches sndmatches;
        for(lMatches::const_iterator it = mp.begin(); it != mp.end(); ++it)
        {

          int indx[HomographySameDistortionFunction::SIZEINDEXES];
          for(int i = 0; i < HomographySameDistortionFunction::SIZEINDEXES; i++)
          {
            indx[i] = i;
          }
          double norm(4.0 / ( width * width + height * height ) );
          HomographySameDistortionFunction hsdf(indx, width * 0.5, height * 0.5, norm, it->ref->x(), it->ref->y(), it->match->x(), it->match->y());
          double f1, f2;
          hsdf.f(p, f1, f2);
          if( fabs(f1) < 10.0 and fabs(f2) < 10.0)
          {
            kDebug(41006) <<"Match accepted" << it->ref->x() <<"" << it->ref->y() <<"" << it->match->x() <<"" <<  it->match->y() <<" f1 =" << f1 <<" f2 =" << f2;
            sndmatches.push_back(*it);
          } else {
            kDebug(41006) <<"Match rejected" << it->ref->x() <<"" << it->ref->y() <<"" << it->match->x() <<"" <<  it->match->y() <<" f1 =" << f1 <<" f2 =" << f2;
          }
        }

        kDebug(41006) <<"Start second optimization with" << sndmatches.size() <<" matches";
        PanoptimFunction<HomographySameDistortionFunction, HomographySameDistortionFunction::SIZEINDEXES> f2( sndmatches, width * 0.5, height * 0.5, width, height );
        r = Optimization::Algorithms::levenbergMarquardt(&f2, p, 1000, 1e-12, 0.01, 10.0);
        kDebug(41006) <<"Remain =" << r;
    //       kDebug(41006) <<"Remain =" << Optimization::gradientDescent(&f, p, 10000, 1e-3, 1e-6);
        #endif

    //       kDebug(41006) <<"a1 =" << p[0] <<" b1 =" << p[1] <<" a2 =" << p[2] <<" b2 =" << p[3] <<" rotation =" << p[4] <<" tx21 =" << p[5] <<" ty21 =" << p[6];
//         kDebug(41006) << transfo(0,0) <<"" << transfo(1,1) <<"" << cos(p[4]) <<"" << cos(p[5]);
#endif

        for(uint i = 0; i < p.size(); i++)
        {
            kDebug(41006) <<"p["<< i <<"]=" << p[i];
        }

// HomographyImageMatchModel

        // blend
        QList<KisImagesBlender::LayerSource> sources;
        KisImagesBlender::LayerSource firstLayer;
        firstLayer.layer = images[0].device;
        firstLayer.a = p[HomographySameDistortionFunction::INDX_a];
        firstLayer.b = p[HomographySameDistortionFunction::INDX_b];
        firstLayer.c = p[HomographySameDistortionFunction::INDX_c];
    /*      firstLayer.a = p[HomographyFunction::INDX_a1];
        firstLayer.b = p[HomographyFunction::INDX_b1];
        firstLayer.c = p[HomographyFunction::INDX_c1];*/
        firstLayer.xc1 = images[0].rect.width() * 0.5;
        firstLayer.xc2 = images[0].rect.width() * 0.5;
        firstLayer.yc1 = images[0].rect.height() * 0.5;
        firstLayer.yc2 = images[0].rect.height() * 0.5;
        firstLayer.norm = (4.0 / ( images[0].rect.width() * images[0].rect.width() + images[0].rect.height() * images[0].rect.height() ) );
        for(int i = 0; i < 3; i++)
        {
            for(int j = 0; j < 3; j++)
            {
            firstLayer.homography(i,j) = 0.0;
            }
            firstLayer.homography(i,i) = 1.0;
        }
        firstLayer.rect = images[0].rect;
        sources.push_back(firstLayer);
        KisImagesBlender::LayerSource secondLayer;
        secondLayer.layer = images[1].device;
        secondLayer.a = p[HomographySameDistortionFunction::INDX_a];
        secondLayer.b = p[HomographySameDistortionFunction::INDX_b];
        secondLayer.c = p[HomographySameDistortionFunction::INDX_c];
    /*      secondLayer.a = p[HomographyFunction::INDX_a2];
        secondLayer.b = p[HomographyFunction::INDX_b2];
        secondLayer.c = p[HomographyFunction::INDX_c2];*/
        secondLayer.xc1 = images[1].rect.width() * 0.5;
        secondLayer.xc2 = images[1].rect.width() * 0.5;
        secondLayer.yc1 = images[1].rect.height() * 0.5;
        secondLayer.yc2 = images[1].rect.height() * 0.5;
        secondLayer.norm = (4.0 / ( images[1].rect.width() * images[1].rect.width() + images[1].rect.height() * images[1].rect.height() ) );
        kDebug(41006) <<" homography = [" << p[HomographySameDistortionFunction::INDX_h11] <<"" << p[HomographySameDistortionFunction::INDX_h21] <<"" << p[HomographySameDistortionFunction::INDX_h31] <<" ;" << p[HomographySameDistortionFunction::INDX_h12] <<"" << p[HomographySameDistortionFunction::INDX_h22] <<"" << p[HomographySameDistortionFunction::INDX_h32] <<" ;" << p[HomographySameDistortionFunction::INDX_h13] <<"" << p[HomographySameDistortionFunction::INDX_h23] <<"" << 1.0 <<" ]";
        secondLayer.homography(0,0) = p[HomographySameDistortionFunction::INDX_h11];
        secondLayer.homography(0,1) = p[HomographySameDistortionFunction::INDX_h21];
        secondLayer.homography(0,2) = p[HomographySameDistortionFunction::INDX_h31];
        secondLayer.homography(1,0) = p[HomographySameDistortionFunction::INDX_h12];
        secondLayer.homography(1,1) = p[HomographySameDistortionFunction::INDX_h22];
        secondLayer.homography(1,2) = p[HomographySameDistortionFunction::INDX_h32];
        secondLayer.homography(2,0) = p[HomographySameDistortionFunction::INDX_h13];
        secondLayer.homography(2,1) = p[HomographySameDistortionFunction::INDX_h23];
        secondLayer.homography(2,2) = 1.0;
        secondLayer.rect = images[1].rect;
        sources.push_back(secondLayer);

        KisImagesBlender::blend(sources, dstdevice);
    } else {
        kDebug(41006) <<"No models found";
    }
}

#include "panorama.moc"
