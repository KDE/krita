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

#include <KoCanvasResourceManager.h>
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

Q_GLOBAL_STATIC(KisDisplayColorConverter, s_instance)


struct KisDisplayColorConverter::Private
{
    Private(KisDisplayColorConverter *_q, KoCanvasResourceManager *_resourceManager)
        : q(_q),
          resourceManager(_resourceManager),
          nodeColorSpace(0),
          paintingColorSpace(0),
          monitorColorSpace(0),
          monitorProfile(0),
          renderingIntent(KoColorConversionTransformation::internalRenderingIntent()),
          conversionFlags(KoColorConversionTransformation::internalConversionFlags()),
          displayFilter(0),
          intermediateColorSpace(0),
          displayRenderer(new DisplayRenderer(_q, _resourceManager))
    {
    }

    KisDisplayColorConverter *const q;

    KoCanvasResourceManager *resourceManager;

    const KoColorSpace *nodeColorSpace;
    const KoColorSpace *paintingColorSpace;
    const KoColorSpace *monitorColorSpace;

    const KoColorProfile *monitorProfile;

    KoColorConversionTransformation::Intent renderingIntent;
    KoColorConversionTransformation::ConversionFlags conversionFlags;

    QSharedPointer<KisDisplayFilter> displayFilter;
    const KoColorSpace *intermediateColorSpace;

    KoColor intermediateFgColor;
    KisNodeSP connectedNode;

    inline KoColor approximateFromQColor(const QColor &qcolor);
    inline QColor approximateToQColor(const KoColor &color);

    void slotCanvasResourceChanged(int key, const QVariant &v);
    void slotUpdateCurrentNodeColorSpace();
    void selectPaintingColorSpace();

    void updateIntermediateFgColor(const KoColor &color);
    void setCurrentNode(KisNodeSP node);
    bool useOcio() const;

    bool finalIsRgba(const KoColorSpace *cs) const;

    template <bool flipToBgra>
    QColor floatArrayToQColor(const float *p);

    template <bool flipToBgra>
    QImage convertToQImageDirect(KisPaintDeviceSP device);

    class DisplayRenderer : public KoColorDisplayRendererInterface {
    public:
        DisplayRenderer(KisDisplayColorConverter *displayColorConverter, KoCanvasResourceManager *resourceManager)
            : m_displayColorConverter(displayColorConverter),
              m_resourceManager(resourceManager)
        {
            displayColorConverter->connect(displayColorConverter, SIGNAL(displayConfigurationChanged()),
                            this, SIGNAL(displayConfigurationChanged()));
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
        QPointer<KoCanvasResourceManager> m_resourceManager;
    };

    QScopedPointer<KoColorDisplayRendererInterface> displayRenderer;
};

KisDisplayColorConverter::KisDisplayColorConverter(KoCanvasResourceManager *resourceManager, QObject *parent)
    : QObject(parent),
      m_d(new Private(this, resourceManager))
{

    connect(m_d->resourceManager, SIGNAL(canvasResourceChanged(int,QVariant)),
            SLOT(slotCanvasResourceChanged(int,QVariant)));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()),
            SLOT(selectPaintingColorSpace()));


    m_d->setCurrentNode(0);
    setMonitorProfile(0);
    setDisplayFilter(QSharedPointer<KisDisplayFilter>(0));
}

KisDisplayColorConverter::KisDisplayColorConverter()
    : m_d(new Private(this, 0))
{
    setDisplayFilter(QSharedPointer<KisDisplayFilter>(0));

    m_d->paintingColorSpace = KoColorSpaceRegistry::instance()->rgb8();

    m_d->setCurrentNode(0);
    setMonitorProfile(0);

}

KisDisplayColorConverter::~KisDisplayColorConverter()
{
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
    return displayFilter && paintingColorSpace->colorModelId() == RGBAColorModelID;
}

void KisDisplayColorConverter::Private::updateIntermediateFgColor(const KoColor &srcColor)
{
    KIS_ASSERT_RECOVER_RETURN(displayFilter);

    KoColor color = srcColor;
    color.convertTo(intermediateColorSpace);
    displayFilter->approximateForwardTransformation(color.data(), 1);
    intermediateFgColor = color;
}

void KisDisplayColorConverter::Private::slotCanvasResourceChanged(int key, const QVariant &v)
{
    if (key == KisCanvasResourceProvider::CurrentKritaNode) {
        KisNodeSP currentNode = v.value<KisNodeWSP>();
        setCurrentNode(currentNode);
    } else if (useOcio() && key == KoCanvasResourceManager::ForegroundColor) {
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
    m_d->monitorColorSpace = KoColorSpaceRegistry::instance()->rgb8(monitorProfile);
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
    m_d->intermediateColorSpace = 0;

    if (m_d->displayFilter) {
        // choosing default profile, which is scRGB
        const KoColorProfile *intermediateProfile = 0;
        m_d->intermediateColorSpace =
            KoColorSpaceRegistry::instance()->
            colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), intermediateProfile);

        KIS_ASSERT_RECOVER(m_d->intermediateColorSpace) {
            m_d->intermediateColorSpace = m_d->monitorColorSpace;
        }

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

bool KisDisplayColorConverter::Private::finalIsRgba(const KoColorSpace *cs) const
{
    /**
     * In Krita RGB color spaces differ: 8/16bit are BGRA, 16f/32f-bit RGBA
     */
    KoID colorDepthId = cs->colorDepthId();
    return colorDepthId == Float16BitsColorDepthID ||
        colorDepthId == Float32BitsColorDepthID;
}

template <bool flipToBgra>
QColor KisDisplayColorConverter::Private::floatArrayToQColor(const float *p) {
    if (flipToBgra) {
        return QColor(KoColorSpaceMaths<float, quint8>::scaleToA(p[0]),
                      KoColorSpaceMaths<float, quint8>::scaleToA(p[1]),
                      KoColorSpaceMaths<float, quint8>::scaleToA(p[2]),
                      KoColorSpaceMaths<float, quint8>::scaleToA(p[3]));
    } else {
        return QColor(KoColorSpaceMaths<float, quint8>::scaleToA(p[2]),
                      KoColorSpaceMaths<float, quint8>::scaleToA(p[1]),
                      KoColorSpaceMaths<float, quint8>::scaleToA(p[0]),
                      KoColorSpaceMaths<float, quint8>::scaleToA(p[3]));
    }
}

QColor KisDisplayColorConverter::toQColor(const KoColor &srcColor) const
{
    KoColor c(srcColor);
    c.convertTo(m_d->paintingColorSpace);

    if (!m_d->useOcio()) {
        // we expect the display profile is rgb8, which is BGRA here
        KIS_ASSERT_RECOVER(m_d->monitorColorSpace->pixelSize() == 4) { return Qt::red; };

        c.convertTo(m_d->monitorColorSpace, m_d->renderingIntent, m_d->conversionFlags);

        const quint8 *p = c.data();
        return QColor(p[2], p[1], p[0], p[3]);
    } else {
        const KoColorSpace *srcCS = c.colorSpace();

        if (m_d->displayFilter->useInternalColorManagement()) {
            srcCS = KoColorSpaceRegistry::instance()->colorSpace(
                RGBAColorModelID.id(),
                Float32BitsColorDepthID.id(),
                m_d->monitorProfile);
            c.convertTo(srcCS, m_d->renderingIntent, m_d->conversionFlags);
        }

        int numChannels = srcCS->channelCount();
        QVector<float> normalizedChannels(numChannels);
        srcCS->normalisedChannelsValue(c.data(), normalizedChannels);
        m_d->displayFilter->filter((quint8*)normalizedChannels.data(), 1);

        const float *p = (const float *)normalizedChannels.constData();

        return m_d->finalIsRgba(srcCS) ?
            m_d->floatArrayToQColor<true>(p) :
            m_d->floatArrayToQColor<false>(p);
    }
}

KoColor KisDisplayColorConverter::approximateFromRenderedQColor(const QColor &c) const
{
    return m_d->approximateFromQColor(c);
}

template <bool flipToBgra>
QImage
KisDisplayColorConverter::Private::convertToQImageDirect(KisPaintDeviceSP device)
{
    QRect bounds = device->exactBounds();
    if (bounds.isEmpty()) return QImage();

    QImage image(bounds.size(), QImage::Format_ARGB32);

    KisSequentialConstIterator it(device, bounds);
    quint8 *dstPtr = image.bits();

    const KoColorSpace *cs = device->colorSpace();
    int numChannels = cs->channelCount();
    QVector<float> normalizedChannels(numChannels);

    while (it.nextPixel()) {
        cs->normalisedChannelsValue(it.rawDataConst(), normalizedChannels);
        displayFilter->filter((quint8*)normalizedChannels.data(), 1);

        const float *p = normalizedChannels.constData();

        if (flipToBgra) {
            dstPtr[0] = KoColorSpaceMaths<float, quint8>::scaleToA(p[2]);
            dstPtr[1] = KoColorSpaceMaths<float, quint8>::scaleToA(p[1]);
            dstPtr[2] = KoColorSpaceMaths<float, quint8>::scaleToA(p[0]);
            dstPtr[3] = KoColorSpaceMaths<float, quint8>::scaleToA(p[3]);
        } else {
            dstPtr[0] = KoColorSpaceMaths<float, quint8>::scaleToA(p[0]);
            dstPtr[1] = KoColorSpaceMaths<float, quint8>::scaleToA(p[1]);
            dstPtr[2] = KoColorSpaceMaths<float, quint8>::scaleToA(p[2]);
            dstPtr[3] = KoColorSpaceMaths<float, quint8>::scaleToA(p[3]);
        }

        dstPtr += 4;
    }

    return image;
}

QImage KisDisplayColorConverter::toQImage(KisPaintDeviceSP srcDevice) const
{
    KisPaintDeviceSP device = srcDevice;
    if (*device->colorSpace() != *m_d->paintingColorSpace) {
        device = new KisPaintDevice(*srcDevice);

        KUndo2Command *cmd = device->convertTo(m_d->paintingColorSpace);
        delete cmd;
    }

    if (!m_d->useOcio()) {
        return device->convertToQImage(m_d->monitorProfile, m_d->renderingIntent, m_d->conversionFlags);
    } else {
        if (m_d->displayFilter->useInternalColorManagement()) {
            if (device == srcDevice) {
                device = new KisPaintDevice(*srcDevice);
            }

            const KoColorSpace *srcCS =
                KoColorSpaceRegistry::instance()->colorSpace(
                    RGBAColorModelID.id(),
                    Float32BitsColorDepthID.id(),
                    m_d->monitorProfile);

            KUndo2Command *cmd = device->convertTo(srcCS, m_d->renderingIntent, m_d->conversionFlags);
            delete cmd;
        }

        return m_d->finalIsRgba(device->colorSpace()) ?
            m_d->convertToQImageDirect<true>(device) :
            m_d->convertToQImageDirect<false>(device);
    }

    return QImage();
}

KoColor KisDisplayColorConverter::Private::approximateFromQColor(const QColor &qcolor)
{
    if (!useOcio()) {
        return KoColor(qcolor, paintingColorSpace);
    } else {
        KoColor color(qcolor, intermediateColorSpace);
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
        color.convertTo(intermediateColorSpace);
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
