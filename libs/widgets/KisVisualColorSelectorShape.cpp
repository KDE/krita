/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisVisualColorSelectorShape.h"

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QVector>
#include <QVector4D>
#include <QtMath>

#include "KoColorConversions.h"
#include "KoColorSpace.h"
#include "KoColorDisplayRendererInterface.h"
#include "KoChannelInfo.h"
#include <KoColorModelStandardIds.h>

#include "kis_debug.h"

struct KisVisualColorSelectorShape::Private
{
    QImage gradient;
    QImage alphaMask;
    QImage staticBackground;
    bool imagesNeedUpdate { true };
    bool alphaNeedsUpdate { true };
    bool acceptTabletEvents { false };
    QPointF currentCoordinates; // somewhat redundant?
    QPointF dragStart;
    QVector4D currentChannelValues;
    Dimensions dimension;
    int channel1;
    int channel2;
    quint32 channelMask;
};

KisVisualColorSelectorShape::KisVisualColorSelectorShape(KisVisualColorSelector *parent,
                                                         KisVisualColorSelectorShape::Dimensions dimension,
                                                         int channel1,
                                                         int channel2): QWidget(parent), m_d(new Private)
{
    m_d->dimension = dimension;
    int maxchannel = parent->selectorModel()->colorSpace()->colorChannelCount()-1;
    m_d->channel1 = qBound(0, channel1, maxchannel);
    m_d->channel2 = qBound(0, channel2, maxchannel);
    m_d->channelMask = 1 << channel1;
    if (dimension == Dimensions::twodimensional) {
        m_d->channelMask |= 1 << channel2;
    }
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

KisVisualColorSelectorShape::~KisVisualColorSelectorShape()
{
}

QPointF KisVisualColorSelectorShape::getCursorPosition() const {
    return m_d->currentCoordinates;
}

void KisVisualColorSelectorShape::setCursorPosition(QPointF position, bool signal)
{
    QPointF newPos(qBound(0.0, position.x(), 1.0), qBound(0.0, position.y(), 1.0));
    if (newPos != m_d->currentCoordinates)
    {
        m_d->currentCoordinates = newPos;
        // for internal consistency, because we have a bit of redundancy here
        m_d->currentChannelValues[m_d->channel1] = newPos.x();
        if (m_d->dimension == Dimensions::twodimensional){
            m_d->currentChannelValues[m_d->channel2] = newPos.y();
        }
        update();
        if (signal){
            Q_EMIT sigCursorMoved(newPos);
        }
    }
}

void KisVisualColorSelectorShape::setChannelValues(QVector4D channelValues, quint32 channelFlags)
{
    //qDebug() << this  << "setChannelValues";
    m_d->currentChannelValues = channelValues;
    bool setCursor = channelFlags & m_d->channelMask;
    if (setCursor) {
        m_d->currentCoordinates = QPointF(qBound(0.f, channelValues[m_d->channel1], 1.f),
                                          qBound(0.f, channelValues[m_d->channel2], 1.f));
    }
    else {
        // for internal consistency, because we have a bit of redundancy here
        m_d->currentChannelValues[m_d->channel1] = m_d->currentCoordinates.x();
        if (m_d->dimension == Dimensions::twodimensional){
            m_d->currentChannelValues[m_d->channel2] = m_d->currentCoordinates.y();
        }
    }
    m_d->imagesNeedUpdate = m_d->imagesNeedUpdate || channelFlags & ~m_d->channelMask;
    update();
}

void KisVisualColorSelectorShape::setAcceptTabletEvents(bool on)
{
    m_d->acceptTabletEvents = on;
}

bool KisVisualColorSelectorShape::isHueControl() const
{
    return selectorModel()->isHSXModel()
            && getDimensions() == KisVisualColorSelectorShape::onedimensional
            && m_d->channel1 == 0;
}

bool KisVisualColorSelectorShape::supportsGamutMask() const
{
    return false;
}

void KisVisualColorSelectorShape::forceImageUpdate()
{
    //qDebug() << this  << "forceImageUpdate";
    m_d->alphaNeedsUpdate = true;
    m_d->imagesNeedUpdate = true;
}

void KisVisualColorSelectorShape::updateGamutMask()
{
    // Nothing to do if gamut masks not supported
}

QColor KisVisualColorSelectorShape::getColorFromConverter(KoColor c)
{
    const KoColorDisplayRendererInterface *renderer = colorSelector()->displayRenderer();
    return renderer->toQColor(c, colorSelector()->proofColors());
}

KisVisualColorSelector *KisVisualColorSelectorShape::colorSelector() const
{
    KisVisualColorSelector* selectorWidget = qobject_cast<KisVisualColorSelector*>(parent());
    KIS_ASSERT(selectorWidget);
    return selectorWidget;
}

KisVisualColorModel *KisVisualColorSelectorShape::selectorModel() const
{
    KisVisualColorSelector* selectorWidget = qobject_cast<KisVisualColorSelector*>(parent());
    KIS_ASSERT(selectorWidget);
    return selectorWidget->selectorModel().data();
}

const QImage& KisVisualColorSelectorShape::getImageMap()
{
    //qDebug() << this  << ">>>>>>>>> getImageMap()" << m_d->imagesNeedUpdate;

    if (m_d->imagesNeedUpdate) {
        // NOTE: pure static backgrounds are currently somewhat implicitly handled,
        // it would be nicer to avoid re-checking and overwriting m_d->gradient.
        // But QImage's implicit data sharing allows all this mindless by-value stuff...
        m_d->gradient = compositeBackground();
        m_d->imagesNeedUpdate = false;
    }
    return m_d->gradient;
}

QImage KisVisualColorSelectorShape::convertImageMap(const quint8 *rawColor, quint32 bufferSize, QSize imgSize) const
{
    const KoColorSpace *colorSpace = selectorModel()->colorSpace();
    Q_ASSERT(bufferSize == imgSize.width() * imgSize.height() * colorSpace->pixelSize());
    const KoColorDisplayRendererInterface *renderer = colorSelector()->displayRenderer();

    // Convert the buffer to a qimage
    QImage image = renderer->toQImage(colorSpace, rawColor, imgSize, colorSelector()->proofColors());

    // safeguard:
    if (image.isNull())
    {
        image = QImage(width(), height(), QImage::Format_ARGB32);
        image.fill(Qt::black);
    }

    return image;
}

QImage KisVisualColorSelectorShape::renderBackground(const QVector4D &channelValues, const QImage &alpha) const
{
    const KisVisualColorModel *selector = selectorModel();
    Q_ASSERT(selector);

    // Hi-DPI aware rendering requires that we determine the device pixel dimension;
    // actual widget size in device pixels is not accessible unfortunately, it might be 1px smaller...
    const qreal deviceDivider = 1.0 / devicePixelRatioF();
    const int deviceWidth = qCeil(width() * devicePixelRatioF());
    const int deviceHeight = qCeil(height() * devicePixelRatioF());
    quint32 imageSize = deviceWidth * deviceHeight * selector->colorSpace()->pixelSize();
    QScopedArrayPointer<quint8> raw(new quint8[imageSize] {});
    quint8 *dataPtr = raw.data();
    QVector4D coordinates = channelValues;
    const qsizetype pixelSize = selector->colorSpace()->pixelSize();

    bool checkAlpha = !alpha.isNull() && alpha.valid(deviceWidth - 1, deviceHeight - 1);
    KIS_SAFE_ASSERT_RECOVER(!checkAlpha || alpha.format() == QImage::Format_Alpha8) {
        checkAlpha = false;
    }

    KoColor filler(Qt::white, selector->colorSpace());
    for (int y = 0; y < deviceHeight; y++) {
        const uchar *alphaLine = checkAlpha ? alpha.scanLine(y) : 0;
        for (int x=0; x < deviceWidth; x++) {
            if (!checkAlpha || alphaLine[x]) {
                QPointF newcoordinate = convertWidgetCoordinateToShapeCoordinate(QPointF(x, y) * deviceDivider);
                coordinates[m_d->channel1] = newcoordinate.x();
                if (m_d->dimension == Dimensions::twodimensional) {
                    coordinates[m_d->channel2] = newcoordinate.y();
                }
                KoColor c = selector->convertChannelValuesToKoColor(coordinates);
                memcpy(dataPtr, c.data(), pixelSize);
            }
            else {
                // need to write a color with non-zero alpha, otherwise the display converter
                // will for some arcane reason crop the final QImage and screw rendering
                memcpy(dataPtr, filler.data(), pixelSize);
            }
            dataPtr += pixelSize;
        }
    }
    QImage image = convertImageMap(raw.data(), imageSize, QSize(deviceWidth, deviceHeight));
    image.setDevicePixelRatio(devicePixelRatioF());

    if (!alpha.isNull()) {
        QPainter painter(&image);
        // transfer alphaMask to Alpha channel
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.drawImage(0, 0, alpha);
    }

    return image;
}

QImage KisVisualColorSelectorShape::compositeBackground() const
{
    // Shapes are expect to return a valid alpha mask or a valid
    // static alpha mask. If they provide both, the rendered backgrounds
    // get composited.
    if (m_d->alphaNeedsUpdate) {
        QImage staticAlpha = renderStaticAlphaMask();
        if (!staticAlpha.isNull()) {
            QVector4D neutralValues(1, 1, 1, 1);
            switch (selectorModel()->colorModel()) {
            case KisVisualColorModel::HSL:
            case KisVisualColorModel::HSI:
            case KisVisualColorModel::HSY:
                neutralValues.setZ(0.5f);
            default:
                break;
            }

            m_d->staticBackground = renderBackground(neutralValues, staticAlpha);
        }
        m_d->alphaMask = renderAlphaMask();
        m_d->alphaNeedsUpdate = false;
    }
    if (m_d->alphaMask.isNull()) {
        return m_d->staticBackground;
    }

    QImage bgImage = renderBackground(m_d->currentChannelValues, m_d->alphaMask);
    if (!m_d->staticBackground.isNull()) {
        QPainter painter(&bgImage);
        // composite static and dynamic background parts
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
        painter.drawImage(0, 0, m_d->staticBackground);
    }
    return bgImage;
}

QImage KisVisualColorSelectorShape::renderAlphaMask() const
{
    return QImage();
}

QImage KisVisualColorSelectorShape::renderStaticAlphaMask() const
{
    return QImage();
}

QPointF KisVisualColorSelectorShape::mousePositionToShapeCoordinate(const QPointF &pos, const QPointF &dragStart) const
{
    Q_UNUSED(dragStart);
    return convertWidgetCoordinateToShapeCoordinate(pos);
}

void KisVisualColorSelectorShape::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_d->dragStart = e->localPos();
        Q_EMIT colorSelector()->sigInteraction(true);
        QPointF coordinates = mousePositionToShapeCoordinate(e->localPos(), m_d->dragStart);
        setCursorPosition(coordinates, true);
    }
    else {
        e->ignore();
    }
}

void KisVisualColorSelectorShape::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton) {
        QPointF coordinates = mousePositionToShapeCoordinate(e->localPos(), m_d->dragStart);
        setCursorPosition(coordinates, true);
    } else {
        e->ignore();
    }
}

void KisVisualColorSelectorShape::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        Q_EMIT colorSelector()->sigInteraction(false);
    } else {
        e->ignore();
    }
}

void KisVisualColorSelectorShape::tabletEvent(QTabletEvent* event)
{
    // only accept tablet events that are associated to "left" button
    // NOTE: QTabletEvent does not have a windowPos() equivalent, but we don't need it
    if (m_d->acceptTabletEvents &&
        (event->button() == Qt::LeftButton || (event->buttons() & Qt::LeftButton)))
    {
        event->accept();
        switch (event->type()) {
        case  QEvent::TabletPress: {
            QMouseEvent mouseEvent(QEvent::MouseButtonPress, event->posF(), event->posF(),
                                   event->globalPosF(), event->button(), event->buttons(),
                                   event->modifiers(), Qt::MouseEventSynthesizedByApplication);
            mousePressEvent(&mouseEvent);
            break;
        }
        case QEvent::TabletMove: {
            QMouseEvent mouseEvent(QEvent::MouseMove, event->posF(), event->posF(),
                                   event->globalPosF(), event->button(), event->buttons(),
                                   event->modifiers(), Qt::MouseEventSynthesizedByApplication);
            mouseMoveEvent(&mouseEvent);
            break;
        }
        case QEvent::TabletRelease: {
            QMouseEvent mouseEvent(QEvent::MouseButtonRelease, event->posF(), event->posF(),
                                   event->globalPosF(), event->button(), event->buttons(),
                                   event->modifiers(), Qt::MouseEventSynthesizedByApplication);
            mouseReleaseEvent(&mouseEvent);
            break;
        }
        default:
            event->ignore();
        }
    }
}

void KisVisualColorSelectorShape::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    const QImage &fullSelector = getImageMap();
    if (!fullSelector.isNull()) {
        painter.drawImage(0, 0, fullSelector);
    }

    drawGamutMask(painter);

    if (isEnabled()) {
        painter.setRenderHint(QPainter::Antialiasing);
        drawCursor(painter);
    }
}

void KisVisualColorSelectorShape::resizeEvent(QResizeEvent *)
{
    // just reuse the content of notifyDevicePixelRationChanged()
    notifyDevicePixelRationChanged();
}

void KisVisualColorSelectorShape::notifyDevicePixelRationChanged()
{
    forceImageUpdate();
    updateGamutMask();
    setMask(getMaskMap());
}

void KisVisualColorSelectorShape::drawGamutMask(QPainter &painter)
{
    // Nothing to do if gamut masks not supported
    Q_UNUSED(painter);
}

KisVisualColorSelectorShape::Dimensions KisVisualColorSelectorShape::getDimensions() const
{
    return m_d->dimension;
}

KoColor KisVisualColorSelectorShape::getCurrentColor()
{
    const KisVisualColorModel *selector = selectorModel();
    if (selector)
    {
        return selector->convertChannelValuesToKoColor(m_d->currentChannelValues);
    }
    return KoColor();
}

int KisVisualColorSelectorShape::channel(int dimension) const
{
    if (dimension == 0) {
        return m_d->channel1;
    }
    if (dimension == 1 && getDimensions() == twodimensional) {
        return m_d->channel2;
    }
    return -1;
}

quint32 KisVisualColorSelectorShape::channelMask() const
{
    return m_d->channelMask;
}
