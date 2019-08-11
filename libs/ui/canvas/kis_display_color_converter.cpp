/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_display_color_converter.h"

#include <QGlobalStatic>
#include <QPointer>

#include <KoColor.h>
#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorConversions.h>

#include <KoCanvasResourceProvider.h>
#include "kis_config_notifier.h"
#include "kis_canvas_resource_provider.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_image.h"
#include "kis_node.h"

#include "kundo2command.h"
#include "kis_config.h"
#include "kis_paint_device.h"
#include "kis_iterator_ng.h"
#include "kis_fixed_paint_device.h"
#include "opengl/KisOpenGLModeProber.h"

Q_GLOBAL_STATIC(KisDisplayColorConverter, s_instance)


struct KisDisplayColorConverter::Private
{
    Private(KisDisplayColorConverter *_q, KoCanvasResourceProvider *_resourceManager)
        : q(_q),
          resourceManager(_resourceManager),
          nodeColorSpace(0),
          paintingColorSpace(0),
          monitorProfile(0),
          renderingIntent(KoColorConversionTransformation::internalRenderingIntent()),
          conversionFlags(KoColorConversionTransformation::internalConversionFlags()),
          displayFilter(0),
          displayRenderer(new DisplayRenderer(_q, _resourceManager))
    {
        useHDRMode = KisOpenGLModeProber::instance()->useHDRMode();
    }

    KisDisplayColorConverter *const q;

    KoCanvasResourceProvider *resourceManager;

    const KoColorSpace *nodeColorSpace;
    const KoColorSpace *paintingColorSpace;

    const KoColorProfile* inputImageProfile = 0;

    const KoColorProfile* qtWidgetsProfile() const {
        return useHDRMode ? KoColorSpaceRegistry::instance()->p709SRGBProfile() : monitorProfile;
    }

    const KoColorProfile* openGLSurfaceProfile() const {
        return useHDRMode && openGLCanvasIsActive ? KisOpenGLModeProber::instance()->rootSurfaceColorProfile() : monitorProfile;
    }

    const KoColorProfile* ocioInputProfile() const {
        return displayFilter && displayFilter->useInternalColorManagement() ?
                    openGLSurfaceProfile() : inputImageProfile;
    }

    const KoColorProfile* ocioOutputProfile() const {
        return openGLSurfaceProfile();
    }

    const KoColorSpace* ocioInputColorSpace() const {
        return KoColorSpaceRegistry::instance()->
                colorSpace(
                    RGBAColorModelID.id(),
                    Float32BitsColorDepthID.id(),
                    ocioInputProfile());
    }

    const KoColorSpace* ocioOutputColorSpace() const {
        return KoColorSpaceRegistry::instance()->
                colorSpace(
                    RGBAColorModelID.id(),
                    Float32BitsColorDepthID.id(),
                    ocioOutputProfile());
    }

    const KoColorSpace* qtWidgetsColorSpace() const {
        return KoColorSpaceRegistry::instance()->
                colorSpace(
                    RGBAColorModelID.id(),
                    Integer8BitsColorDepthID.id(),
                    qtWidgetsProfile());
    }

    const KoColorSpace* openGLSurfaceColorSpace(const KoID &bitDepthId) const {
        return KoColorSpaceRegistry::instance()->
                colorSpace(
                    RGBAColorModelID.id(),
                    bitDepthId.id(),
                    openGLSurfaceProfile());
    }

    const KoColorSpace* intermediateColorSpace() const {
        // the color space where we apply exposure and
        // gamma should always be linear
        return KoColorSpaceRegistry::instance()->
                colorSpace(
                    RGBAColorModelID.id(),
                    Float32BitsColorDepthID.id(),
                    KoColorSpaceRegistry::instance()->p2020G10Profile());
    }

    const KoColorProfile *monitorProfile;

    KoColorConversionTransformation::Intent renderingIntent;
    KoColorConversionTransformation::ConversionFlags conversionFlags;

    QSharedPointer<KisDisplayFilter> displayFilter;

    KoColor intermediateFgColor;
    KisNodeSP connectedNode;
    KisImageSP image;

    bool useHDRMode = false;
    bool openGLCanvasIsActive = false;

    inline KoColor approximateFromQColor(const QColor &qcolor);
    inline QColor approximateToQColor(const KoColor &color);

    void slotCanvasResourceChanged(int key, const QVariant &v);
    void slotUpdateCurrentNodeColorSpace();
    void selectPaintingColorSpace();

    void updateIntermediateFgColor(const KoColor &color);
    void setCurrentNode(KisNodeSP node);
    bool useOcio() const;

    class DisplayRenderer : public KoColorDisplayRendererInterface {
    public:
        DisplayRenderer(KisDisplayColorConverter *displayColorConverter, KoCanvasResourceProvider *resourceManager)
            : m_displayColorConverter(displayColorConverter),
              m_resourceManager(resourceManager)
        {
            displayColorConverter->connect(displayColorConverter, SIGNAL(displayConfigurationChanged()),
                            this, SIGNAL(displayConfigurationChanged()), Qt::UniqueConnection);
        }

        QImage convertToQImage(const KoColorSpace *srcColorSpace, const quint8 *data, qint32 width, qint32 height) const override {
            KisPaintDeviceSP dev = new KisPaintDevice(srcColorSpace);
            dev->writeBytes(data, 0, 0, width, height);
            return m_displayColorConverter->toQImage(dev);
        }

        QColor toQColor(const KoColor &c) const override {
            return m_displayColorConverter->toQColor(c);
        }

        KoColor approximateFromRenderedQColor(const QColor &c) const override {
            return m_displayColorConverter->approximateFromRenderedQColor(c);
        }

        KoColor fromHsv(int h, int s, int v, int a) const override {
            return m_displayColorConverter->fromHsv(h, s, v, a);
        }

        void getHsv(const KoColor &srcColor, int *h, int *s, int *v, int *a) const override {
            m_displayColorConverter->getHsv(srcColor, h, s, v, a);
        }

        qreal minVisibleFloatValue(const KoChannelInfo *chaninfo) const override {
            return chaninfo->getUIMin();
        }

        qreal maxVisibleFloatValue(const KoChannelInfo *chaninfo) const override {
            qreal maxValue = chaninfo->getUIMax();

            if (m_resourceManager) {
                qreal exposure = m_resourceManager->resource(KisCanvasResourceProvider::HdrExposure).value<qreal>();
                // not sure if *= is what we want
                maxValue *= std::pow(2.0, -exposure);
            }

            return maxValue;
        }

        const KoColorSpace* getPaintingColorSpace() const override {
            return m_displayColorConverter->paintingColorSpace();
        }

    private:
        KisDisplayColorConverter *m_displayColorConverter;
        QPointer<KoCanvasResourceProvider> m_resourceManager;
    };

    QScopedPointer<KoColorDisplayRendererInterface> displayRenderer;
};

KisDisplayColorConverter::KisDisplayColorConverter(KoCanvasResourceProvider *resourceManager, QObject *parent)
    : QObject(parent),
      m_d(new Private(this, resourceManager))
{

    connect(m_d->resourceManager, SIGNAL(canvasResourceChanged(int,QVariant)),
            SLOT(slotCanvasResourceChanged(int,QVariant)));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()),
            SLOT(selectPaintingColorSpace()));

    m_d->inputImageProfile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    m_d->setCurrentNode(0);
    setMonitorProfile(0);
    setDisplayFilter(QSharedPointer<KisDisplayFilter>(0));
}

KisDisplayColorConverter::KisDisplayColorConverter()
    : m_d(new Private(this, 0))
{
    setDisplayFilter(QSharedPointer<KisDisplayFilter>(0));

    m_d->inputImageProfile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    m_d->paintingColorSpace = KoColorSpaceRegistry::instance()->rgb8();

    m_d->setCurrentNode(0);
    setMonitorProfile(0);
}

KisDisplayColorConverter::~KisDisplayColorConverter()
{
}

void KisDisplayColorConverter::setImageColorSpace(const KoColorSpace *cs)
{
    m_d->inputImageProfile =
        cs->colorModelId() == RGBAColorModelID ?
        cs->profile() :
        KoColorSpaceRegistry::instance()->p709SRGBProfile();

    emit displayConfigurationChanged();
}


KisDisplayColorConverter* KisDisplayColorConverter::dumbConverterInstance()
{
    return s_instance;
}

KoColorDisplayRendererInterface* KisDisplayColorConverter::displayRendererInterface() const
{
    return m_d->displayRenderer.data();
}

bool KisDisplayColorConverter::Private::useOcio() const
{
    return displayFilter && paintingColorSpace && paintingColorSpace->colorModelId() == RGBAColorModelID;
}

void KisDisplayColorConverter::Private::updateIntermediateFgColor(const KoColor &srcColor)
{
    KIS_ASSERT_RECOVER_RETURN(displayFilter);

    KoColor color = srcColor;
    color.convertTo(intermediateColorSpace());
    displayFilter->approximateForwardTransformation(color.data(), 1);
    intermediateFgColor = color;
}

void KisDisplayColorConverter::Private::slotCanvasResourceChanged(int key, const QVariant &v)
{
    if (key == KisCanvasResourceProvider::CurrentKritaNode) {
        KisNodeSP currentNode = v.value<KisNodeWSP>();
        setCurrentNode(currentNode);
    } else if (useOcio() && key == KoCanvasResourceProvider::ForegroundColor) {
        updateIntermediateFgColor(v.value<KoColor>());
    }
}

void KisDisplayColorConverter::Private::slotUpdateCurrentNodeColorSpace()
{
    setCurrentNode(connectedNode);
}

inline KisPaintDeviceSP findValidDevice(KisNodeSP node) {
    return node->paintDevice() ? node->paintDevice() : node->original();
}

void KisDisplayColorConverter::Private::setCurrentNode(KisNodeSP node)
{
    if (connectedNode) {
        KisPaintDeviceSP device = findValidDevice(connectedNode);

        if (device) {
            q->disconnect(device, 0);
        }
    }

    nodeColorSpace = 0;

    if (node) {
        KisPaintDeviceSP device = findValidDevice(node);

        nodeColorSpace = device ?
            device->compositionSourceColorSpace() :
            node->colorSpace();

        KIS_SAFE_ASSERT_RECOVER_NOOP(nodeColorSpace);

        if (device) {
            q->connect(device, SIGNAL(profileChanged(const KoColorProfile*)),
                       SLOT(slotUpdateCurrentNodeColorSpace()), Qt::UniqueConnection);
            q->connect(device, SIGNAL(colorSpaceChanged(const KoColorSpace*)),
                       SLOT(slotUpdateCurrentNodeColorSpace()), Qt::UniqueConnection);
        }

    }

    if (!nodeColorSpace) {
        nodeColorSpace = KoColorSpaceRegistry::instance()->rgb8();
    }

    connectedNode = node;
    selectPaintingColorSpace();
}

void KisDisplayColorConverter::Private::selectPaintingColorSpace()
{
    KisConfig cfg(true);
    paintingColorSpace = cfg.customColorSelectorColorSpace();

    if (!paintingColorSpace || displayFilter) {
        paintingColorSpace = nodeColorSpace;
    }

    emit q->displayConfigurationChanged();
}

const KoColorSpace* KisDisplayColorConverter::paintingColorSpace() const
{
    KIS_ASSERT_RECOVER(m_d->paintingColorSpace) {
        return KoColorSpaceRegistry::instance()->rgb8();
    }

    return m_d->paintingColorSpace;
}

void KisDisplayColorConverter::setMonitorProfile(const KoColorProfile *monitorProfile)
{
    if (m_d->useHDRMode) {
        // we don't use ICCcolor management in HDR mode
        monitorProfile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    }

    m_d->monitorProfile = monitorProfile;

    m_d->renderingIntent = renderingIntent();
    m_d->conversionFlags = conversionFlags();

    emit displayConfigurationChanged();
}

void KisDisplayColorConverter::setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter)
{
    if (m_d->displayFilter && displayFilter &&
        displayFilter->lockCurrentColorVisualRepresentation()) {

        KoColor color(m_d->intermediateFgColor);
        displayFilter->approximateInverseTransformation(color.data(), 1);
        color.convertTo(m_d->paintingColorSpace);
        m_d->resourceManager->setForegroundColor(color);
    }

    m_d->displayFilter = displayFilter;

    if (m_d->displayFilter) {
        m_d->updateIntermediateFgColor(
            m_d->resourceManager->foregroundColor());
    }


    { // sanity check
        // KisConfig cfg;
        // KIS_ASSERT_RECOVER_NOOP(cfg.useOcio() == (bool) m_d->displayFilter);
    }

    m_d->selectPaintingColorSpace();
}


KoColorConversionTransformation::Intent
KisDisplayColorConverter::renderingIntent()
{
    KisConfig cfg(true);
    return (KoColorConversionTransformation::Intent)cfg.monitorRenderIntent();
}

KoColorConversionTransformation::ConversionFlags
KisDisplayColorConverter::conversionFlags()
{
    KoColorConversionTransformation::ConversionFlags conversionFlags =
        KoColorConversionTransformation::HighQuality;

    KisConfig cfg(true);

    if (cfg.useBlackPointCompensation()) conversionFlags |= KoColorConversionTransformation::BlackpointCompensation;
    if (!cfg.allowLCMSOptimization()) conversionFlags |= KoColorConversionTransformation::NoOptimization;

    return conversionFlags;
}

QSharedPointer<KisDisplayFilter> KisDisplayColorConverter::displayFilter() const
{
    return m_d->displayFilter;
}

const KoColorProfile* KisDisplayColorConverter::monitorProfile() const
{
    return m_d->monitorProfile;
}

const KoColorProfile* KisDisplayColorConverter::openGLCanvasSurfaceProfile() const
{
    return m_d->openGLSurfaceProfile();
}

bool KisDisplayColorConverter::isHDRMode() const
{
    return  m_d->useHDRMode;
}

void KisDisplayColorConverter::notifyOpenGLCanvasIsActive(bool value)
{
    m_d->openGLCanvasIsActive = value;
    emit displayConfigurationChanged();
}


QColor KisDisplayColorConverter::toQColor(const KoColor &srcColor) const
{
    KoColor c(srcColor);

    if (m_d->useOcio()) {
        KIS_ASSERT_RECOVER(m_d->ocioInputColorSpace()->pixelSize() == 16) {
            return QColor(Qt::green);
        }

        c.convertTo(m_d->ocioInputColorSpace());
        m_d->displayFilter->filter(c.data(), 1);
        c.setProfile(m_d->ocioOutputProfile());
    }

    // we expect the display profile is rgb8, which is BGRA here
    KIS_ASSERT_RECOVER(m_d->qtWidgetsColorSpace()->pixelSize() == 4) {
        return QColor(Qt::red);
    }

    c.convertTo(m_d->qtWidgetsColorSpace(), m_d->renderingIntent, m_d->conversionFlags);
    const quint8 *p = c.data();
    return QColor(p[2], p[1], p[0], p[3]);
}

KoColor KisDisplayColorConverter::applyDisplayFiltering(const KoColor &srcColor,
                                                        const KoID &bitDepthId) const
{
    KoColor c(srcColor);

    if (m_d->useOcio()) {
        KIS_ASSERT_RECOVER(m_d->ocioInputColorSpace()->pixelSize() == 16) {
            return srcColor;
        }

        c.convertTo(m_d->ocioInputColorSpace());
        m_d->displayFilter->filter(c.data(), 1);
        c.setProfile(m_d->ocioOutputProfile());
    }

    c.convertTo(m_d->openGLSurfaceColorSpace(bitDepthId), m_d->renderingIntent, m_d->conversionFlags);
    return c;
}

bool KisDisplayColorConverter::canSkipDisplayConversion(const KoColorSpace *cs) const
{
    const KoColorProfile *displayProfile = m_d->openGLSurfaceProfile();

    return !m_d->useOcio() &&
        cs->colorModelId() == RGBAColorModelID &&
        (!!cs->profile() == !!displayProfile) &&
        (!cs->profile() ||
         cs->profile()->uniqueId() == displayProfile->uniqueId());
}


KoColor KisDisplayColorConverter::approximateFromRenderedQColor(const QColor &c) const
{
    return m_d->approximateFromQColor(c);
}

QImage KisDisplayColorConverter::toQImage(KisPaintDeviceSP srcDevice) const
{
    KisPaintDeviceSP device = srcDevice;

    QRect bounds = srcDevice->exactBounds();
    if (bounds.isEmpty()) return QImage();


    if (m_d->useOcio()) {
        KIS_ASSERT_RECOVER(m_d->ocioInputColorSpace()->pixelSize() == 16) {
            return QImage();
        }

        device = new KisPaintDevice(*srcDevice);
        device->convertTo(m_d->ocioInputColorSpace());

        KisSequentialIterator it(device, bounds);
        int numConseqPixels = it.nConseqPixels();
        while (it.nextPixels(numConseqPixels)) {
            numConseqPixels = it.nConseqPixels();
            m_d->displayFilter->filter(it.rawData(), numConseqPixels);
        }

        device->setProfile(m_d->ocioOutputProfile());
    }

    // we expect the display profile is rgb8, which is BGRA here
    KIS_ASSERT_RECOVER(m_d->qtWidgetsColorSpace()->pixelSize() == 4) {
        return QImage();
    }

    return device->convertToQImage(m_d->qtWidgetsProfile(),
                                   bounds,
                                   m_d->renderingIntent, m_d->conversionFlags);
}

void KisDisplayColorConverter::applyDisplayFilteringF32(KisFixedPaintDeviceSP device,
                                                        const KoID &bitDepthId) const
{
    /**
     * This method is optimized for the case when device is already in 32f
     * version of the pating color space.
     */

    KIS_SAFE_ASSERT_RECOVER_RETURN(device->colorSpace()->colorDepthId() == Float32BitsColorDepthID);
    KIS_SAFE_ASSERT_RECOVER_RETURN(device->colorSpace()->colorModelId() == RGBAColorModelID);
    KIS_SAFE_ASSERT_RECOVER_RETURN(device->bounds().isValid());

    if (m_d->useOcio()) {
        KIS_ASSERT_RECOVER_RETURN(m_d->ocioInputColorSpace()->pixelSize() == 16);

        device->convertTo(m_d->ocioInputColorSpace());
        m_d->displayFilter->filter(device->data(), device->bounds().width() * device->bounds().height());
        device->setProfile(m_d->ocioOutputProfile());
    }

    device->convertTo(m_d->openGLSurfaceColorSpace(bitDepthId));
}

KoColor KisDisplayColorConverter::Private::approximateFromQColor(const QColor &qcolor)
{
    if (!useOcio()) {
        return KoColor(qcolor, paintingColorSpace);
    } else {
        KoColor color(qcolor, intermediateColorSpace());
        displayFilter->approximateInverseTransformation(color.data(), 1);
        color.convertTo(paintingColorSpace);
        return color;
    }

    qFatal("Must not be reachable");
    return KoColor();
}

QColor KisDisplayColorConverter::Private::approximateToQColor(const KoColor &srcColor)
{
    KoColor color(srcColor);

    if (useOcio()) {
        color.convertTo(intermediateColorSpace());
        displayFilter->approximateForwardTransformation(color.data(), 1);
    }

    return color.toQColor();
}


KoColor KisDisplayColorConverter::fromHsv(int h, int s, int v, int a) const
{
    // generate HSV from sRGB!
    QColor qcolor(QColor::fromHsv(h, s, v, a));
    return m_d->approximateFromQColor(qcolor);
}

void KisDisplayColorConverter::getHsv(const KoColor &srcColor, int *h, int *s, int *v, int *a) const
{
    // we are going through sRGB here!
    QColor color = m_d->approximateToQColor(srcColor);
    color.getHsv(h, s, v, a);
}

KoColor KisDisplayColorConverter::fromHsvF(qreal h, qreal s, qreal v, qreal a)
{
    // generate HSV from sRGB!
    QColor qcolor(QColor::fromHsvF(h, s, v, a));
    return m_d->approximateFromQColor(qcolor);
}

void KisDisplayColorConverter::getHsvF(const KoColor &srcColor, qreal *h, qreal *s, qreal *v, qreal *a)
{
    // we are going through sRGB here!
    QColor color = m_d->approximateToQColor(srcColor);
    color.getHsvF(h, s, v, a);
}

KoColor KisDisplayColorConverter::fromHslF(qreal h, qreal s, qreal l, qreal a)
{
    // generate HSL from sRGB!
    QColor qcolor(QColor::fromHslF(h, s, l, a));
    if (!qcolor.isValid()) {
        warnKrita << "Could not construct valid color from h" << h << "s" << s << "l" << l << "a" << a;
        qcolor = Qt::black;
    }
    return m_d->approximateFromQColor(qcolor);

}

void KisDisplayColorConverter::getHslF(const KoColor &srcColor, qreal *h, qreal *s, qreal *l, qreal *a)
{
    // we are going through sRGB here!
    QColor color = m_d->approximateToQColor(srcColor);
    color.getHslF(h, s, l, a);
}

KoColor KisDisplayColorConverter::fromHsiF(qreal h, qreal s, qreal i)
{
    // generate HSI from sRGB!
    qreal r=0.0;
    qreal g=0.0;
    qreal b=0.0;
    qreal a=1.0;
    HSIToRGB(h, s, i, &r, &g, &b);
    QColor qcolor;
    qcolor.setRgbF(qBound(0.0,r,1.0), qBound(0.0,g,1.0), qBound(0.0,b,1.0), a);
    return m_d->approximateFromQColor(qcolor);
}

void KisDisplayColorConverter::getHsiF(const KoColor &srcColor, qreal *h, qreal *s, qreal *i)
{
    // we are going through sRGB here!
    QColor color = m_d->approximateToQColor(srcColor);
    qreal r=color.redF();
    qreal g=color.greenF();
    qreal b=color.blueF();
    RGBToHSI(r, g, b, h, s, i);
}

KoColor KisDisplayColorConverter::fromHsyF(qreal h, qreal s, qreal y, qreal R, qreal G, qreal B, qreal gamma)
{
    // generate HSL from sRGB!
    QVector <qreal> channelValues(3);
    y = pow(y, gamma);
    HSYToRGB(h, s, y, &channelValues[0], &channelValues[1], &channelValues[2], R, G, B);
    KoColorSpaceRegistry::instance()->rgb8()->profile()->delinearizeFloatValueFast(channelValues);
    QColor qcolor;
    qcolor.setRgbF(qBound(0.0,channelValues[0],1.0), qBound(0.0,channelValues[1],1.0), qBound(0.0,channelValues[2],1.0), 1.0);
    return m_d->approximateFromQColor(qcolor);
}

void KisDisplayColorConverter::getHsyF(const KoColor &srcColor, qreal *h, qreal *s, qreal *y, qreal R, qreal G, qreal B, qreal gamma)
{
    // we are going through sRGB here!
    QColor color = m_d->approximateToQColor(srcColor);
    QVector <qreal> channelValues(3);
    channelValues[0]=color.redF();
    channelValues[1]=color.greenF();
    channelValues[2]=color.blueF();
    //TODO: if we're going to have KoColor here, remember to check whether the TRC of the profile exists...
    KoColorSpaceRegistry::instance()->rgb8()->profile()->linearizeFloatValueFast(channelValues);
    RGBToHSY(channelValues[0], channelValues[1], channelValues[2], h, s, y, R, G, B);
    *y = pow(*y, 1/gamma);
}

#include "moc_kis_display_color_converter.cpp"
