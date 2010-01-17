/*
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

#include "bracketing2hdr.h"
#include <stdlib.h>

#include <kis_debug.h>
#include <kfiledialog.h>
#include <kpluginfactory.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kactioncollection.h>
#include <kplotobject.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>
#include <KoColorSpace.h>

#include <KoFilter.h>
#include <KoFilterManager.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_doc2.h"
#include "kis_filter_strategy.h"
#include "kis_global.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_types.h"
#include "kis_view2.h"
#include "kis_layer_manager.h"

#include "kis_meta_data_schema.h"
#include "kis_meta_data_store.h"
#include "kis_meta_data_value.h"

#include "ui_wdgbracketing2hdr.h"

#include <config-powf.h>

#ifndef HAVE_POWF
#undef powf
#define powf pow
#endif
#include <KoColorModelStandardIds.h>

const double epsilonOptimization = 1e-5;
const double epsilonGaussian = 1e-6;

K_PLUGIN_FACTORY(Bracketing2HDRPluginFactory, registerPlugin<Bracketing2HDRPlugin>();)
K_EXPORT_PLUGIN(Bracketing2HDRPluginFactory("krita"))


Bracketing2HDRPlugin::Bracketing2HDRPlugin(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent), m_wdgBracketing2HDR(0),
        m_responseType(RESPONSE_LINEAR), m_bitDepth(16), m_numberOfInputLevels(2 << (m_bitDepth - 1)), m_cameraResponseIsComputed(false)
{
    if (parent->inherits("KisView2")) {
        m_view = (KisView2*) parent;

        setComponentData(Bracketing2HDRPluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/bracketing2hdr.rc"), true);

        KAction *action  = new KAction(i18n("HDR Layer from bracketing"), this);
        actionCollection()->addAction("Bracketing2HDR", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotNewHDRLayerFromBracketing()));
    }
}

Bracketing2HDRPlugin::~Bracketing2HDRPlugin()
{
    m_view = 0;
}

void Bracketing2HDRPlugin::addImage(const QString& filename)
{
#if 1
    KisDoc2 d;
    KoFilterManager manager(&d);
    QByteArray nativeFormat = d.nativeFormatMimeType();
    KoFilter::ConversionStatus status;
    QString s = manager.importDocument(filename, status);
    KisImageWSP importedImage = d.image();
    KisPaintLayerSP projection = 0;
    if (importedImage) {
        projection = dynamic_cast<KisPaintLayer*>(importedImage->rootLayer()->firstChild().data());
    }
    if (!projection) {
        dbgPlugins <<"Image" << filename <<" has fail to load.";
        return;
    }
    KisMetaData::Store* exifInfo = projection->metaData();
    double exposure = 0.;
    if (exifInfo->containsEntry(KisMetaData::Schema::EXIFSchemaUri, "ExposureTime")) {
        exposure = exifInfo->getValue(KisMetaData::Schema::EXIFSchemaUri, "ExposureTime").asDouble();
    }
    dbgPlugins <<" Exposure Time: " << exposure;
    double aperture = 0.;
    if (exifInfo->containsEntry(KisMetaData::Schema::EXIFSchemaUri, "ApertureValue")) {
        aperture = exifInfo->getValue(KisMetaData::Schema::EXIFSchemaUri, "ApertureValue").asDouble();
        aperture = pow(2.0, aperture * 0.5);
    }
    dbgPlugins <<" Aperture: " << aperture;
    qint32 iso = 100;
    if (exifInfo->containsEntry(KisMetaData::Schema::EXIFSchemaUri, "ISOSpeedRatings")) {
        QList<KisMetaData::Value> isoSpeed = exifInfo->getValue(KisMetaData::Schema::EXIFSchemaUri, "ISOSpeedRatings").asArray();
        dbgPlugins << exifInfo->getValue(KisMetaData::Schema::EXIFSchemaUri, "ISOSpeedRatings");
        if(isoSpeed.size() >= 1)
        {
            iso = isoSpeed[0].asInteger();
            dbgPlugins << iso;
        } else {
            dbgPlugins << "Invalid iso value";
        }
    }
    dbgPlugins <<" ISO : " << iso;
    int index = m_wdgBracketing2HDR->tableWidgetImages->rowCount();
    m_wdgBracketing2HDR->tableWidgetImages->insertRow(index);
    m_wdgBracketing2HDR->tableWidgetImages->setItem(index, 0, new QTableWidgetItem(filename)); // TODO: generate a preview
    m_wdgBracketing2HDR->tableWidgetImages->setItem(index, 1, new QTableWidgetItem(QString::number(exposure)));
    m_wdgBracketing2HDR->tableWidgetImages->setItem(index, 2, new QTableWidgetItem(QString::number(aperture)));
    m_wdgBracketing2HDR->tableWidgetImages->setItem(index, 3, new QTableWidgetItem(QString::number(iso)));
    
   m_wdgBracketing2HDR->pushButtonCalculateCameraResponse->setEnabled(true);
   m_wdgBracketing2HDR->pushButtonCreateHDRLayer->setEnabled(true);
#endif
}

void Bracketing2HDRPlugin::slotAddImages()
{
//     dbgPlugins <<"Add image";
    QStringList openfiles = KFileDialog::getOpenFileNames(KUrl(), "*", m_view);
//     dbgPlugins << openfiles.size() <<" files selected for inclusion";
    foreach(const QString & filename, openfiles) {
        addImage(filename);
    }
}

void Bracketing2HDRPlugin::slotNewHDRLayerFromBracketing()
{
    if (m_wdgBracketing2HDR) delete m_wdgBracketing2HDR;
    m_wdgBracketing2HDR = new Ui_WdgBracketing2HDR();
    QDialog* dialog = new QDialog(m_view);
    dialog->setModal(true);
    m_wdgBracketing2HDR->setupUi(dialog);
    connect(m_wdgBracketing2HDR->pushButtonCancel, SIGNAL(released()), dialog, SLOT(reject()));
    connect(m_wdgBracketing2HDR->pushButtonCreateHDRLayer, SIGNAL(released()), dialog, SLOT(accept()));
    connect(m_wdgBracketing2HDR->pushButtonAddImages, SIGNAL(released()), this, SLOT(slotAddImages()));
    connect(m_wdgBracketing2HDR->pushButtonCalculateCameraResponse, SIGNAL(released()), this, SLOT(computeCameraResponse()));

    if (dialog->exec() == QDialog::Accepted) {
        dbgPlugins << "Start creating the HDR layer";
        if (!m_cameraResponseIsComputed) {
            computeCameraResponse();
        }
        KisImageWSP image = m_view->image();
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
        if (!cs) {
            KMessageBox::error(m_view, i18n("HDR colorspace RGBAF32 not found, please check your installation."), i18n("Layer Creation Error"));
            return;
        }
        KisPaintLayerSP layer = new KisPaintLayer(image, i18n("HDR Layer"), 255, cs);
        Q_ASSERT(layer);
        KisGroupLayerSP parent;
        KisLayerSP above;
        if (KisGroupLayer* pactive = qobject_cast<KisGroupLayer*>(m_view->layerManager()->activeLayer().data())) {
            parent = pactive;
        }
        if (!parent)
            parent = m_view->image()->rootLayer();
        above = qobject_cast<KisLayer*>(parent->firstChild().data());
        image->addNode(layer.data(), parent.data(), above.data());

        createHDRPaintDevice(m_imagesFrames, layer->paintDevice());
        m_view->canvas()->update();
    }
    delete dialog;
}


void Bracketing2HDRPlugin::createHDRPaintDevice(QList<BracketingFrame> frames,  KisPaintDeviceSP device)
{
    int width = frames[0].image->width();
    int height = frames[0].image->height();
    // Create iterators
    for (int k = 0; k < frames.size(); k++) {
        frames[k].it = new KisHLineConstIteratorPixel(frames[k].device->createHLineConstIterator(0, 0, width));
    }
    KisHLineIteratorPixel dstIt = device->createHLineIterator(0, 0, width);
//     QColor c;
    quint8* c = KoRgbU16Traits::allocate(1);
    struct {
        double rsum, rdiv, gsum, gdiv, bsum, bdiv;
    } v;
    // Loop over pixels
    for (int j = 0; j < height; j++) {
        while (!dstIt.isDone()) {
            // compute the sum and normalization coefficient for rgb values from the bracketing ldr images
            v.rsum = v.rdiv = v.gsum = v.gdiv = v.bsum = v.bdiv = 0.0;
            for (int k = 0; k < frames.size(); k++) {
                double coef = frames[k].apexBrightness;
                frames[k].device->colorSpace()->toRgbA16(frames[k].it->rawData(), c, 1);
                v.rsum += m_intensityR[ KoRgbU16Traits::red(c)] * m_weights[ KoRgbU16Traits::red(c)] * coef ;
                v.rdiv += m_weights[ KoRgbU16Traits::red(c)] * coef * coef ;
                v.gsum += m_intensityG[ KoRgbU16Traits::green(c)] * m_weights[ KoRgbU16Traits::green(c)] * coef ;
                v.gdiv += m_weights[ KoRgbU16Traits::green(c)] * coef * coef ;
                v.bsum += m_intensityB[ KoRgbU16Traits::blue(c)] * m_weights[ KoRgbU16Traits::blue(c)] * coef ;
                v.bdiv += m_weights[ KoRgbU16Traits::blue(c)] * coef * coef ;
                ++(*frames[k].it);
            }
            float* pixelData = reinterpret_cast<float*>(dstIt.rawData());
            if (v.rdiv != 0.0) {
                pixelData[KoRgbU16Traits::red_pos] = v.rsum / v.rdiv;
            } else {
                pixelData[KoRgbU16Traits::red_pos] = 0.0;
            }
            if (v.gdiv != 0.0) {
                pixelData[KoRgbU16Traits::green_pos] = v.gsum / v.gdiv;
            } else {
                pixelData[KoRgbU16Traits::green_pos] = 0.0;
            }
            if (v.bdiv != 0.0) {
                pixelData[KoRgbU16Traits::blue_pos] = v.bsum / v.bdiv;
            } else {
                pixelData[KoRgbU16Traits::blue_pos] = 0.0;
            }
            device->colorSpace()->setAlpha(dstIt.rawData(), 255, 1);
            ++dstIt;
        }
        dstIt.nextRow();
        for (int k = 0; k < frames.size(); k++) {
            frames[k].it->nextRow();
        }
    }
    // Delete iterators
    for (int k = 0; k < frames.size(); k++) {
        delete frames[k].it;
    }
}

void Bracketing2HDRPlugin::normalize(QVector<double>& intensity)
{
    double norm = 0.0;
    int pos = numberOfInputLevels() / 2;
    while ((norm = intensity[pos]) == 0.0) {
        pos++;
    }
    for (int i = 0; i < numberOfInputLevels(); i++) {
        intensity[i] /= norm;
    }
}

void Bracketing2HDRPlugin::fillHoles(QVector<double>& intensity)
{
    if (intensity[0] == 0.0) {
        int pos = 1;
        double v = 0.0;
        while ((v = intensity[pos]) == 0.0) {
            pos++;
        }
        for (int i = 0; i < pos; i++) {
            intensity[i] = v;
        }
    }
    if (intensity[numberOfInputLevels() - 1] == 0.0) {
        int pos = numberOfInputLevels() - 2;
        double v = 0.0;
        while ((v = intensity[pos]) == 0.0) {
            pos--;
        }
        for (int i = pos + 1; i < numberOfInputLevels(); i++) {
            intensity[i] = v;
        }
    }
    double lastV = intensity[0];
    int lastPos = 0;
    for (int i = 1; i < numberOfInputLevels(); i++) {
        double v = intensity[i];
        if (v != 0.0) {
            lastV = v;
            lastPos = i;
        } else {
            int pos = i + 1;
            double endV = 0.0;
            while ((endV = intensity[pos]) == 0.0) {
                pos++;
            }
            double coef = (endV - lastV) / (pos - lastPos);
            double origin = lastV - lastPos * coef;
            for (; i < pos; i++) {
                intensity[i] = origin + i * coef;
            }
            lastV = endV;
            lastPos = pos;
        }
    }
}

QList<Bracketing2HDRPlugin::BracketingFrame> Bracketing2HDRPlugin::reduceSizeOfFrames(double size, QList<BracketingFrame> originalFrames)
{
    QList<BracketingFrame> frames = originalFrames;
    for (QList<BracketingFrame>::iterator it = frames.begin();
            it != frames.end(); ++it) {
        it->image = new KisImage(*it->image);
        it->device = it->image->projection();
        double scalecoefficient = size / qMax(it->image->width(), it->image->height());
        it->image->scale(scalecoefficient, scalecoefficient, 0, new KisBoxFilterStrategy());
    }
    return frames;
}


void Bracketing2HDRPlugin::computeCameraResponse()
{
    QTime time;
    time.start();
    dbgPlugins <<"computeCameraResponse()";
    if(!loadImagesInMemory()) return;
    switch (responseType()) {
    case RESPONSE_LINEAR:
        computeLinearResponse(m_intensityR);
        computeLinearResponse(m_intensityG);
        computeLinearResponse(m_intensityB);
        computePseudoGaussianWeights();
        break;
    default:
        kError() << "NOT IMPLEMENTED YET !" << endl;
        Q_ASSERT(false);
    }
    // First compute on downscaled version
    QList<BracketingFrame> frames1000 = reduceSizeOfFrames(1000, m_imagesFrames);
    QList<BracketingFrame> frames500 = reduceSizeOfFrames(500, frames1000);
    QList<BracketingFrame> frames100 = reduceSizeOfFrames(100, frames500);
    computeCameraResponse(frames100);
    computeCameraResponse(frames500);
    computeCameraResponse(frames1000);
//     computeCameraResponse(m_imagesFrames);
    dbgPlugins << "Computing curves took:" << time.elapsed();
}
void Bracketing2HDRPlugin::computeCameraResponse(QList<BracketingFrame> frames)
{
    Q_ASSERT(frames.size() > 0);
//     return;
    // Normalize the intensity responses
    normalize(m_intensityR);
    normalize(m_intensityG);
    normalize(m_intensityB);
#if 1
    // Now optimize the camera response
    // Create a temporary paint device and fill it with the current value
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
    if (!cs) {
        KMessageBox::error(m_view, i18n("HDR colorspace RGBAF32 not found, please check your installation."), i18n("Compute Camera Response Error"));
        return;
    }
    KisPaintDeviceSP device = new KisPaintDevice(cs);
    // Initialize some variables used for computation
    QVector<qint64> counts[3];
    QVector<double> sums[3];
    for (int k = 0; k < 3; k++) {
        counts[k].resize(numberOfInputLevels());
        sums[k].resize(numberOfInputLevels());
    }
    QVector<double> iROld, iGOld, iBOld;
    int width = frames[0].image->width();
    int height = frames[0].image->height();
    while (true) {
//         dbgPlugins <<"New loop";
        // Compute answer
        createHDRPaintDevice(frames, device);
        // Copy the old responses
        iROld = m_intensityR;
        iGOld = m_intensityG;
        iBOld = m_intensityB;
        // Compute new response curve
        for (int k = 0; k < 3; k++) {
            counts[k].fill(0);
            sums[k].fill(0.0);
        }
        // Create iterators
        for (int k = 0; k < frames.size(); k++) {
            frames[k].it = new KisHLineConstIteratorPixel(frames[k].device->createHLineConstIterator(0, 0, width));
        }
        KisHLineIteratorPixel dstIt = device->createHLineIterator(0, 0, width);
        quint8* c = KoRgbU16Traits::allocate(1);
        // Loop over pixels
        for (int j = 0; j < height; j++) {
            while (!dstIt.isDone()) {
                float* pixelData = reinterpret_cast<float*>(dstIt.rawData());
                // compute the sum and normalization coefficient for rgb values from the bracketing ldr images
                for (int k = 0; k < frames.size(); k++) {
                    double coef = frames[k].apexBrightness;
                    frames[k].device->colorSpace()->toRgbA16(frames[k].it->rawData(), c, 1);
//                     if( KoRgbU16Traits::red(c) < 100)
//                         dbgPlugins << KoRgbU16Traits::red(c) <<"" << (int)frames[k].it->rawData()[2]  <<"" << k <<"" << (pixelData[2] * coef) <<""  << pixelData[2] <<"" << sums[KoRgbU16Traits::red_pos][ KoRgbU16Traits::red(c) ] <<"" << counts[KoRgbU16Traits::red_pos][ KoRgbU16Traits::red(c) ] <<"" <<;
                    sums[KoRgbU16Traits::red_pos][ KoRgbU16Traits::red(c)] += pixelData[2] * coef ;
                    counts[KoRgbU16Traits::red_pos][ KoRgbU16Traits::red(c)] ++;
                    sums[KoRgbU16Traits::green_pos][ KoRgbU16Traits::green(c)] += pixelData[1] * coef ;
                    counts[KoRgbU16Traits::green_pos][ KoRgbU16Traits::green(c)] ++;
                    sums[KoRgbU16Traits::blue_pos][ KoRgbU16Traits::blue(c)] += pixelData[0] * coef ;
                    counts[KoRgbU16Traits::blue_pos][ KoRgbU16Traits::blue(c)] ++;
                    ++(*frames[k].it);
                }
                ++dstIt;
            }
            dstIt.nextRow();
            for (int k = 0; k < frames.size(); k++) {
                frames[k].it->nextRow();
            }
        }
        // Delete iterators
        for (int k = 0; k < frames.size(); k++) {
            delete frames[k].it;
        }
        // Update intensity responses
#define UPDATEINTENSITY(n,vec) \
    for(int i = 0; i < numberOfInputLevels(); i++) \
    { \
        if(counts[n][i] != 0) \
            vec[i] = sums[n][i] / counts[n][i]; \
        else \
            vec[i] = 0.0; \
    } \
    normalize(vec); \
    fillHoles(vec);
        UPDATEINTENSITY(KoRgbU16Traits::red_pos, m_intensityR);
        UPDATEINTENSITY(KoRgbU16Traits::green_pos, m_intensityG);
        UPDATEINTENSITY(KoRgbU16Traits::blue_pos, m_intensityB);
#undef UPDATEINTENSITY
        // Compute the convergence
        int count = 0;
        double sumdiff = 0.0;
#define DIFF(vec1, vec2) \
    if(vec1[i] != 0.0) \
    { \
        double diff = vec1[i] - vec2[i]; \
        sumdiff += diff * diff; \
        count++; \
    }
        for (int i = 0; i < numberOfInputLevels(); i++) {
            DIFF(m_intensityR, iROld);
            DIFF(m_intensityG, iGOld);
            DIFF(m_intensityB, iBOld);
        }
#undef DIFF
        dbgPlugins << "Optimization delta =" << (sumdiff / count) << " =" << sumdiff << " /" << count << " " << isnan(sumdiff) << " " << isinf(sumdiff);
        Q_ASSERT(isnan(sumdiff) != 0);
        Q_ASSERT(isinf(sumdiff) != 0);
        if (sumdiff / count < epsilonOptimization)
            break;
    }
#endif

    // plot the curve
//     m_wdgBracketing2HDR->plotWidget;
//     QVector<double> m_intensityR, m_intensityG, m_intensityB, m_weights;
    KPlotObject *redCurve = new KPlotObject(Qt::red, KPlotObject::Lines);
    KPlotObject *greenCurve = new KPlotObject(Qt::green, KPlotObject::Lines);
    KPlotObject *blueCurve = new KPlotObject(Qt::blue, KPlotObject::Lines);
    double min = INFINITY;
    double max = -INFINITY;
    for (int i = 0; i < numberOfInputLevels(); i ++) {
        redCurve->addPoint(i, m_intensityR[i]);
        min = qMin(min, m_intensityR[i]);
        max = qMax(max, m_intensityR[i]);
        greenCurve->addPoint(i, m_intensityG[i]);
        min = qMin(min, m_intensityG[i]);
        max = qMax(max, m_intensityG[i]);
        blueCurve->addPoint(i, m_intensityB[i]);
        min = qMin(min, m_intensityB[i]);
        max = qMax(max, m_intensityB[i]);
//         dbgPlugins << i <<"" << m_intensityR[i] <<"" << m_intensityG[i] <<"" << m_intensityB[i];
    }
    m_wdgBracketing2HDR->plotWidget->addPlotObject(redCurve);
    m_wdgBracketing2HDR->plotWidget->addPlotObject(greenCurve);
    m_wdgBracketing2HDR->plotWidget->addPlotObject(blueCurve);
    m_wdgBracketing2HDR->plotWidget->setLimits(0, numberOfInputLevels(), min, max);
    dbgPlugins << "Response curve computed";
    m_cameraResponseIsComputed = true;
}

bool Bracketing2HDRPlugin::loadImagesInMemory()
{
    m_imagesFrames.clear();
    double apexNorm = 0.0;
    for (int i = 0; i < m_wdgBracketing2HDR->tableWidgetImages->rowCount(); i++) {
        // read the info about the frame in the table
        QString fileName = m_wdgBracketing2HDR->tableWidgetImages->item(i, 0)->text();

        BracketingFrame f;
        f.exposure = m_wdgBracketing2HDR->tableWidgetImages->item(i, 1)->text().toDouble();
        f.aperture = m_wdgBracketing2HDR->tableWidgetImages->item(i, 2)->text().toDouble();
        f.sensitivity = m_wdgBracketing2HDR->tableWidgetImages->item(i, 3)->text().toInt();
        // Compute the apex brightness, more info at http://en.wikipedia.org/wiki/APEX_system
        f.apexBrightness = 2.0 * log(f.aperture) + log(1.0 / f.exposure) - log(f.sensitivity / 3.125);
        f.apexBrightness /= log(2.0);
        f.apexBrightness = 1.0 / (powf(2.0, f.apexBrightness));  // * ( 1.0592f * 11.4f / 3.125f ) ); // TODO: the magic number is apparrently dependent of the camera, this value is taken from pfscalibrate, this need to be configurable (it is the reflected-light meter calibration constant)
        if(f.apexBrightness == 0.0 || isnan(f.apexBrightness) == 1 || isinf(f.apexBrightness) == 1 )
        {
            return false;
        }
        if (i >=  m_wdgBracketing2HDR->tableWidgetImages->rowCount() / 2 &&
                i < m_wdgBracketing2HDR->tableWidgetImages->rowCount() / 2 + 1) {
            apexNorm = f.apexBrightness;
        }
        dbgPlugins << "Loading fileName" << fileName << " Exposure =" << f.exposure << " APEX Brightness =" << f.apexBrightness << " Aperture" << f.aperture << " Sensitivity =" << f.sensitivity;
        // import the image
        KisDoc2 d;
        KoFilterManager manager(&d);
        QByteArray nativeFormat = d.nativeFormatMimeType();
        KoFilter::ConversionStatus status;
        QString s = manager.importDocument(fileName, status);
        f.image = d.image();
        f.device = 0;
        f.image->setUndoAdapter(0);
        if (f.image) {
            f.device = f.image->projection();
        }
        if (!f.device) {
            dbgPlugins <<"Image" << fileName <<" has fail to load.";
            return false;
        }
        // compute the maximum and minimu response
        // add the frame to the list of images
        m_imagesFrames.push_back(f);
    }
    if(apexNorm != 0.0)
    {
      for (int i = 0; i < m_wdgBracketing2HDR->tableWidgetImages->rowCount(); i++) {
          m_imagesFrames[i].apexBrightness /= apexNorm;
          dbgPlugins << ppVar(m_imagesFrames[i].apexBrightness);
      }
    }
    return true;
}

void Bracketing2HDRPlugin::computeLinearResponse(QVector<double>& intensity)
{
    intensity.resize(numberOfInputLevels());
    intensity.fill(0.0);
    for (int i = 0; i < numberOfInputLevels(); i++) {
        intensity[i] = i * (1. / numberOfInputLevels());
    }
}


void Bracketing2HDRPlugin::computePseudoGaussianWeights()
{
    double pseudosigma = numberOfInputLevels() * 0.5;
    m_weights.resize(numberOfInputLevels());
    for (int i = 0; i < numberOfInputLevels(); i++) {
        double v = (i - pseudosigma) / pseudosigma;
        m_weights[i] = exp(-8.0 * v * v);
        if (m_weights[i] < epsilonGaussian)
            m_weights[i] = 0.;
//         dbgPlugins <<" Gaussian weights [" << i <<"] =" << m_weights[i];
    }
}


#include "bracketing2hdr.moc"
