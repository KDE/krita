/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
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
#include "KisVisualColorSelectorShape.h"

#include <QColor>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QVector>
#include <QVector4D>
#include <QVBoxLayout>
#include <QList>
#include <QPolygon>
#include <QtMath>

#include <KSharedConfig>
#include <KConfigGroup>

#include "KoColorConversions.h"
#include "KoColorDisplayRendererInterface.h"
#include "KoChannelInfo.h"
#include <KoColorModelStandardIds.h>
#include <QPointer>

#include "kis_signal_compressor.h"
#include "kis_debug.h"

struct KisVisualColorSelectorShape::Private
{
    QImage gradient;
    QImage fullSelector;
    bool imagesNeedUpdate {true};
    QPointF currentCoordinates; // somewhat redundant?
    QVector4D currentChannelValues;
    Dimensions dimension;
    const KoColorSpace *colorSpace;
    int channel1;
    int channel2;
    bool mousePressActive = false;
    const KoColorDisplayRendererInterface *displayRenderer = 0;
};

KisVisualColorSelectorShape::KisVisualColorSelectorShape(QWidget *parent,
                                                         KisVisualColorSelectorShape::Dimensions dimension,
                                                         const KoColorSpace *cs,
                                                         int channel1,
                                                         int channel2,
                                                         const KoColorDisplayRendererInterface *displayRenderer): QWidget(parent), m_d(new Private)
{
    m_d->dimension = dimension;
    m_d->colorSpace = cs;
    int maxchannel = m_d->colorSpace->colorChannelCount()-1;
    m_d->channel1 = qBound(0, channel1, maxchannel);
    m_d->channel2 = qBound(0, channel2, maxchannel);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setDisplayRenderer(displayRenderer);
}

KisVisualColorSelectorShape::~KisVisualColorSelectorShape()
{
}

QPointF KisVisualColorSelectorShape::getCursorPosition() {
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
            emit sigCursorMoved(newPos);
        }
    }
}

void KisVisualColorSelectorShape::setChannelValues(QVector4D channelValues, bool setCursor)
{
    //qDebug() << this  << "setChannelValues";
    m_d->currentChannelValues = channelValues;
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
    m_d->imagesNeedUpdate = true;
    update();
}

void KisVisualColorSelectorShape::setDisplayRenderer (const KoColorDisplayRendererInterface *displayRenderer)
{
    if (displayRenderer) {
        if (m_d->displayRenderer) {
            m_d->displayRenderer->disconnect(this);
        }
        m_d->displayRenderer = displayRenderer;
    } else {
        m_d->displayRenderer = KoDumbColorDisplayRenderer::instance();
    }
}

void KisVisualColorSelectorShape::forceImageUpdate()
{
    //qDebug() << this  << "forceImageUpdate";
    m_d->imagesNeedUpdate = true;
}

QColor KisVisualColorSelectorShape::getColorFromConverter(KoColor c){
    QColor col;
    KoColor color = c;
    if (m_d->displayRenderer) {
        color.convertTo(m_d->displayRenderer->getPaintingColorSpace());
        col = m_d->displayRenderer->toQColor(c);
    } else {
        col = c.toQColor();
    }
    return col;
}

// currently unused?
void KisVisualColorSelectorShape::slotSetActiveChannels(int channel1, int channel2)
{
    //qDebug() << this  << "slotSetActiveChannels";
    int maxchannel = m_d->colorSpace->colorChannelCount()-1;
    m_d->channel1 = qBound(0, channel1, maxchannel);
    m_d->channel2 = qBound(0, channel2, maxchannel);
    m_d->imagesNeedUpdate = true;
    update();
}

bool KisVisualColorSelectorShape::imagesNeedUpdate() const {
    return m_d->imagesNeedUpdate;
}

QImage KisVisualColorSelectorShape::getImageMap()
{
    //qDebug() << this  << ">>>>>>>>> getImageMap()" << m_d->imagesNeedUpdate;

    if (m_d->imagesNeedUpdate) {
        // Fill a buffer with the right kocolors
        m_d->gradient = renderBackground(m_d->currentChannelValues, m_d->colorSpace->pixelSize());
        m_d->imagesNeedUpdate = false;
    }
    return m_d->gradient;
}

QImage KisVisualColorSelectorShape::convertImageMap(const quint8 *rawColor, quint32 size) const
{
    Q_ASSERT(size == width()*height()*m_d->colorSpace->pixelSize());
    QImage image;
    // Convert the buffer to a qimage
    if (m_d->displayRenderer) {
        image = m_d->displayRenderer->convertToQImage(m_d->colorSpace, rawColor, width(), height());
    }
    else {
        image = m_d->colorSpace->convertToQImage(rawColor, width(), height(), 0, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
    }
    // safeguard:
    if (image.isNull())
    {
        image = QImage(width(), height(), QImage::Format_ARGB32);
        image.fill(Qt::black);
    }

    return image;
}

QImage KisVisualColorSelectorShape::renderBackground(const QVector4D &channelValues, quint32 pixelSize) const
{
    const KisVisualColorSelector *selector = qobject_cast<KisVisualColorSelector*>(parent());
    Q_ASSERT(selector);

    quint32 imageSize = width() * height() * m_d->colorSpace->pixelSize();
    QScopedArrayPointer<quint8> raw(new quint8[imageSize] {});
    quint8 *dataPtr = raw.data();
    QVector4D coordinates = channelValues;
    for (int y = 0; y < height(); y++) {
        for (int x=0; x < width(); x++) {
            QPointF newcoordinate = convertWidgetCoordinateToShapeCoordinate(QPoint(x, y));
            coordinates[m_d->channel1] = newcoordinate.x();
            if (m_d->dimension == Dimensions::twodimensional){
                coordinates[m_d->channel2] = newcoordinate.y();
            }
            KoColor c = selector->convertShapeCoordsToKoColor(coordinates);
            memcpy(dataPtr, c.data(), pixelSize);
            dataPtr += pixelSize;
        }
    }
    return convertImageMap(raw.data(), imageSize);
}

void KisVisualColorSelectorShape::mousePressEvent(QMouseEvent *e)
{
    if (e->button()==Qt::LeftButton) {
        m_d->mousePressActive = true;
        QPointF coordinates = convertWidgetCoordinateToShapeCoordinate(e->pos());
        setCursorPosition(coordinates, true);
    }
}

void KisVisualColorSelectorShape::mouseMoveEvent(QMouseEvent *e)
{
    if (m_d->mousePressActive==true) {
        QPointF coordinates = convertWidgetCoordinateToShapeCoordinate(e->pos());
        setCursorPosition(coordinates, true);
    } else {
        e->ignore();
    }
}

void KisVisualColorSelectorShape::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button()==Qt::LeftButton) {
        m_d->mousePressActive = false;
    }
}
void KisVisualColorSelectorShape::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    drawCursor();
    painter.drawImage(0,0,m_d->fullSelector);
}

void KisVisualColorSelectorShape::resizeEvent(QResizeEvent *)
{
    forceImageUpdate();
    setMask(getMaskMap());
}

KisVisualColorSelectorShape::Dimensions KisVisualColorSelectorShape::getDimensions() const
{
    return m_d->dimension;
}

void KisVisualColorSelectorShape::setFullImage(QImage full)
{
    m_d->fullSelector = full;
}
KoColor KisVisualColorSelectorShape::getCurrentColor()
{
    const KisVisualColorSelector *selector = qobject_cast<KisVisualColorSelector*>(parent());
    if (selector)
    {
        return selector->convertShapeCoordsToKoColor(m_d->currentChannelValues);
    }
    return KoColor(m_d->colorSpace);
}

QVector <int> KisVisualColorSelectorShape::getChannels() const
{
    QVector <int> channels(2);
    channels[0] = m_d->channel1;
    channels[1] = m_d->channel2;
    return channels;
}
