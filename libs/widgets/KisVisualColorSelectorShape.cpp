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
    ColorModel model;
    const KoColorSpace *colorSpace;
    KoColor currentColor;// TODO: relocate to parent
    int channel1;
    int channel2;
    KisSignalCompressor *updateTimer {0}; // To be removed
    bool mousePressActive = false;
    const KoColorDisplayRendererInterface *displayRenderer = 0;
    qreal hue = 0.0; // To be removed
    qreal sat = 0.0; // To be removed
    qreal tone = 0.0; // To be removed
    bool usesOCIO = false;
    bool isRGBA = false;
    bool is8Bit = false;

};

KisVisualColorSelectorShape::KisVisualColorSelectorShape(QWidget *parent,
                                                         KisVisualColorSelectorShape::Dimensions dimension,
                                                         KisVisualColorSelectorShape::ColorModel model,
                                                         const KoColorSpace *cs,
                                                         int channel1,
                                                         int channel2,
                                                         const KoColorDisplayRendererInterface *displayRenderer): QWidget(parent), m_d(new Private)
{
    m_d->dimension = dimension;
    m_d->model = model;
    m_d->colorSpace = cs;

    // TODO: The following is done because the IDs are actually strings. Ideally, in the future, we
    // refactor everything so that the IDs are actually proper enums or something faster.
    if (m_d->displayRenderer
            && (m_d->colorSpace->colorDepthId() == Float16BitsColorDepthID
                || m_d->colorSpace->colorDepthId() == Float32BitsColorDepthID
                || m_d->colorSpace->colorDepthId() == Float64BitsColorDepthID)
            && m_d->colorSpace->colorModelId() != LABAColorModelID
            && m_d->colorSpace->colorModelId() != CMYKAColorModelID) {
        m_d->usesOCIO = true;
    } else {
        m_d->usesOCIO = false;
    }
    if (m_d->colorSpace->colorModelId() == RGBAColorModelID) {
        m_d->isRGBA = true;
    } else {
        m_d->isRGBA = false;
    }
    if (m_d->colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        m_d->is8Bit = true;
    } else {
        m_d->is8Bit = false;
    }
    m_d->currentColor = KoColor();
    m_d->currentColor.setOpacity(1.0);
    m_d->currentColor.convertTo(cs);
    int maxchannel = m_d->colorSpace->colorChannelCount()-1;
    m_d->channel1 = qBound(0, channel1, maxchannel);
    m_d->channel2 = qBound(0, channel2, maxchannel);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // HACK: the updateTimer isn't connected to anything, we only check whether it's still active
    //       and running in order to determine whether we will emit a certain signal.
    //m_d->updateTimer = new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE, this);
    setDisplayRenderer(displayRenderer);
    show();

}

KisVisualColorSelectorShape::~KisVisualColorSelectorShape()
{
}

void KisVisualColorSelectorShape::updateCursor()
{
    QPointF point1 = convertKoColorToShapeCoordinate(m_d->currentColor);
    if (point1 != m_d->currentCoordinates) {
        m_d->currentCoordinates = point1;
    }
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
        m_d->currentCoordinates = QPointF(channelValues[m_d->channel1], channelValues[m_d->channel2]);
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

void KisVisualColorSelectorShape::setColor(KoColor c)
{
    //qDebug() << this  << "KisVisualColorSelectorShape::setColor";
    if (c.colorSpace() != m_d->colorSpace) {
        c.convertTo(m_d->colorSpace);
    }
    m_d->currentColor = c;
    updateCursor();

    m_d->imagesNeedUpdate = true;
    update();
}

void KisVisualColorSelectorShape::setColorFromSibling(KoColor c)
{
    //qDebug() << this  << "setColorFromSibling";
    if (c.colorSpace() != m_d->colorSpace) {
        c.convertTo(m_d->colorSpace);
    }
    m_d->currentColor = c;

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
    connect(m_d->displayRenderer, SIGNAL(displayConfigurationChanged()),
            SLOT(updateFromChangedDisplayRenderer()), Qt::UniqueConnection);

}

void KisVisualColorSelectorShape::updateFromChangedDisplayRenderer()
{
    //qDebug() << this  << "updateFromChangedDisplayRenderer();";
    m_d->imagesNeedUpdate = true;
    //updateCursor();
    //m_d->currentColor = convertShapeCoordinateToKoColor(getCursorPosition());
    update();
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
    const KisVisualColorSelector *selector = qobject_cast<KisVisualColorSelector*>(parent());

    if (m_d->imagesNeedUpdate == true) {
        // Fill a buffer with the right kocolors
        quint8 *data = new quint8[width() * height() * m_d->colorSpace->pixelSize()];
        quint8 *dataPtr = data;
        QVector4D coordinates = m_d->currentChannelValues;
        for (int y = 0; y < height(); y++) {
            for (int x=0; x < width(); x++) {
                QPointF newcoordinate = convertWidgetCoordinateToShapeCoordinate(QPoint(x, y));
                coordinates[m_d->channel1] = newcoordinate.x();
                if (m_d->dimension == Dimensions::twodimensional){
                    coordinates[m_d->channel2] = newcoordinate.y();
                }
                KoColor c = selector->convertShapeCoordsToKoColor(coordinates);
                memcpy(dataPtr, c.data(), m_d->colorSpace->pixelSize());
                dataPtr += m_d->colorSpace->pixelSize();
            }
        }
        // Convert the buffer to a qimage
        if (m_d->displayRenderer) {
            m_d->gradient = m_d->displayRenderer->convertToQImage(m_d->colorSpace, data, width(), height());
        }
        else {
            m_d->gradient = m_d->colorSpace->convertToQImage(data, width(), height(), 0, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
        }
        delete[] data;

        m_d->imagesNeedUpdate = false;
        // safeguard:
        if (m_d->gradient.isNull())
        {
            m_d->gradient = QImage(width(), height(), QImage::Format_ARGB32);
            m_d->gradient.fill(Qt::black);
        }
    }
    return m_d->gradient;
}

KoColor KisVisualColorSelectorShape::convertShapeCoordinateToKoColor(QPointF coordinates, bool cursor)
{
    //qDebug() << this  << ">>>>>>>>> convertShapeCoordinateToKoColor()" << coordinates;

    KoColor c = m_d->currentColor;
    QVector <float> channelValues (c.colorSpace()->channelCount());
    channelValues.fill(1.0);
    c.colorSpace()->normalisedChannelsValue(c.data(), channelValues);
    QVector <float> channelValuesDisplay = channelValues;
    QVector <qreal> maxvalue(c.colorSpace()->channelCount());
    maxvalue.fill(1.0);

    if (m_d->usesOCIO == true) {

        for (int ch = 0; ch < maxvalue.size(); ch++) {
            KoChannelInfo *channel = m_d->colorSpace->channels()[ch];
            maxvalue[ch] = m_d->displayRenderer->maxVisibleFloatValue(channel);
            channelValues[ch] = channelValues[ch]/(maxvalue[ch]);
            channelValuesDisplay[KoChannelInfo::displayPositionToChannelIndex(ch, m_d->colorSpace->channels())] = channelValues[ch];
        }
    }
    else {
        for (int i =0; i < channelValues.size();i++) {
            channelValuesDisplay[KoChannelInfo::displayPositionToChannelIndex(i, m_d->colorSpace->channels())] = qBound((float)0.0,channelValues[i], (float)1.0);
        }
    }

    qreal huedivider = 1.0;
    qreal huedivider2 = 1.0;

    if (m_d->channel1 == 0) {
        huedivider = 360.0;
    }

    if (m_d->channel2 == 0) {
        huedivider2 = 360.0;
    }

    if (m_d->model != ColorModel::Channel && m_d->isRGBA == true) {

        if (m_d->model == ColorModel::HSV) {
            /*
             * RGBToHSV has a undefined hue possibility. This means that hue will be -1.
             * This can be annoying for dealing with a selector, but I understand it is being
             * used for the KoColorSelector... For now implement a qMax here.
             */
            QVector <float> inbetween(3);
            RGBToHSV(channelValuesDisplay[0],channelValuesDisplay[1], channelValuesDisplay[2], &inbetween[0], &inbetween[1], &inbetween[2]);
            inbetween = convertvectorqrealTofloat(getHSX(convertvectorfloatToqreal(inbetween)));
            inbetween[m_d->channel1] = coordinates.x()*huedivider;
            if (m_d->dimension == Dimensions::twodimensional) {
                inbetween[m_d->channel2] = coordinates.y()*huedivider2;
            }
            if (cursor) {
                setHSX(convertvectorfloatToqreal(inbetween));
                Q_EMIT sigHSXchange();
            }
            HSVToRGB(qMax(inbetween[0],(float)0.0), inbetween[1], inbetween[2], &channelValuesDisplay[0], &channelValuesDisplay[1], &channelValuesDisplay[2]);
        }
        else if (m_d->model == ColorModel::HSL) {
            /*
             * HSLToRGB can give negative values on the grey. I fixed the fromNormalisedChannel function to clamp,
             * but you might want to manually clamp for floating point values.
             */
            QVector <float> inbetween(3);
            RGBToHSL(channelValuesDisplay[0],channelValuesDisplay[1], channelValuesDisplay[2], &inbetween[0], &inbetween[1], &inbetween[2]);
            inbetween = convertvectorqrealTofloat(getHSX(convertvectorfloatToqreal(inbetween)));
            inbetween[m_d->channel1] = fmod(coordinates.x()*huedivider, 360.0);
            if (m_d->dimension == Dimensions::twodimensional) {
                inbetween[m_d->channel2] = coordinates.y()*huedivider2;
            }
            if (cursor) {
                setHSX(convertvectorfloatToqreal(inbetween));
                Q_EMIT sigHSXchange();
            }
            HSLToRGB(qMax(inbetween[0], (float)0.0), inbetween[1], inbetween[2], &channelValuesDisplay[0], &channelValuesDisplay[1], &channelValuesDisplay[2]);
        }
        else if (m_d->model == ColorModel::HSI) {
            /*
             * HSI is a modified HSY function.
             */
            QVector <qreal> chan2 = convertvectorfloatToqreal(channelValuesDisplay);
            QVector <qreal> inbetween(3);
            RGBToHSI(chan2[0],chan2[1], chan2[2], &inbetween[0], &inbetween[1], &inbetween[2]);
            inbetween = getHSX(inbetween);
            inbetween[m_d->channel1] = coordinates.x();
            if (m_d->dimension == Dimensions::twodimensional) {
                inbetween[m_d->channel2] = coordinates.y();
            }
            if (cursor) {
                setHSX(inbetween);
                Q_EMIT sigHSXchange();
            }
            HSIToRGB(inbetween[0], inbetween[1], inbetween[2],&chan2[0],&chan2[1], &chan2[2]);
            channelValuesDisplay = convertvectorqrealTofloat(chan2);
        }
        else /*if (m_d->model == ColorModel::HSY)*/ {
            /*
             * HSY is pretty slow to render due being a pretty over-the-top function.
             * Might be worth investigating whether HCY can be used instead, but I have had
             * some weird results with that.
             */
            QVector <qreal> luma= m_d->colorSpace->lumaCoefficients();
            QVector <qreal> chan2 = convertvectorfloatToqreal(channelValuesDisplay);
            QVector <qreal> inbetween(3);
            RGBToHSY(chan2[0],chan2[1], chan2[2], &inbetween[0], &inbetween[1], &inbetween[2],
                    luma[0], luma[1], luma[2]);
            inbetween = getHSX(inbetween);
            inbetween[m_d->channel1] = coordinates.x();
            if (m_d->dimension == Dimensions::twodimensional) {
                inbetween[m_d->channel2] = coordinates.y();
            }
            if (cursor) {
                setHSX(inbetween);
                Q_EMIT sigHSXchange();
            }
            HSYToRGB(inbetween[0], inbetween[1], inbetween[2],&chan2[0],&chan2[1], &chan2[2],
                    luma[0], luma[1], luma[2]);
            channelValuesDisplay = convertvectorqrealTofloat(chan2);
        }

    }
    else {
        channelValuesDisplay[m_d->channel1] = coordinates.x();
        if (m_d->dimension == Dimensions::twodimensional) {
            channelValuesDisplay[m_d->channel2] = coordinates.y();
        }
    }

    for (int i=0; i<channelValues.size();i++) {
        channelValues[i] = channelValuesDisplay[KoChannelInfo::displayPositionToChannelIndex(i, m_d->colorSpace->channels())]*(maxvalue[i]);
    }

    c.colorSpace()->fromNormalisedChannelsValue(c.data(), channelValues);

    return c;
}

QPointF KisVisualColorSelectorShape::convertKoColorToShapeCoordinate(KoColor c)
{
    ////qDebug() << this  << ">>>>>>>>> convertKoColorToShapeCoordinate()";

    if (c.colorSpace() != m_d->colorSpace) {
        c.convertTo(m_d->colorSpace);
    }
    QVector <float> channelValues (m_d->currentColor.colorSpace()->channelCount());
    channelValues.fill(1.0);
    m_d->colorSpace->normalisedChannelsValue(c.data(), channelValues);
    QVector <float> channelValuesDisplay = channelValues;
    QVector <qreal> maxvalue(c.colorSpace()->channelCount());
    maxvalue.fill(1.0);
    if (m_d->usesOCIO == true) {
        for (int ch = 0; ch<maxvalue.size(); ch++) {
            KoChannelInfo *channel = m_d->colorSpace->channels()[ch];
            maxvalue[ch] = m_d->displayRenderer->maxVisibleFloatValue(channel);
            channelValues[ch] = channelValues[ch]/(maxvalue[ch]);
            channelValuesDisplay[KoChannelInfo::displayPositionToChannelIndex(ch, m_d->colorSpace->channels())] = channelValues[ch];
        }
    } else {
        for (int i =0; i<channelValues.size();i++) {
            channelValuesDisplay[KoChannelInfo::displayPositionToChannelIndex(i, m_d->colorSpace->channels())] = qBound((float)0.0,channelValues[i], (float)1.0);
        }
    }
    QPointF coordinates(0.0,0.0);
    qreal huedivider = 1.0;
    qreal huedivider2 = 1.0;
    if (m_d->channel1==0) {
        huedivider = 360.0;
    }
    if (m_d->channel2==0) {
        huedivider2 = 360.0;
    }
    if (m_d->model != ColorModel::Channel && m_d->isRGBA == true) {
        if (m_d->isRGBA == true) {
            if (m_d->model == ColorModel::HSV){
                QVector <float> inbetween(3);
                RGBToHSV(channelValuesDisplay[0],channelValuesDisplay[1], channelValuesDisplay[2], &inbetween[0], &inbetween[1], &inbetween[2]);
                inbetween = convertvectorqrealTofloat(getHSX(convertvectorfloatToqreal(inbetween)));
                coordinates.setX(inbetween[m_d->channel1]/huedivider);
                if (m_d->dimension == Dimensions::twodimensional) {
                    coordinates.setY(inbetween[m_d->channel2]/huedivider2);
                }
            } else if (m_d->model == ColorModel::HSL) {
                QVector <float> inbetween(3);
                RGBToHSL(channelValuesDisplay[0],channelValuesDisplay[1], channelValuesDisplay[2], &inbetween[0], &inbetween[1], &inbetween[2]);
                inbetween = convertvectorqrealTofloat(getHSX(convertvectorfloatToqreal(inbetween)));
                coordinates.setX(inbetween[m_d->channel1]/huedivider);
                if (m_d->dimension == Dimensions::twodimensional) {
                    coordinates.setY(inbetween[m_d->channel2]/huedivider2);
                }
            } else if (m_d->model == ColorModel::HSI) {
                QVector <qreal> chan2 = convertvectorfloatToqreal(channelValuesDisplay);
                QVector <qreal> inbetween(3);
                RGBToHSI(channelValuesDisplay[0],channelValuesDisplay[1], channelValuesDisplay[2], &inbetween[0], &inbetween[1], &inbetween[2]);
                inbetween = getHSX(inbetween);
                coordinates.setX(inbetween[m_d->channel1]);
                if (m_d->dimension == Dimensions::twodimensional) {
                    coordinates.setY(inbetween[m_d->channel2]);
                }
            } else if (m_d->model == ColorModel::HSY) {
                QVector <qreal> luma = m_d->colorSpace->lumaCoefficients();
                QVector <qreal> chan2 = convertvectorfloatToqreal(channelValuesDisplay);
                QVector <qreal> inbetween(3);
                RGBToHSY(channelValuesDisplay[0],channelValuesDisplay[1], channelValuesDisplay[2], &inbetween[0], &inbetween[1], &inbetween[2], luma[0], luma[1], luma[2]);
                inbetween = getHSX(inbetween);
                coordinates.setX(inbetween[m_d->channel1]);
                if (m_d->dimension == Dimensions::twodimensional) {
                    coordinates.setY(inbetween[m_d->channel2]);
                }
            }
        }
    } else {
        coordinates.setX(qBound((float)0.0, channelValuesDisplay[m_d->channel1], (float)1.0));
        if (m_d->dimension == Dimensions::twodimensional) {
            coordinates.setY(qBound((float)0.0, channelValuesDisplay[m_d->channel2], (float)1.0));
        }
    }
    return coordinates;
}

QVector<float> KisVisualColorSelectorShape::convertvectorqrealTofloat(QVector<qreal> real)
{
    QVector <float> vloat(real.size());
    for (int i=0; i<real.size(); i++) {
        vloat[i] = real[i];
    }
    return vloat;
}

QVector<qreal> KisVisualColorSelectorShape::convertvectorfloatToqreal(QVector <float> vloat)
{
    QVector <qreal> real(vloat.size());
    for (int i=0; i<vloat.size(); i++) {
        real[i] = vloat[i];
    }
    return real;
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

    //check if old and new colors differ.

    if (m_d->imagesNeedUpdate) {
        setMask(getMaskMap());
    }
    drawCursor();
    painter.drawImage(0,0,m_d->fullSelector);
}

KisVisualColorSelectorShape::Dimensions KisVisualColorSelectorShape::getDimensions() const
{
    return m_d->dimension;
}

KisVisualColorSelectorShape::ColorModel KisVisualColorSelectorShape::getColorModel()
{
    return m_d->model;
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

QVector <qreal> KisVisualColorSelectorShape::getHSX(QVector<qreal> hsx, bool wrangler)
{
    QVector <qreal> ihsx = hsx;
    if (!wrangler){
        //Ok, so this docker will not update luminosity if there's not at the least 3% more variation.
        //This is necessary for 8bit.
        if (m_d->is8Bit == true){
            if (hsx[2]>m_d->tone-0.03 && hsx[2]<m_d->tone+0.03) {
                ihsx[2] = m_d->tone;
            }
        } else {
            if (hsx[2]>m_d->tone-0.005 && hsx[2]<m_d->tone+0.005) {
                ihsx[2] = m_d->tone;
            }
        }
        if (m_d->model==HSV){
            if (hsx[2]<=0.0) {
                ihsx[1] = m_d->sat;
            }
        } else {
            if ((hsx[2]<=0.0 || hsx[2]>=1.0)) {
                ihsx[1] = m_d->sat;
            }
        }
        if ((hsx[1]<=0.0 || hsx[0]<0.0)){
            ihsx[0]=m_d->hue;
        }
    } else {
        ihsx[0]=m_d->hue;
        ihsx[1]=m_d->sat;
        ihsx[2]=m_d->tone;
    }
    return ihsx;
}

void KisVisualColorSelectorShape::setHSX(QVector<qreal> hsx, bool wrangler)
{
    if (wrangler){
        m_d->tone = hsx[2];
        m_d->sat = hsx[1];
        m_d->hue = hsx[0];
    } else {
        if (m_d->channel1==2 || m_d->channel2==2){
            m_d->tone=hsx[2];
        }
        if (m_d->model==HSV){
            if (hsx[2]>0.0) {
                m_d->sat = hsx[1];
            }
        } else {
            if ((hsx[2]>0.0 || hsx[2]<1.0)) {
                m_d->sat = hsx[1];
            }
        }
        if ((hsx[1]>0.0 && hsx[0]>=0.0)){
            m_d->hue = hsx[0];
        }
    }
}

QVector <int> KisVisualColorSelectorShape::getChannels() const
{
    QVector <int> channels(2);
    channels[0] = m_d->channel1;
    channels[1] = m_d->channel2;
    return channels;
}
