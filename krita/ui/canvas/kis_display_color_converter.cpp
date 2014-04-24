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

#include <KoColor.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

#include <KoCanvasResourceManager.h>
#include "kis_config_notifier.h"
#include "kis_canvas_resource_provider.h"
#include "kis_canvas2.h"
#include "kis_view2.h"
#include "kis_image.h"
#include "kis_node.h"

#include "kundo2command.h"
#include "kis_config.h"
#include "kis_paint_device.h"
#include "kis_iterator_ng.h"


struct KisDisplayColorConverter::Private
{
    Private(KisDisplayColorConverter *_q)
        : q(_q),
          parentCanvas(0),
          nodeColorSpace(0),
          paintingColorSpace(0),
          monitorColorSpace(0),
          monitorProfile(0),
          intermediateColorSpace(0)
    {
    }

    KisDisplayColorConverter * const q;

    KisCanvas2 *parentCanvas;

    const KoColorSpace *nodeColorSpace;
    const KoColorSpace *paintingColorSpace;
    const KoColorSpace *monitorColorSpace;

    const KoColorProfile *monitorProfile;

    KoColorConversionTransformation::Intent renderingIntent;
    KoColorConversionTransformation::ConversionFlags conversionFlags;

    KisDisplayFilterSP displayFilter;
    const KoColorSpace *intermediateColorSpace;

    KoColor intermediateFgColor;
    KisNodeSP connectedNode;

    inline KoColor approximateFromQColor(const QColor &qcolor);
    inline QColor approximateToQColor(const KoColor &color);

    void slotCanvasResourceChanged(int key, const QVariant &v);
    void slotUpdateCurrentNodeColorSpace();
    void selectPaintingColorSpace();

    void setCurrentNode(KisNodeSP node);
    bool useOcio() const;
};

KisDisplayColorConverter::KisDisplayColorConverter(KisCanvas2 *parentCanvas)
    : QObject(parentCanvas),
      m_d(new Private(this))
{
    m_d->parentCanvas = parentCanvas;

    connect(m_d->parentCanvas->resourceManager(), SIGNAL(canvasResourceChanged(int, const QVariant&)),
            SLOT(slotCanvasResourceChanged(int, const QVariant&)));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()),
            SLOT(selectPaintingColorSpace()));


    m_d->setCurrentNode(0);
    setMonitorProfile(0);
    setDisplayFilter(KisDisplayFilterSP());
}

KisDisplayColorConverter::KisDisplayColorConverter()
    : m_d(new Private(this))
{
    m_d->paintingColorSpace = KoColorSpaceRegistry::instance()->rgb8();

    m_d->setCurrentNode(0);
    setMonitorProfile(0);
    setDisplayFilter(KisDisplayFilterSP());
}

KisDisplayColorConverter::~KisDisplayColorConverter()
{
}

KisDisplayColorConverter* KisDisplayColorConverter::dumbConverterInstance()
{
    K_GLOBAL_STATIC(KisDisplayColorConverter, s_instance);
    return s_instance;
}

bool KisDisplayColorConverter::Private::useOcio() const
{
    return displayFilter && paintingColorSpace->colorModelId() == RGBAColorModelID;
}

void KisDisplayColorConverter::Private::slotCanvasResourceChanged(int key, const QVariant &v)
{
    if (key == KisCanvasResourceProvider::CurrentKritaNode) {
        KisNodeSP currentNode = v.value<KisNodeSP>();
        setCurrentNode(currentNode);
    } else if (useOcio() && key == KoCanvasResourceManager::ForegroundColor) {
        KoColor color = v.value<KoColor>();
        color.convertTo(intermediateColorSpace);
        displayFilter->approximateForwardTransformation(color.data(), 1);
        intermediateFgColor = color;
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

    if (node) {
        KisPaintDeviceSP device = findValidDevice(node);

        nodeColorSpace = device ?
            device->compositionSourceColorSpace() :
            node->colorSpace();

        KIS_ASSERT_RECOVER_NOOP(nodeColorSpace);

        if (device) {
            q->connect(device, SIGNAL(profileChanged(const KoColorProfile*)),
                       SLOT(slotUpdateCurrentNodeColorSpace()), Qt::UniqueConnection);
            q->connect(device, SIGNAL(colorSpaceChanged(const KoColorSpace*)),
                       SLOT(slotUpdateCurrentNodeColorSpace()), Qt::UniqueConnection);
        }

    } else {
        nodeColorSpace = KoColorSpaceRegistry::instance()->rgb8();
    }

    connectedNode = node;
    selectPaintingColorSpace();
}

void KisDisplayColorConverter::Private::selectPaintingColorSpace()
{
    KisConfig cfg;
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

void KisDisplayColorConverter::setDisplayFilter(KisDisplayFilterSP displayFilter)
{
    if (m_d->displayFilter && displayFilter) {
        KoColor color(m_d->intermediateFgColor);
        displayFilter->approximateInverseTransformation(color.data(), 1);
        color.convertTo(m_d->paintingColorSpace);
        m_d->parentCanvas->resourceManager()->setForegroundColor(color);
    }

    m_d->displayFilter = displayFilter;
    m_d->intermediateColorSpace = 0;

    if (m_d->displayFilter) {
        m_d->intermediateColorSpace =
            KoColorSpaceRegistry::instance()->
            colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);

        KIS_ASSERT_RECOVER(m_d->displayFilter) {
            m_d->intermediateColorSpace = m_d->monitorColorSpace;
        }
    }


    { // sanity check
        KisConfig cfg;
        //KIS_ASSERT_RECOVER_NOOP(cfg.useOcio() == (bool) m_d->displayFilter);
    }

    m_d->selectPaintingColorSpace();
}


KoColorConversionTransformation::Intent
KisDisplayColorConverter::renderingIntent()
{
    KisConfig cfg;
    return (KoColorConversionTransformation::Intent)cfg.renderIntent();
}

KoColorConversionTransformation::ConversionFlags
KisDisplayColorConverter::conversionFlags()
{
    KoColorConversionTransformation::ConversionFlags conversionFlags =
        KoColorConversionTransformation::HighQuality;

    KisConfig cfg;

    if (cfg.useBlackPointCompensation()) conversionFlags |= KoColorConversionTransformation::BlackpointCompensation;
    if (!cfg.allowLCMSOptimization()) conversionFlags |= KoColorConversionTransformation::NoOptimization;

    return conversionFlags;
}

QColor KisDisplayColorConverter::toQColor(const KoColor &srcColor)
{
    KoColor c(srcColor);
    c.convertTo(m_d->paintingColorSpace);

    if (!m_d->useOcio()) {
        QByteArray pixel(m_d->monitorColorSpace->pixelSize(), 0);
        c.colorSpace()->convertPixelsTo(c.data(), (quint8*)pixel.data(),
                                        m_d->monitorColorSpace, 1,
                                        m_d->renderingIntent, m_d->conversionFlags);


        // we expect the display profile is rgb8, which is BGRA here
        KIS_ASSERT_RECOVER(m_d->monitorColorSpace->pixelSize() == 4) { return Qt::red; };

        const quint8 *p = (const quint8 *)pixel.constData();
        return QColor(p[2], p[1], p[0], p[3]);
    } else {
        int numChannels = m_d->paintingColorSpace->channelCount();
        QVector<float> normalizedChannels(numChannels);
        m_d->paintingColorSpace->normalisedChannelsValue(c.data(), normalizedChannels);
        m_d->displayFilter->filter((quint8*)normalizedChannels.data(), 1);

        const float *p = (const float *)normalizedChannels.constData();
        return QColor(KoColorSpaceMaths<float, quint8>::scaleToA(p[0]),
                      KoColorSpaceMaths<float, quint8>::scaleToA(p[1]),
                      KoColorSpaceMaths<float, quint8>::scaleToA(p[2]),
                      KoColorSpaceMaths<float, quint8>::scaleToA(p[3]));
    }
}

QImage KisDisplayColorConverter::toQImage(KisPaintDeviceSP srcDevice)
{
    KisPaintDeviceSP device = srcDevice;
    if (!(*device->colorSpace() == *m_d->paintingColorSpace)) {
        device = new KisPaintDevice(*srcDevice);

        KUndo2Command *cmd = device->convertTo(m_d->paintingColorSpace);
        delete cmd;
    }

    if (!m_d->useOcio()) {
        return device->convertToQImage(m_d->monitorProfile, m_d->renderingIntent, m_d->conversionFlags);
    } else {
        QRect bounds = device->exactBounds();
        if (bounds.isEmpty()) return QImage();

        QImage image(bounds.size(), QImage::Format_ARGB32);

        KisSequentialConstIterator it(device, bounds);
        quint8 *dstPtr = image.bits();

        int numChannels = m_d->paintingColorSpace->channelCount();
        QVector<float> normalizedChannels(numChannels);

        do {
            m_d->paintingColorSpace->normalisedChannelsValue(it.rawDataConst(), normalizedChannels);
            m_d->displayFilter->filter((quint8*)normalizedChannels.data(), 1);

            const float *p = normalizedChannels.constData();
            dstPtr[0] = KoColorSpaceMaths<float, quint8>::scaleToA(p[2]);
            dstPtr[1] = KoColorSpaceMaths<float, quint8>::scaleToA(p[1]);
            dstPtr[2] = KoColorSpaceMaths<float, quint8>::scaleToA(p[0]);
            dstPtr[3] = KoColorSpaceMaths<float, quint8>::scaleToA(p[3]);

            dstPtr += 4;
        } while (it.nextPixel());

        return image;
    }

    return QImage();
}

KoColor
KisDisplayColorConverter::Private::approximateFromQColor(const QColor &qcolor)
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
    return m_d->approximateFromQColor(qcolor);
}

void KisDisplayColorConverter::getHslF(const KoColor &srcColor, qreal *h, qreal *s, qreal *l, qreal *a)
{
    // we are going through sRGB here!
    QColor color = m_d->approximateToQColor(srcColor);
    color.getHslF(h, s, l, a);
}

#include "moc_kis_display_color_converter.cpp"
