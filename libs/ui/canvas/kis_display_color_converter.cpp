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
          monitorColorSpace(0),
          monitorProfile(0),
          renderingIntent(KoColorConversionTransformation::internalRenderingIntent()),
          conversionFlags(KoColorConversionTransformation::internalConversionFlags()),
          displayFilter(0),
          intermediateColorSpace(0),
          displayRenderer(new DisplayRenderer(_q, _resourceManager)),
          expectedOcioOutputColorSpace(0)
    {
        QSurfaceFormat format = KisOpenGLModeProber::instance()->surfaceformatInUse();

        if (format.colorSpace() == QSurfaceFormat::scRGBColorSpace) {
            expectedOcioOutputColorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id());
        } else if (format.colorSpace() == QSurfaceFormat::bt2020PQColorSpace) {
            expectedOcioOutputColorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), KoColorSpaceRegistry::instance()->p2020PQProfile());
        }

        useHDRMode = KisOpenGLModeProber::instance()->useHDRMode();
    }

    KisDisplayColorConverter *const q;

    KoCanvasResourceProvider *resourceManager;

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

    const KoColorSpace *expectedOcioOutputColorSpace;
    bool useHDRMode = false;



    inline KoColor approximateFromQColor(const QColor &qcolor);
    inline QColor approximateToQColor(const KoColor &color);

    void slotCanvasResourceChanged(int key, const QVariant &v);
    void slotUpdateCurrentNodeColorSpace();
    void selectPaintingColorSpace();

    void updateIntermediateFgColor(const KoColor &color);
    void setCurrentNode(KisNodeSP node);
    bool useOcio() const;

    template <bool flipToBgra>
    QImage convertToQImageDirect(KisPaintDeviceSP device);

    class DisplayRenderer : public KoColorDisplayRendererInterface {
    public:
        DisplayRenderer(KisDisplayColorConverter *displayColorConverter, KoCanvasResourceProvider *resourceManager)
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
    return displayFilter && paintingColorSpace && paintingColorSpace->colorModelId() == RGBAColorModelID;
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

const KoColorProfile* KisDisplayColorConverter::openGLCanvasSurfaceProfile() const
{
    return m_d->useHDRMode ?
        KisOpenGLModeProber::instance()->rootSurfaceColorProfile() :
        monitorProfile();
}

bool KisDisplayColorConverter::isHDRMode() const
{
    return  m_d->useHDRMode;
}

bool finalIsRgba(const KoColorSpace *cs)
{
    /**
     * In Krita RGB color spaces differ: 8/16bit are BGRA, 16f/32f-bit RGBA
     */
    KoID colorDepthId = cs->colorDepthId();
    return colorDepthId == Float16BitsColorDepthID ||
        colorDepthId == Float32BitsColorDepthID;
}

template <bool flipToBgra>
QColor floatArrayToQColor(const float *p) {
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

namespace {
struct WriteToQColorPolicy {
    using Result = QColor;

    static QColor convertWhenNoOcio(const KoColor &c) {
        const quint8 *p = c.data();
        return QColor(p[2], p[1], p[0], p[3]);
    }

    static QColor convertWhenOcio(const QVector<float> &values, const KoColorSpace *expectedOcioOutputColorSpace, const KoColorSpace *baseColorSpace) {
        const float *p = values.constData();

        if (!expectedOcioOutputColorSpace) {
            return floatArrayToQColor<true>(p);
        } else {
            const KoColorSpace *sRGB = KoColorSpaceRegistry::instance()->rgb8();
            QVector<quint8> dstPixelData(4);

            expectedOcioOutputColorSpace->
                convertPixelsTo((const quint8*)p, (quint8*)dstPixelData.data(),
                                sRGB, 1,
                                KoColorConversionTransformation::internalRenderingIntent(),
                                KoColorConversionTransformation::internalConversionFlags());

            return QColor(dstPixelData[2], dstPixelData[1], dstPixelData[0], dstPixelData[3]);
        }
    }
};

struct WriteToKoColorPolicy {
    using Result = KoColor;

    static KoColor convertWhenNoOcio(const KoColor &c) {
        return c;
    }

    static KoColor convertWhenOcio(const QVector<float> &values, const KoColorSpace *expectedOcioOutputColorSpace, const KoColorSpace *baseColorSpace) {

        if (expectedOcioOutputColorSpace) {
            KoColor c(expectedOcioOutputColorSpace);
            expectedOcioOutputColorSpace->fromNormalisedChannelsValue(c.data(), values);
            return c;
        } else {
            KoColor c(baseColorSpace);
            baseColorSpace->fromNormalisedChannelsValue(c.data(), values);
            return c;
        }
    }
};
}

template <class Policy>
typename Policy::Result KisDisplayColorConverter::convertToDisplayImpl(const KoColor &srcColor, bool alreadyInDestinationF32) const
{
    KoColor c(srcColor);
    if (!alreadyInDestinationF32) {
        c.convertTo(m_d->paintingColorSpace);
    }

    if (!m_d->useOcio()) {
        // we expect the display profile is rgb8, which is BGRA here
        KIS_ASSERT_RECOVER(m_d->monitorColorSpace->pixelSize() == 4) { return typename Policy::Result(); };

        c.convertTo(m_d->monitorColorSpace, m_d->renderingIntent, m_d->conversionFlags);

        return Policy::convertWhenNoOcio(c);
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

        if (!finalIsRgba(srcCS)) {
            std::swap(normalizedChannels[0], normalizedChannels[2]);
        }

        m_d->displayFilter->filter((quint8*)normalizedChannels.data(), 1);

        return Policy::convertWhenOcio(normalizedChannels, m_d->expectedOcioOutputColorSpace, srcCS);
    }
}

QColor KisDisplayColorConverter::toQColor(const KoColor &srcColor) const
{
    return convertToDisplayImpl<WriteToQColorPolicy>(srcColor);
}

KoColor KisDisplayColorConverter::applyDisplayFiltering(const KoColor &c, bool alreadyInF32) const
{
    if (alreadyInF32) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(c.colorSpace()->colorDepthId() == Float32BitsColorDepthID, c);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(c.colorSpace()->colorModelId() == RGBAColorModelID, c);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(c.colorSpace()->profile()->uniqueId() == m_d->paintingColorSpace->profile()->uniqueId(), c);
    }

    return convertToDisplayImpl<WriteToKoColorPolicy>(c, alreadyInF32);
}

bool KisDisplayColorConverter::canSkipDisplayConversion(const KoColorSpace *cs) const
{
    const KoColorProfile *displayProfile = this->openGLCanvasSurfaceProfile();

    return !m_d->useOcio() && !m_d->expectedOcioOutputColorSpace &&
        cs->colorModelId() == RGBAColorModelID &&
        (!!cs->profile() == !!displayProfile) &&
        (!cs->profile() ||
         cs->profile()->uniqueId() == displayProfile->uniqueId());
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

    if (!expectedOcioOutputColorSpace) {
        while (it.nextPixel()) {
            cs->normalisedChannelsValue(it.rawDataConst(), normalizedChannels);

            if (!flipToBgra) {
                std::swap(normalizedChannels[0], normalizedChannels[2]);
            }

            displayFilter->filter((quint8*)normalizedChannels.data(), 1);

            const float *p = normalizedChannels.constData();

            dstPtr[0] = KoColorSpaceMaths<float, quint8>::scaleToA(p[2]);
            dstPtr[1] = KoColorSpaceMaths<float, quint8>::scaleToA(p[1]);
            dstPtr[2] = KoColorSpaceMaths<float, quint8>::scaleToA(p[0]);
            dstPtr[3] = KoColorSpaceMaths<float, quint8>::scaleToA(p[3]);

            dstPtr += 4;
        }
    } else {
        const KoColorSpace *sRGB = KoColorSpaceRegistry::instance()->rgb8();

        while (it.nextPixel()) {
            cs->normalisedChannelsValue(it.rawDataConst(), normalizedChannels);

            if (!flipToBgra) {
                std::swap(normalizedChannels[0], normalizedChannels[2]);
            }

            displayFilter->filter((quint8*)normalizedChannels.data(), 1);

            expectedOcioOutputColorSpace->
                convertPixelsTo((const quint8*)normalizedChannels.data(), dstPtr,
                                sRGB, 1,
                                KoColorConversionTransformation::internalRenderingIntent(),
                                KoColorConversionTransformation::internalConversionFlags());


            dstPtr += 4;
        }
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
        ENTER_FUNCTION() << ppVar(m_d->monitorProfile->name());

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

        return finalIsRgba(device->colorSpace()) ?
            m_d->convertToQImageDirect<true>(device) :
            m_d->convertToQImageDirect<false>(device);
    }

    return QImage();
}

void KisDisplayColorConverter::applyDisplayFilteringF32(KisFixedPaintDeviceSP device)
{
    /**
     * This method is optimized for the case when device is already in 32f
     * version of the pating color space.
     */

    KIS_SAFE_ASSERT_RECOVER_RETURN(device->colorSpace()->colorDepthId() == Float32BitsColorDepthID);
    KIS_SAFE_ASSERT_RECOVER_RETURN(device->colorSpace()->colorModelId() == RGBAColorModelID);

    if (!m_d->useOcio()) {
        if (m_d->monitorProfile) {
            const KoColorSpace *monitorColorSpaceF32 =
                KoColorSpaceRegistry::instance()->colorSpace(
                    RGBAColorModelID.id(),
                    Float32BitsColorDepthID.id(),
                    m_d->monitorProfile);

            device->convertTo(monitorColorSpaceF32, m_d->renderingIntent, m_d->conversionFlags);
        }

        // TODO: this conversion doesn't look to be correct in the case
        //       when we have !useOcio && !m_d->monitorProfile && paintingColorSpace != rgb-8bit
        if (m_d->expectedOcioOutputColorSpace) {
            device->convertTo(m_d->expectedOcioOutputColorSpace);
        }
    } else {
        if (m_d->monitorProfile && m_d->displayFilter->useInternalColorManagement()) {
            const KoColorSpace *srcCS =
                KoColorSpaceRegistry::instance()->colorSpace(
                    RGBAColorModelID.id(),
                    Float32BitsColorDepthID.id(),
                    m_d->monitorProfile);

            device->convertTo(srcCS, m_d->renderingIntent, m_d->conversionFlags);
        } else if (*device->colorSpace() != *m_d->paintingColorSpace) {
            const KoColorSpace *imageCS =
                KoColorSpaceRegistry::instance()->colorSpace(
                    RGBAColorModelID.id(),
                    Float32BitsColorDepthID.id(),
                    m_d->paintingColorSpace->profile());

            device->convertTo(imageCS);
        }

        m_d->displayFilter->filter(device->data(), device->bounds().width() * device->bounds().height());

        // HACK ALERT!
        if (m_d->expectedOcioOutputColorSpace && m_d->displayFilter->useInternalColorManagement()) {
            device->convertTo(m_d->expectedOcioOutputColorSpace);
        }
    }
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
