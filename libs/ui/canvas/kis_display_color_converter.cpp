/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "KisDisplayConfig.h"

Q_GLOBAL_STATIC(KisDisplayColorConverter, s_instance)


struct KisDisplayColorConverter::Private
{
    Private(KisDisplayColorConverter *_q, KoCanvasResourceProvider *_resourceManager)
        : q(_q),
          resourceManager(_resourceManager),
          nodeColorSpace(0),
          paintingColorSpace(0),
          displayFilter(0),
          displayRenderer(new DisplayRenderer(_q, _resourceManager))
    {
    }

    KisDisplayColorConverter *const q;

    KoCanvasResourceProvider *resourceManager;

    const KoColorSpace *nodeColorSpace;
    const KoColorSpace *paintingColorSpace;

    const KoColorProfile* inputImageProfile = 0;

    mutable const KoColorSpace *cachedOcioInputColorSpace = 0;
    mutable const KoColorSpace *cachedOcioOutputColorSpace = 0;
    mutable const KoColorSpace *cachedQtWidgetsColorSpace = 0;
    mutable const KoColorSpace *cachedOpenGLSurfaceColorSpace = 0;

    // this color space will never change during the run of Krita
    mutable const KoColorSpace *cachedIntermediateColorSpace = 0;

    void notifyDisplayConfigurationChanged() {
        cachedOcioInputColorSpace = 0;
        cachedOcioOutputColorSpace = 0;
        cachedQtWidgetsColorSpace = 0;
        cachedOpenGLSurfaceColorSpace = 0;

        Q_EMIT q->displayConfigurationChanged();
    }

    const KoColorProfile* qtWidgetsProfile() const {
        return multiSurfaceDisplayConfig.uiProfile;
    }

    const KoColorProfile* openGLSurfaceProfile() const {
        return multiSurfaceDisplayConfig.canvasProfile;
    }

    const KoColorProfile* ocioInputProfile() const {
        return displayFilter && displayFilter->useInternalColorManagement() ?
                    openGLSurfaceProfile() : inputImageProfile;
    }

    const KoColorProfile* ocioOutputProfile() const {
        return openGLSurfaceProfile();
    }

    const KoColorSpace* ocioInputColorSpace() const {
        return cachedOcioInputColorSpace
            ? cachedOcioInputColorSpace
            : (cachedOcioInputColorSpace =
                KoColorSpaceRegistry::instance()->
                                colorSpace(
                                    RGBAColorModelID.id(),
                                    Float32BitsColorDepthID.id(),
                                    ocioInputProfile()));
    }

    const KoColorSpace* ocioOutputColorSpace() const {
        return cachedOcioOutputColorSpace
            ? cachedOcioOutputColorSpace
            : (cachedOcioOutputColorSpace =
                KoColorSpaceRegistry::instance()->
                                colorSpace(
                                    RGBAColorModelID.id(),
                                    Float32BitsColorDepthID.id(),
                                    ocioOutputProfile()));

    }

    const KoColorSpace* qtWidgetsColorSpace() const {
        return cachedQtWidgetsColorSpace
            ? cachedQtWidgetsColorSpace
            : (cachedQtWidgetsColorSpace =
                KoColorSpaceRegistry::instance()->
                                colorSpace(
                                    RGBAColorModelID.id(),
                                    Integer8BitsColorDepthID.id(),
                                    qtWidgetsProfile()));

    }

    const KoColorSpace* openGLSurfaceColorSpace(const KoID &bitDepthId) const {
        return cachedOpenGLSurfaceColorSpace
            ? cachedOpenGLSurfaceColorSpace
            : (cachedOpenGLSurfaceColorSpace =
                KoColorSpaceRegistry::instance()->
                                colorSpace(
                                    RGBAColorModelID.id(),
                                    bitDepthId.id(),
                                    openGLSurfaceProfile()));
    }

    const KoColorSpace* intermediateColorSpace() const {
        // the color space where we apply exposure and
        // gamma should always be linear
        return cachedIntermediateColorSpace
            ? cachedIntermediateColorSpace
            : (cachedIntermediateColorSpace =
                KoColorSpaceRegistry::instance()->
                                colorSpace(
                                    RGBAColorModelID.id(),
                                    Float32BitsColorDepthID.id(),
                                    KoColorSpaceRegistry::instance()->p2020G10Profile()));
    }

    KisMultiSurfaceDisplayConfig multiSurfaceDisplayConfig;

    QSharedPointer<KisDisplayFilter> displayFilter;

    KoColor intermediateFgColor;
    KisNodeSP connectedNode;
    KisImageSP image;

    inline KoColor approximateFromQColor(const QColor &qcolor);
    inline QColor approximateToQColor(const KoColor &color);

    void slotCanvasResourceChanged(int key, const QVariant &v);
    void slotUpdateCurrentNodeColorSpace();
    void selectPaintingColorSpace();

    void updateIntermediateFgColor(const KoColor &color);
    void setCurrentNode(KisNodeSP node);
    bool useOcio() const;
    bool needsColorProofing(const KoColorSpace *srcColorSpace) const;

    class DisplayRenderer : public KoColorDisplayRendererInterface {
    public:
        DisplayRenderer(KisDisplayColorConverter *displayColorConverter, KoCanvasResourceProvider *resourceManager)
            : m_displayColorConverter(displayColorConverter),
              m_resourceManager(resourceManager)
        {
            displayColorConverter->connect(displayColorConverter, SIGNAL(displayConfigurationChanged()),
                            this, SIGNAL(displayConfigurationChanged()), Qt::UniqueConnection);
        }

        QImage toQImage(const KoColorSpace *srcColorSpace, const quint8 *data, QSize size, bool proofPaintColors = false) const override {
            QImage result = m_displayColorConverter->toQImage(srcColorSpace, data, size, proofPaintColors);
            return result;
        }

        QColor toQColor(const KoColor &c, bool proofToPaintColors = false) const override {
            return m_displayColorConverter->toQColor(c, proofToPaintColors);
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
                qreal exposure = m_resourceManager->resource(KoCanvasResource::HdrExposure).value<qreal>();
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
    setDisplayFilter(QSharedPointer<KisDisplayFilter>(0));
}

KisDisplayColorConverter::KisDisplayColorConverter()
    : m_d(new Private(this, 0))
{
    setDisplayFilter(QSharedPointer<KisDisplayFilter>(0));

    m_d->inputImageProfile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    m_d->paintingColorSpace = KoColorSpaceRegistry::instance()->rgb8();

    m_d->setCurrentNode(0);
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

    m_d->notifyDisplayConfigurationChanged();
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

bool KisDisplayColorConverter::Private::needsColorProofing(const KoColorSpace *srcColorSpace) const
{
    if (!paintingColorSpace || srcColorSpace == paintingColorSpace || *srcColorSpace == *paintingColorSpace) {
        return false;
    }
    // TODO: ideally, we'd identify color profiles that only differ in transfer curves but
    // define the same primaries/gamut and return false for them aswell
    if (srcColorSpace->colorModelId() == paintingColorSpace->colorModelId()) {
        const KoColorProfile *paintProfile = paintingColorSpace->profile();
        const KoColorProfile *srcProfile = srcColorSpace->profile();
        bool matchingProfiles = (paintProfile == srcProfile) ||
                                (paintProfile && srcProfile && *paintProfile == *srcProfile);
        // unless we go float->int, the color spaces are considered compatible
        if (matchingProfiles &&
                (srcColorSpace->colorDepthId() == Integer8BitsColorDepthID ||
                 srcColorSpace->colorDepthId() == Integer16BitsColorDepthID ||
                 paintingColorSpace->colorModelId() == Float16BitsColorDepthID ||
                 paintingColorSpace->colorModelId() == Float32BitsColorDepthID)) {
            return false;
        }
    }
    return true;
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
    if (key == KoCanvasResource::CurrentKritaNode) {
        KisNodeSP currentNode = v.value<KisNodeWSP>();
        setCurrentNode(currentNode);
    } else if (useOcio() && key == KoCanvasResource::ForegroundColor) {
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

    notifyDisplayConfigurationChanged();
}

const KoColorSpace* KisDisplayColorConverter::paintingColorSpace() const
{
    KIS_ASSERT_RECOVER(m_d->paintingColorSpace) {
        return KoColorSpaceRegistry::instance()->rgb8();
    }

    return m_d->paintingColorSpace;
}

const KoColorSpace *KisDisplayColorConverter::nodeColorSpace() const
{
    return m_d->nodeColorSpace;
}

void KisDisplayColorConverter::setMultiSurfaceDisplayConfig(const KisMultiSurfaceDisplayConfig &config)
{
    if (m_d->multiSurfaceDisplayConfig == config) return;



    m_d->multiSurfaceDisplayConfig = config;
    m_d->notifyDisplayConfigurationChanged();
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

KisDisplayConfig KisDisplayColorConverter::displayConfig() const
{
    return m_d->multiSurfaceDisplayConfig.uiDisplayConfig();
}

QSharedPointer<KisDisplayFilter> KisDisplayColorConverter::displayFilter() const
{
    return m_d->displayFilter;
}

KisMultiSurfaceDisplayConfig KisDisplayColorConverter::multiSurfaceDisplayConfig() const
{
    return m_d->multiSurfaceDisplayConfig;
}

KisDisplayColorConverter::ConversionOptions KisDisplayColorConverter::conversionOptions() const
{
    return m_d->multiSurfaceDisplayConfig.options();
}

QColor KisDisplayColorConverter::toQColor(const KoColor &srcColor, bool proofToPaintColors) const
{
    KoColor c(srcColor);

    if (proofToPaintColors && m_d->needsColorProofing(c.colorSpace())) {
        c.convertTo(m_d->paintingColorSpace, m_d->multiSurfaceDisplayConfig.intent, m_d->multiSurfaceDisplayConfig.conversionFlags);
    }

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

    c.convertTo(m_d->qtWidgetsColorSpace(), m_d->multiSurfaceDisplayConfig.intent, m_d->multiSurfaceDisplayConfig.conversionFlags);
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

    c.convertTo(m_d->openGLSurfaceColorSpace(bitDepthId), m_d->multiSurfaceDisplayConfig.intent, m_d->multiSurfaceDisplayConfig.conversionFlags);
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

QImage KisDisplayColorConverter::toQImage(KisPaintDeviceSP srcDevice, bool proofPaintColors) const
{
    KisPaintDeviceSP device = srcDevice;

    QRect bounds = srcDevice->exactBounds();
    if (bounds.isEmpty()) return QImage();

    if (proofPaintColors && m_d->needsColorProofing(srcDevice->colorSpace())) {
        srcDevice->convertTo(paintingColorSpace(), m_d->multiSurfaceDisplayConfig.intent, m_d->multiSurfaceDisplayConfig.conversionFlags);
    }

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

        device->setProfile(m_d->ocioOutputProfile(), 0);
    }

    // we expect the display profile is rgb8, which is BGRA here
    KIS_ASSERT_RECOVER(m_d->qtWidgetsColorSpace()->pixelSize() == 4) {
        return QImage();
    }

    return device->convertToQImage(m_d->qtWidgetsProfile(),
                                   bounds,
                                   m_d->multiSurfaceDisplayConfig.intent,
                                   m_d->multiSurfaceDisplayConfig.conversionFlags);
}

QImage KisDisplayColorConverter::toQImage(const KoColorSpace *srcColorSpace, const quint8 *data, QSize size, bool proofPaintColors) const
{
    const int numPixels = size.width() * size.height();

    const KoColorSpace  *colorSpace = srcColorSpace;
    const quint8 *pixels = data;

    QScopedArrayPointer<quint8> proofBuffer;

    if (proofPaintColors && m_d->needsColorProofing(srcColorSpace)) {
        const int imageSize = numPixels * paintingColorSpace()->pixelSize();
        proofBuffer.reset(new quint8[imageSize]);
        colorSpace->convertPixelsTo(pixels, proofBuffer.data(),
                                      paintingColorSpace(),
                                      numPixels,
                                      m_d->multiSurfaceDisplayConfig.intent,
                                      m_d->multiSurfaceDisplayConfig.conversionFlags);
        colorSpace = paintingColorSpace();
        pixels = proofBuffer.data();
    }

    QScopedArrayPointer<quint8> ocioBuffer;
    if (m_d->useOcio()) {
        const int imageSize = numPixels * m_d->ocioInputColorSpace()->pixelSize();
        ocioBuffer.reset(new quint8[imageSize]);
        colorSpace->convertPixelsTo(pixels, ocioBuffer.data(),
                                      m_d->ocioInputColorSpace(),
                                      numPixels,
                                      m_d->multiSurfaceDisplayConfig.intent,
                                      m_d->multiSurfaceDisplayConfig.conversionFlags);
        m_d->displayFilter->filter(ocioBuffer.data(), numPixels);

        return m_d->ocioOutputColorSpace()->convertToQImage(ocioBuffer.data(), size.width(), size.height(),
                                                            m_d->qtWidgetsProfile(),
                                                            m_d->multiSurfaceDisplayConfig.intent,
                                                            m_d->multiSurfaceDisplayConfig.conversionFlags);
    }

    return colorSpace->convertToQImage(pixels, size.width(), size.height(),
                                          m_d->qtWidgetsProfile(),
                                          m_d->multiSurfaceDisplayConfig.intent,
                                          m_d->multiSurfaceDisplayConfig.conversionFlags);
}

void KisDisplayColorConverter::applyDisplayFilteringF32(KisFixedPaintDeviceSP device,
                                                        const KoColorSpace *dstColorSpace) const
{
    /**
     * This method is optimized for the case when device is already in 32f
     * version of the painting color space.
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

    KIS_SAFE_ASSERT_RECOVER_RETURN(dstColorSpace);
    device->convertTo(dstColorSpace);
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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    color.getHsvF(h, s, v, a);
#else
    float fH, fS, fV, fA;
    fH = *h;
    fS = *s;
    fV = *v;
    if (a) {
        fA = *a;
    }
    color.getHsvF(&fH, &fS, &fV, &fA);
    *h = fH;
    *s = fS;
    *v = fV;
    if (a) {
        *a = fA;
    }
#endif
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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    color.getHslF(h, s, l, a);
#else
    float fH, fS, fL, fA;
    fH = *h;
    fS = *s;
    fL = *l;
    if (a) {
        fA = *a;
    }
    color.getHslF(&fH, &fS, &fL, &fA);
    *h = fH;
    *s = fS;
    *l = fL;
    if (a) {
        *a = fA;
    }
#endif
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
