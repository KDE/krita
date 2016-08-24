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
#include "kis_visual_color_selector.h"

#include <QColor>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QVector>
#include <QVBoxLayout>
#include <QList>
#include <QPolygon>
#include <QRect>
#include <QtMath>

#include <KSharedConfig>
#include <KConfigGroup>

#include "KoColorConversions.h"
#include "KoColorDisplayRendererInterface.h"
#include "KoChannelInfo.h"
#include <KoColorModelStandardIds.h>
#include <QPointer>
#include "kis_signal_compressor.h"

struct KisVisualColorSelector::Private
{
    KoColor currentcolor;
    const KoColorSpace *currentCS;
    QList <KisVisualColorSelectorShape*> widgetlist;
    bool updateSelf = false;
    const KoColorDisplayRendererInterface *displayRenderer = 0;
    KisVisualColorSelector::Configuration acs_config;
    //Current coordinates.
    QVector <qreal> currentCoordinates;
};

KisVisualColorSelector::KisVisualColorSelector(QWidget *parent) : QWidget(parent), m_d(new Private)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    QVBoxLayout *layout = new QVBoxLayout;
    this->setLayout(layout);

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    m_d->acs_config = Configuration::fromString(cfg.readEntry("colorSelectorConfiguration", KisVisualColorSelector::Configuration().toString()));
    //m_d->updateSelf = new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE, this);
}

KisVisualColorSelector::~KisVisualColorSelector()
{

}

void KisVisualColorSelector::slotSetColor(KoColor c)
{
    if (m_d->updateSelf==false) {
        m_d->currentcolor = c;
        if (m_d->currentCS != c.colorSpace()) {
            slotsetColorSpace(c.colorSpace());
        }
    }
    updateSelectorElements();
}

void KisVisualColorSelector::slotsetColorSpace(const KoColorSpace *cs)
{
    if (m_d->currentCS != cs)
    {
        m_d->currentCS = cs;
        slotRebuildSelectors();
    }

}

void KisVisualColorSelector::slotRebuildSelectors()
{
    if (this->children().at(0)) {
        qDeleteAll(this->children());
    }
    m_d->widgetlist.clear();
    QLayout *layout = new QHBoxLayout;
    //redraw all the widgets.
    if (m_d->currentCS->colorChannelCount() == 1) {
        KisVisualRectangleSelectorShape *bar =  new KisVisualRectangleSelectorShape(this, KisVisualRectangleSelectorShape::onedimensional,KisVisualColorSelectorShape::Channel, m_d->currentCS, 0, 0);
        bar->setMaximumWidth(width()*0.1);
        bar->setMaximumHeight(height());
        connect (bar, SIGNAL(sigNewColor(KoColor)), this, SLOT(updateFromWidgets(KoColor)));
        layout->addWidget(bar);
        m_d->widgetlist.append(bar);
    } else if (m_d->currentCS->colorChannelCount() == 3) {
        int sizeValue = qMin(width(), height());
        int borderWidth = qMax(sizeValue*0.1, 20.0);

        KisVisualColorSelectorShape::ColorModel modelS = KisVisualColorSelectorShape::HSV;
        int channel1 = 0;
        int channel2 = 1;
        int channel3 = 2;

        switch(m_d->acs_config.subTypeParameter)
        {
        case H:
            channel1 = 0; break;
        case hsyS:
        case hsiS:
        case hslS:
        case hsvS:
            channel1 = 1; break;
        case V:
        case L:
        case I:
        case Y:
            channel1 = 2; break;
        }

        switch(m_d->acs_config.mainTypeParameter)
        {
        case hsySH:
            modelS = KisVisualColorSelectorShape::HSY;
            channel2 = 0;
            channel3 = 1;
            break;
        case hsiSH:
            modelS = KisVisualColorSelectorShape::HSL;
            channel2 = 0;
            channel3 = 1;
            break;
        case hslSH:
            modelS = KisVisualColorSelectorShape::HSI;
            channel2 = 0;
            channel3 = 1;
            break;
        case hsvSH:
            modelS = KisVisualColorSelectorShape::HSV;
            channel2 = 0;
            channel3 = 1;
            break;
        case YH:
            modelS = KisVisualColorSelectorShape::HSY;
            channel2 = 0;
            channel3 = 2;
            break;
        case LH:
            modelS = KisVisualColorSelectorShape::HSL;
            channel2 = 0;
            channel3 = 2;
            break;
        case IH:
            modelS = KisVisualColorSelectorShape::HSL;
            channel2 = 0;
            channel3 = 2;
            break;
        case VH:
            modelS = KisVisualColorSelectorShape::HSV;
            channel2 = 0;
            channel3 = 2;
            break;
        case SY:
            modelS = KisVisualColorSelectorShape::HSY;
            channel2 = 1;
            channel3 = 2;
            break;
        case SI:
            modelS = KisVisualColorSelectorShape::HSI;
            channel2 = 1;
            channel3 = 2;
            break;
        case SL:
            modelS = KisVisualColorSelectorShape::HSL;
            channel2 = 1;
            channel3 = 2;
            break;
        case SV:
        case SV2:
            modelS = KisVisualColorSelectorShape::HSV;
            channel2 = 1;
            channel3 = 2;
            break;
        }
        if (m_d->acs_config.mainType==Triangle) {
            modelS = KisVisualColorSelectorShape::HSV;
            //Triangle only really works in HSV mode.
        }
        KisVisualColorSelectorShape *bar;
        if (m_d->acs_config.subType==Ring) {
             bar =  new KisVisualEllipticalSelectorShape(this,
                                                                                     KisVisualColorSelectorShape::onedimensional,
                                                                                     modelS,
                                                                                     m_d->currentCS, channel1, channel1,
                                                                                     m_d->displayRenderer, borderWidth,KisVisualEllipticalSelectorShape::border);
            bar->resize(sizeValue, sizeValue);
        } else if (m_d->acs_config.subType==Slider) {
            bar =  new KisVisualRectangleSelectorShape(this,
                                                                                        KisVisualRectangleSelectorShape::onedimensional,
                                                                                        modelS,
                                                                                        m_d->currentCS, channel1, channel2,
                                                                                        m_d->displayRenderer, borderWidth);
            bar->setMaximumWidth(borderWidth);
            bar->setMinimumWidth(borderWidth);
            bar->setMinimumHeight(sizeValue);
        }

        bar->setColor(m_d->currentcolor);
        m_d->widgetlist.append(bar);

        KisVisualColorSelectorShape *block;
        if (m_d->acs_config.mainType==Triangle) {
            block =  new KisVisualTriangleSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                                                        modelS,
                                                                                        m_d->currentCS, channel2, channel3,
                                                                                        m_d->displayRenderer);
            block->setGeometry(bar->getSpaceForTriangle(this->geometry()));
        } else if (m_d->acs_config.mainType==Square) {
            block =  new KisVisualRectangleSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                                                          modelS,
                                                                                          m_d->currentCS, channel2, channel3,
                                                                                          m_d->displayRenderer);
            block->setGeometry(bar->getSpaceForSquare(this->geometry()));
         } else {
            block =  new KisVisualEllipticalSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                                                            modelS,
                                                                                            m_d->currentCS, channel2, channel3,
                                                                                            m_d->displayRenderer);
            block->setGeometry(bar->getSpaceForCircle(this->geometry()));

        }

        block->setColor(m_d->currentcolor);
        connect (bar, SIGNAL(sigNewColor(KoColor)), block, SLOT(setColorFromSibling(KoColor)));
        connect (block, SIGNAL(sigNewColor(KoColor)), SLOT(updateFromWidgets(KoColor)));
        connect (bar, SIGNAL(sigHSXchange()), SLOT(HSXwrangler()));
        connect (block, SIGNAL(sigHSXchange()), SLOT(HSXwrangler()));
        m_d->widgetlist.append(block);


    } else if (m_d->currentCS->colorChannelCount() == 4) {
        KisVisualRectangleSelectorShape *block =  new KisVisualRectangleSelectorShape(this, KisVisualRectangleSelectorShape::twodimensional,KisVisualColorSelectorShape::Channel, m_d->currentCS, 0, 1);
        KisVisualRectangleSelectorShape *block2 =  new KisVisualRectangleSelectorShape(this, KisVisualRectangleSelectorShape::twodimensional,KisVisualColorSelectorShape::Channel, m_d->currentCS, 2, 3);
        block->setMaximumWidth(width()*0.5);
        block->setMaximumHeight(height());
        block2->setMaximumWidth(width()*0.5);
        block2->setMaximumHeight(height());
        block->setColor(m_d->currentcolor);
        block2->setColor(m_d->currentcolor);
        connect (block, SIGNAL(sigNewColor(KoColor)), block2, SLOT(setColorFromSibling(KoColor)));
        connect (block2, SIGNAL(sigNewColor(KoColor)), SLOT(updateFromWidgets(KoColor)));
        layout->addWidget(block);
        layout->addWidget(block2);
        m_d->widgetlist.append(block);
        m_d->widgetlist.append(block2);
    }
    this->setLayout(layout);
}

void KisVisualColorSelector::setDisplayRenderer (const KoColorDisplayRendererInterface *displayRenderer) {
    m_d->displayRenderer = displayRenderer;
    if (m_d->widgetlist.size()>0) {
        Q_FOREACH (KisVisualColorSelectorShape *shape, m_d->widgetlist) {
            shape->setDisplayRenderer(displayRenderer);
        }
    }
}

void KisVisualColorSelector::updateSelectorElements()
{
    //first lock all elements from sending updates, then update all elements.
    Q_FOREACH (KisVisualColorSelectorShape *shape, m_d->widgetlist) {
        shape->blockSignals(true);
    }

    Q_FOREACH (KisVisualColorSelectorShape *shape, m_d->widgetlist) {
        if (m_d->updateSelf==false) {
            shape->setColor(m_d->currentcolor);
        } else {
            shape->setColorFromSibling(m_d->currentcolor);
        }
    }
    Q_FOREACH (KisVisualColorSelectorShape *shape, m_d->widgetlist) {
        shape->blockSignals(false);
    }

}

void KisVisualColorSelector::updateFromWidgets(KoColor c)
{
    m_d->currentcolor = c;
    Q_EMIT sigNewColor(c);
    m_d->updateSelf = true;
}

void KisVisualColorSelector::leaveEvent(QEvent *)
{
    m_d->updateSelf = false;
}

void KisVisualColorSelector::resizeEvent(QResizeEvent *) {
    int sizeValue = qMin(width(), height());
    int borderWidth = qMax(sizeValue*0.1, 20.0);
    QRect newrect(0,0, this->geometry().width(), this->geometry().height());
    if (m_d->currentCS->colorChannelCount()==3) {
        if (m_d->acs_config.subType==Ring) {
            m_d->widgetlist.at(0)->resize(sizeValue,sizeValue);
        } else if (m_d->acs_config.subType==Slider) {
            m_d->widgetlist.at(0)->setMaximumWidth(borderWidth);
            m_d->widgetlist.at(0)->setMinimumWidth(borderWidth);
            m_d->widgetlist.at(0)->setMinimumHeight(sizeValue);
            m_d->widgetlist.at(0)->setMaximumHeight(sizeValue);
        }
        m_d->widgetlist.at(0)->setBorderWidth(borderWidth);

        if (m_d->acs_config.mainType==Triangle) {
            m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForTriangle(newrect));
        } else if (m_d->acs_config.mainType==Square) {
            m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForSquare(newrect));
        } else if (m_d->acs_config.mainType==Wheel) {
            m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForCircle(newrect));
        }
    }
    Q_FOREACH (KisVisualColorSelectorShape *shape, m_d->widgetlist) {
        shape->update();
    }
}

void KisVisualColorSelector::HSXwrangler()
{
    m_d->currentCoordinates = QVector <qreal>(3);

    QVector <qreal> w1 = m_d->widgetlist.at(0)->getHSX(m_d->currentCoordinates, true);
    QVector <qreal> w2 = m_d->widgetlist.at(1)->getHSX(m_d->currentCoordinates, true);
    QVector <int> ch(3);q

    ch[0] = m_d->widgetlist.at(0)->getChannels().at(0);
    ch[1] = m_d->widgetlist.at(1)->getChannels().at(0);
    ch[2] = m_d->widgetlist.at(1)->getChannels().at(1);

    m_d->currentCoordinates[ch[0]] = w1[ch[0]];
    m_d->currentCoordinates[ch[1]] = w2[ch[1]];
    m_d->currentCoordinates[ch[2]] = w2[ch[2]];

    m_d->widgetlist.at(0)->setHSX(m_d->currentCoordinates);
    m_d->widgetlist.at(1)->setHSX(m_d->currentCoordinates);
}

/*------------Selector shape------------*/
struct KisVisualColorSelectorShape::Private
{
    QImage gradient;
    QImage fullSelector;
    bool imagesNeedUpdate= true;
    QPointF currentCoordinates;
    Dimensions dimension;
    ColorModel model;
    const KoColorSpace *cs;
    KoColor currentColor;
    int channel1;
    int channel2;
    KisSignalCompressor *updateTimer;
    KisSignalCompressor *siblingTimer;
    bool mousePressActive = false;
    const KoColorDisplayRendererInterface *displayRenderer = 0;
    qreal hue = 0.0;
    qreal sat = 0.0;

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
    m_d->cs = cs;
    m_d->currentColor = KoColor();
    m_d->currentColor.setOpacity(1.0);
    m_d->currentColor.convertTo(cs);
    int maxchannel = m_d->cs->colorChannelCount()-1;
    m_d->channel1 = qBound(0, channel1, maxchannel);
    m_d->channel2 = qBound(0, channel2, maxchannel);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_d->updateTimer = new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE, this);
    m_d->siblingTimer = new KisSignalCompressor(30 /* ms */, KisSignalCompressor::POSTPONE, this);
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

void KisVisualColorSelectorShape::setColor(KoColor c)
{
    if (c.colorSpace() != m_d->cs) {
        c.convertTo(m_d->cs);
    }
    m_d->currentColor = c;
    updateCursor();
    convertShapeCoordinateToKoColor(getCursorPosition());
    m_d->imagesNeedUpdate = true;
    update();
}

void KisVisualColorSelectorShape::setColorFromSibling(KoColor c)
{
    if (c.colorSpace() != m_d->cs) {
        c.convertTo(m_d->cs);
    }
    m_d->currentColor = c;
    Q_EMIT sigNewColor(c);
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
    m_d->imagesNeedUpdate = true;
    updateCursor();
    //m_d->currentColor = convertShapeCoordinateToKoColor(getCursorPosition());
    update();
}

void KisVisualColorSelectorShape::forceImageUpdate()
{
    m_d->imagesNeedUpdate = true;
}

QColor KisVisualColorSelectorShape::getColorFromConverter(KoColor c){
    QColor col;
    if (m_d->displayRenderer && m_d->displayRenderer->getPaintingColorSpace()==m_d->cs) {
        col = m_d->displayRenderer->toQColor(c);
    } else {
        col = c.toQColor();
    }
    return col;
}

void KisVisualColorSelectorShape::slotSetActiveChannels(int channel1, int channel2)
{
    int maxchannel = m_d->cs->colorChannelCount()-1;
    m_d->channel1 = qBound(0, channel1, maxchannel);
    m_d->channel2 = qBound(0, channel2, maxchannel);
    m_d->imagesNeedUpdate = true;
    update();
}

QImage KisVisualColorSelectorShape::getImageMap()
{
    if (m_d->imagesNeedUpdate == true) {
        m_d->imagesNeedUpdate = false;
        m_d->gradient = QImage(width(), height(), QImage::Format_ARGB32);
        m_d->gradient.fill(Qt::transparent);
        QImage img(width(), height(), QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        for (int y = 0; y<img.height(); y++) {
            uint* data = reinterpret_cast<uint*>(img.scanLine(y));
            for (int x=0; x<img.width(); x++,  ++data) {
                QPoint widgetPoint(x,y);
                QPointF newcoordinate = convertWidgetCoordinateToShapeCoordinate(widgetPoint);
                KoColor c = convertShapeCoordinateToKoColor(newcoordinate);
                QColor col = getColorFromConverter(c);
                *data = col.rgba();
            }
        }

        m_d->gradient = img;
    }
    return m_d->gradient;
}

KoColor KisVisualColorSelectorShape::convertShapeCoordinateToKoColor(QPointF coordinates, bool cursor)
{
    KoColor c = m_d->currentColor;
    QVector <float> channelValues (c.colorSpace()->channelCount());
    channelValues.fill(1.0);
    c.colorSpace()->normalisedChannelsValue(c.data(), channelValues);
    QVector <float> channelValuesDisplay = channelValues;
    QVector <qreal> maxvalue(c.colorSpace()->channelCount());
    maxvalue.fill(1.0);
    if (m_d->displayRenderer
            && m_d->displayRenderer->getPaintingColorSpace()==m_d->cs
            && m_d->cs->colorDepthId() == Float16BitsColorDepthID
            && m_d->cs->colorDepthId() == Float32BitsColorDepthID
            && m_d->cs->colorDepthId() == Float64BitsColorDepthID
            && m_d->cs->colorModelId() != LABAColorModelID
            && m_d->cs->colorModelId() != CMYKAColorModelID) {
        for (int ch = 0; ch<maxvalue.size(); ch++) {
            KoChannelInfo *channel = m_d->cs->channels()[ch];
            maxvalue[ch] = m_d->displayRenderer->maxVisibleFloatValue(channel);
            channelValues[ch] = channelValues[ch]/(maxvalue[ch]);
            channelValuesDisplay[KoChannelInfo::displayPositionToChannelIndex(ch, m_d->cs->channels())] = qBound((float)0.0,channelValues[ch], (float)1.0);
        }
    } else {
        for (int i =0; i<channelValues.size();i++) {
            channelValuesDisplay[KoChannelInfo::displayPositionToChannelIndex(i, m_d->cs->channels())] = qBound((float)0.0,channelValues[i], (float)1.0);
        }
    }
    qreal huedivider = 1.0;
    qreal huedivider2 = 1.0;

    if (m_d->channel1==0) {
        huedivider = 360.0;
    }
    if (m_d->channel2==0) {
        huedivider2 = 360.0;
    }
    if (m_d->model != ColorModel::Channel && c.colorSpace()->colorModelId().id() == "RGBA") {
        if (c.colorSpace()->colorModelId().id() == "RGBA") {
            if (m_d->model == ColorModel::HSV){
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
                if (cursor==true){setHSX(convertvectorfloatToqreal(inbetween));Q_EMIT sigHSXchange();}
                HSVToRGB(qMax(inbetween[0],(float)0.0), inbetween[1], inbetween[2], &channelValuesDisplay[0], &channelValuesDisplay[1], &channelValuesDisplay[2]);
            } else if (m_d->model == ColorModel::HSL) {
                /*
                 * HSLToRGB can give negative values on the grey. I fixed the fromNormalisedChannel function to clamp,
                 * but you might want to manually clamp for floating point values.
                 */
                QVector <float> inbetween(3);
                RGBToHSL(channelValuesDisplay[1],channelValuesDisplay[1], channelValuesDisplay[2], &inbetween[0], &inbetween[1], &inbetween[2]);
                inbetween = convertvectorqrealTofloat(getHSX(convertvectorfloatToqreal(inbetween)));
                inbetween[m_d->channel1] = coordinates.x()*huedivider;
                if (m_d->dimension == Dimensions::twodimensional) {
                    inbetween[m_d->channel2] = coordinates.y()*huedivider2;
                }
                if (cursor==true){setHSX(convertvectorfloatToqreal(inbetween));Q_EMIT sigHSXchange();}
                HSLToRGB(inbetween[0], inbetween[1], inbetween[2],&channelValuesDisplay[0],&channelValuesDisplay[1], &channelValuesDisplay[2]);
            } else if (m_d->model == ColorModel::HSI) {
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
                if (cursor==true){setHSX(inbetween);Q_EMIT sigHSXchange();}
                HSIToRGB(inbetween[0], inbetween[1], inbetween[2],&chan2[0],&chan2[1], &chan2[2]);
                channelValuesDisplay = convertvectorqrealTofloat(chan2);
            } else /*if (m_d->model == ColorModel::HSY)*/ {
                /*
                 * HSY is pretty slow to render due being a pretty over-the-top function.
                 * Might be worth investigating whether HCY can be used instead, but I have had
                 * some weird results with that.
                 */
                QVector <qreal> luma= m_d->cs->lumaCoefficients();
                QVector <qreal> chan2 = convertvectorfloatToqreal(channelValuesDisplay);
                QVector <qreal> inbetween(3);
                RGBToHSY(chan2[0],chan2[1], chan2[2], &inbetween[0], &inbetween[1], &inbetween[2],
                        luma[0], luma[1], luma[2]);
                inbetween = getHSX(inbetween);
                inbetween[m_d->channel1] = coordinates.x();
                if (m_d->dimension == Dimensions::twodimensional) {
                    inbetween[m_d->channel2] = coordinates.y();
                }
                if (cursor==true){setHSX(inbetween);Q_EMIT sigHSXchange();}
                HSYToRGB(inbetween[0], inbetween[1], inbetween[2],&chan2[0],&chan2[1], &chan2[2],
                        luma[0], luma[1], luma[2]);
                channelValuesDisplay = convertvectorqrealTofloat(chan2);
            }
        }
    } else {
        channelValuesDisplay[m_d->channel1] = coordinates.x();
        if (m_d->dimension == Dimensions::twodimensional) {
            channelValuesDisplay[m_d->channel2] = coordinates.y();
        }
    }
    for (int i=0; i<channelValues.size();i++) {
        channelValues[i] = qBound(0.0,channelValuesDisplay[KoChannelInfo::displayPositionToChannelIndex(i, m_d->cs->channels())]*(maxvalue[i]),1.0);
    }
    c.colorSpace()->fromNormalisedChannelsValue(c.data(), channelValues);
    return c;
}

QPointF KisVisualColorSelectorShape::convertKoColorToShapeCoordinate(KoColor c)
{
    if (c.colorSpace() != m_d->cs) {
        c.convertTo(m_d->cs);
    }
    QVector <float> channelValues (m_d->currentColor.colorSpace()->channelCount());
    channelValues.fill(1.0);
    m_d->cs->normalisedChannelsValue(c.data(), channelValues);
    QVector <float> channelValuesDisplay = channelValues;
    QVector <qreal> maxvalue(c.colorSpace()->channelCount());
    maxvalue.fill(1.0);
    if (m_d->displayRenderer
            && m_d->displayRenderer->getPaintingColorSpace()==m_d->cs
            && m_d->cs->colorDepthId() == Float16BitsColorDepthID
            && m_d->cs->colorDepthId() == Float32BitsColorDepthID
            && m_d->cs->colorDepthId() == Float64BitsColorDepthID
            && m_d->cs->colorModelId() != LABAColorModelID
            && m_d->cs->colorModelId() != CMYKAColorModelID) {
        for (int ch = 0; ch<maxvalue.size(); ch++) {
            KoChannelInfo *channel = m_d->cs->channels()[ch];
            maxvalue[ch] = m_d->displayRenderer->maxVisibleFloatValue(channel);
            channelValues[ch] = channelValues[ch]/(maxvalue[ch]);
            channelValuesDisplay[KoChannelInfo::displayPositionToChannelIndex(ch, m_d->cs->channels())] = qBound((float)0.0,channelValues[ch], (float)1.0);
        }
    } else {
        for (int i =0; i<channelValues.size();i++) {
            channelValuesDisplay[KoChannelInfo::displayPositionToChannelIndex(i, m_d->cs->channels())] = qBound((float)0.0,channelValues[i], (float)1.0);
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
    if (m_d->model != ColorModel::Channel && c.colorSpace()->colorModelId().id() == "RGBA") {
        if (c.colorSpace()->colorModelId().id() == "RGBA") {
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
                QVector <qreal> luma = m_d->cs->lumaCoefficients();
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
        coordinates.setX(channelValuesDisplay[m_d->channel1]);
        if (m_d->dimension == Dimensions::twodimensional) {
            coordinates.setY(channelValuesDisplay[m_d->channel2]);
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
    m_d->mousePressActive = true;
    QPointF coordinates = convertWidgetCoordinateToShapeCoordinate(e->pos());
    KoColor col = convertShapeCoordinateToKoColor(coordinates, true);
    setColor(col);
    Q_EMIT sigNewColor(col);
    m_d->updateTimer->start();
}

void KisVisualColorSelectorShape::mouseMoveEvent(QMouseEvent *e)
{
    if (m_d->mousePressActive==true && this->mask().contains(e->pos())) {
        QPointF coordinates = convertWidgetCoordinateToShapeCoordinate(e->pos());
        KoColor col = convertShapeCoordinateToKoColor(coordinates, true);
        setColor(col);
        if (!m_d->updateTimer->isActive()) {
            Q_EMIT sigNewColor(col);
            m_d->updateTimer->start();
        }
    } else {
        e->ignore();
    }
}

void KisVisualColorSelectorShape::mouseReleaseEvent(QMouseEvent *)
{
    m_d->mousePressActive = false;
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

KisVisualColorSelectorShape::Dimensions KisVisualColorSelectorShape::getDimensions()
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
    return m_d->currentColor;
}

QVector <qreal> KisVisualColorSelectorShape::getHSX(QVector<qreal> hsx, bool wrangler)
{
    QVector <qreal> ihsx = hsx;
    if (!wrangler){
        if (m_d->model==HSV){
            if (hsx[2]<=0.0) {
                ihsx[1] = m_d->sat;
            }
        } else {
            if ((hsx[2]<=0.0 || hsx[2]>=1.0)) {
                ihsx[1] = m_d->sat;
            }
        }
        if ((hsx[1]<0.0 || hsx[0]<0.0)){
            ihsx[0]=m_d->hue;
        }
    } else {
        ihsx[0]=m_d->hue;
        ihsx[1]=m_d->sat;
    }
    return ihsx;
}

void KisVisualColorSelectorShape::setHSX(QVector<qreal> hsx)
{
    m_d->sat = hsx[1];
    m_d->hue = hsx[0];
}

QVector <int> KisVisualColorSelectorShape::getChannels()
{
    QVector <int> channels(2);
    channels[0] = m_d->channel1;
    channels[1] = m_d->channel2;
    return channels;
}

/*-----------Rectangle Shape------------*/

KisVisualRectangleSelectorShape::KisVisualRectangleSelectorShape(QWidget *parent,
                                                                 Dimensions dimension,
                                                                 ColorModel model,
                                                                 const KoColorSpace *cs,
                                                                 int channel1, int channel2,
                                                                 const KoColorDisplayRendererInterface *displayRenderer,
                                                                 int width,
                                                                 singelDTypes d)
    : KisVisualColorSelectorShape(parent, dimension, model, cs, channel1, channel2, displayRenderer)
{
    m_type = d;
    m_barWidth = width;
}

KisVisualRectangleSelectorShape::~KisVisualRectangleSelectorShape()
{

}

void KisVisualRectangleSelectorShape::setBorderWidth(int width)
{
    m_barWidth = width;
}

QRect KisVisualRectangleSelectorShape::getSpaceForSquare(QRect geom)
{
    QPointF tl;
    QPointF br;

    if (m_type==KisVisualRectangleSelectorShape::vertical) {
        br = geom.bottomRight();
        tl = QPoint(geom.topLeft().x()+m_barWidth, geom.topLeft().y());
    } else if (m_type==KisVisualRectangleSelectorShape::horizontal) {
        br = geom.bottomRight();
        tl = QPoint(geom.topLeft().x(), geom.topLeft().y()+m_barWidth);
    } else {
        tl = QPointF (geom.topLeft().x()+m_barWidth, geom.topLeft().y()+m_barWidth);
        br = QPointF (geom.bottomRight().x()-m_barWidth, geom.bottomRight().y()-m_barWidth);

    }
    QRect a(tl.toPoint(), br.toPoint());
    QRect r(a.left(), a.top(), qMin(a.height(), a.width()), qMin(a.height(), a.width()));
    return r;
}

QRect KisVisualRectangleSelectorShape::getSpaceForCircle(QRect geom)
{
    return getSpaceForSquare(geom);
}

QRect KisVisualRectangleSelectorShape::getSpaceForTriangle(QRect geom)
{
    return getSpaceForSquare(geom);
}

QPointF KisVisualRectangleSelectorShape::convertShapeCoordinateToWidgetCoordinate(QPointF coordinate)
{
    qreal x = m_barWidth/2;
    qreal y = m_barWidth/2;
    KisVisualColorSelectorShape::Dimensions dimension = getDimensions();
    if (dimension == KisVisualColorSelectorShape::onedimensional) {
        if ( m_type == KisVisualRectangleSelectorShape::vertical) {
            y = coordinate.x()*height();
        } else if (m_type == KisVisualRectangleSelectorShape::horizontal) {
            x = coordinate.x()*width();
        } else if (m_type == KisVisualRectangleSelectorShape::border) {

            QRectF innerRect(m_barWidth/2, m_barWidth/2, width()-m_barWidth, height()-m_barWidth);
            QPointF left (innerRect.left(),innerRect.center().y());
            QList <QLineF> polygonLines;
            polygonLines.append(QLineF(left, innerRect.topLeft()));
            polygonLines.append(QLineF(innerRect.topLeft(), innerRect.topRight()));
            polygonLines.append(QLineF(innerRect.topRight(), innerRect.bottomRight()));
            polygonLines.append(QLineF(innerRect.bottomRight(), innerRect.bottomLeft()));
            polygonLines.append(QLineF(innerRect.bottomLeft(), left));

            qreal totalLength =0.0;
            Q_FOREACH(QLineF line, polygonLines) {
                totalLength += line.length();
            }

            qreal length = coordinate.x()*totalLength;
            QPointF intersect(x,y);
            Q_FOREACH(QLineF line, polygonLines) {
                if (line.length()>length && length>0){
                    intersect = line.pointAt(length/line.length());

                }
                length-=line.length();
            }
            x = qRound(intersect.x());
            y = qRound(intersect.y());

        } else /*if (m_type == KisVisualRectangleSelectorShape::borderMirrored)*/  {

            QRectF innerRect(m_barWidth/2, m_barWidth/2, width()-m_barWidth, height()-m_barWidth);
            QPointF bottom (innerRect.center().x(), innerRect.bottom());
            QList <QLineF> polygonLines;
            polygonLines.append(QLineF(bottom, innerRect.bottomLeft()));
            polygonLines.append(QLineF(innerRect.bottomLeft(), innerRect.topLeft()));
            polygonLines.append(QLineF(innerRect.topLeft(), innerRect.topRight()));
            polygonLines.append(QLineF(innerRect.topRight(), innerRect.bottomRight()));
            polygonLines.append(QLineF(innerRect.bottomRight(), bottom));

            qreal totalLength =0.0;
            Q_FOREACH(QLineF line, polygonLines) {
                totalLength += line.length();
            }

            qreal length = coordinate.x()*(totalLength/2);
            QPointF intersect(x,y);
            if (coordinate.y()==1) {
                for (int i = polygonLines.size()-1; i==0; i--) {
                    QLineF line = polygonLines.at(i);
                    if (line.length()>length && length>0){
                        intersect = line.pointAt(length/line.length());

                    }
                    length-=line.length();
                }
            } else {
                Q_FOREACH(QLineF line, polygonLines) {
                    if (line.length()>length && length>0){
                        intersect = line.pointAt(length/line.length());

                    }
                    length-=line.length();
                }
            }
            x = qRound(intersect.x());
            y = qRound(intersect.y());

        }
    } else {
        x = coordinate.x()*width();
        y = coordinate.y()*height();
    }
    return QPointF(x,y);
}

QPointF KisVisualRectangleSelectorShape::convertWidgetCoordinateToShapeCoordinate(QPoint coordinate)
{
    //default implementation:
    qreal x = 0.5;
    qreal y = 0.5;
    KisVisualColorSelectorShape::Dimensions dimension = getDimensions();
    if (getMaskMap().contains(coordinate)) {
        if (dimension == KisVisualColorSelectorShape::onedimensional ) {
            if (m_type == KisVisualRectangleSelectorShape::vertical) {
                x = (qreal)coordinate.y()/(qreal)height();
            } else if (m_type == KisVisualRectangleSelectorShape::horizontal) {
                x = (qreal)coordinate.x()/(qreal)width();
            } else if (m_type == KisVisualRectangleSelectorShape::border) {
                //border

                QRectF innerRect(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2));
                QPointF left (innerRect.left(),innerRect.center().y());
                QList <QLineF> polygonLines;
                polygonLines.append(QLineF(left, innerRect.topLeft()));
                polygonLines.append(QLineF(innerRect.topLeft(), innerRect.topRight()));
                polygonLines.append(QLineF(innerRect.topRight(), innerRect.bottomRight()));
                polygonLines.append(QLineF(innerRect.bottomRight(), innerRect.bottomLeft()));
                polygonLines.append(QLineF(innerRect.bottomLeft(), left));

                QLineF radius(coordinate, this->geometry().center());
                QPointF intersect(0.5,0.5);
                qreal length = 0.0;
                qreal totalLength = 0.0;
                bool foundIntersect = false;
                Q_FOREACH(QLineF line, polygonLines) {
                    if (line.intersect(radius,&intersect)==QLineF::BoundedIntersection && foundIntersect==false)
                    {
                        foundIntersect = true;
                        length+=QLineF(line.p1(), intersect).length();

                    }
                    if (foundIntersect==false) {
                        length+=line.length();
                    }
                    totalLength+=line.length();
                }

                x = length/totalLength;

            } else /*if (m_type == KisVisualRectangleSelectorShape::borderMirrored)*/  {
                //border

                QRectF innerRect(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2));
                QPointF bottom (innerRect.center().x(), innerRect.bottom());
                QList <QLineF> polygonLines;
                polygonLines.append(QLineF(bottom, innerRect.bottomLeft()));
                polygonLines.append(QLineF(innerRect.bottomLeft(), innerRect.topLeft()));
                polygonLines.append(QLineF(innerRect.topLeft(), innerRect.topRight()));
                polygonLines.append(QLineF(innerRect.topRight(), innerRect.bottomRight()));
                polygonLines.append(QLineF(innerRect.bottomRight(), bottom));

                QLineF radius(coordinate, this->geometry().center());
                QPointF intersect(0.5,0.5);
                qreal length = 0.0;
                qreal totalLength = 0.0;
                bool foundIntersect = false;
                Q_FOREACH(QLineF line, polygonLines) {
                    if (line.intersect(radius,&intersect)==QLineF::BoundedIntersection && foundIntersect==false)
                    {
                        foundIntersect = true;
                        length+=QLineF(line.p1(), intersect).length();

                    }
                    if (foundIntersect==false) {
                        length+=line.length();
                    }
                    totalLength+=line.length();
                }
                int halflength = totalLength/2;

                if (length>halflength) {
                    x = (halflength - (length-halflength))/halflength;
                    y = 1.0;
                } else {
                    x = length/halflength;
                    y = 0.0;
                }
            }
        }
        else {
            x = (qreal)coordinate.x()/(qreal)width();
            y = (qreal)coordinate.y()/(qreal)height();
        }
    }
    return QPointF(x, y);
}

QRegion KisVisualRectangleSelectorShape::getMaskMap()
{
    QRegion mask = QRegion(0,0,width(),height());
    if (m_type==KisVisualRectangleSelectorShape::border || m_type==KisVisualRectangleSelectorShape::borderMirrored) {
        mask = mask.subtracted(QRegion(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2)));
    }
    return mask;
}
void KisVisualRectangleSelectorShape::resizeEvent(QResizeEvent *)
{
    forceImageUpdate();
}

void KisVisualRectangleSelectorShape::drawCursor()
{
    QPointF cursorPoint = convertShapeCoordinateToWidgetCoordinate(getCursorPosition());
    QImage fullSelector = getImageMap();
    QColor col = getColorFromConverter(getCurrentColor());
    QPainter painter;
    painter.begin(&fullSelector);
    painter.setRenderHint(QPainter::Antialiasing);
    //QPainterPath path;
    QBrush fill;
    fill.setStyle(Qt::SolidPattern);

    int cursorwidth = 5;
    QRect rect(cursorPoint.toPoint().x()-cursorwidth,cursorPoint.toPoint().y()-cursorwidth,
               cursorwidth*2,cursorwidth*2);
    if (m_type==KisVisualRectangleSelectorShape::vertical){
        int x = ( cursorPoint.x()-(width()/2)+1 );
        int y = ( cursorPoint.y()-cursorwidth );
        rect.setCoords(x, y, x+width()-2, y+(cursorwidth*2));
    } else {
        int x = cursorPoint.x()-cursorwidth;
        int y = cursorPoint.y()-(height()/2)+1;
        rect.setCoords(x, y, x+(cursorwidth*2), y+cursorwidth-2);
    }
    QRectF innerRect(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2));
    if (getDimensions() == KisVisualColorSelectorShape::onedimensional && m_type!=KisVisualRectangleSelectorShape::border && m_type!=KisVisualRectangleSelectorShape::borderMirrored) {
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawRect(rect);
        //set filter conversion!
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        rect.setCoords(rect.topLeft().x()+1, rect.topLeft().y()+1,
                       rect.topLeft().x()+rect.width()-2, rect.topLeft().y()+rect.height()-2);
        painter.drawRect(rect);

    }else if(m_type==KisVisualRectangleSelectorShape::borderMirrored){
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
        QPoint mirror(innerRect.center().x()+(innerRect.center().x()-cursorPoint.x()),cursorPoint.y());
        painter.drawEllipse(mirror, cursorwidth, cursorwidth);
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth-1, cursorwidth-1);
        painter.drawEllipse(mirror, cursorwidth-1, cursorwidth-1);

    } else {
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth-1.0, cursorwidth-1.0);
    }
    painter.end();
    setFullImage(fullSelector);
}

//----------------Elliptical--------------------------//
KisVisualEllipticalSelectorShape::KisVisualEllipticalSelectorShape(QWidget *parent,
                                                                 Dimensions dimension,
                                                                 ColorModel model,
                                                                 const KoColorSpace *cs,
                                                                 int channel1, int channel2,
                                                                 const KoColorDisplayRendererInterface *displayRenderer,
                                                                 int borwidth,
                                                                 singelDTypes d)
    : KisVisualColorSelectorShape(parent, dimension, model, cs, channel1, channel2, displayRenderer)
{
    m_type = d;
    m_barWidth = borwidth;
}

KisVisualEllipticalSelectorShape::~KisVisualEllipticalSelectorShape()
{

}

QSize KisVisualEllipticalSelectorShape::sizeHint() const
{
    return QSize(180,180);
}
void KisVisualEllipticalSelectorShape::setBorderWidth(int width)
{
    m_barWidth = width;
}

QRect KisVisualEllipticalSelectorShape::getSpaceForSquare(QRect geom)
{
    int sizeValue = qMin(width(),height());
    QRect b(geom.left(), geom.top(), sizeValue, sizeValue);
    QLineF radius(b.center(), QPointF(b.left()+m_barWidth, b.center().y()) );
    radius.setAngle(135);
    QPointF tl = radius.p2();
    radius.setAngle(315);
    QPointF br = radius.p2();
    QRect r(tl.toPoint(), br.toPoint());
    return r;
}

QRect KisVisualEllipticalSelectorShape::getSpaceForCircle(QRect geom)
{
    int sizeValue = qMin(width(),height());
    QRect b(geom.left(), geom.top(), sizeValue, sizeValue);
    QPointF tl = QPointF (b.topLeft().x()+m_barWidth, b.topLeft().y()+m_barWidth);
    QPointF br = QPointF (b.bottomRight().x()-m_barWidth, b.bottomRight().y()-m_barWidth);
    QRect r(tl.toPoint(), br.toPoint());
    return r;
}

QRect KisVisualEllipticalSelectorShape::getSpaceForTriangle(QRect geom)
{
    int sizeValue = qMin(width(),height());
    QRect b(geom.left(), geom.top(), sizeValue, sizeValue);
    QLineF radius(b.center(), QPointF(b.left()+m_barWidth, b.center().y()) );
    radius.setAngle(90);//point at yellowgreen :)
    QPointF t = radius.p2();
    radius.setAngle(330);//point to purple :)
    QPointF br = radius.p2();
    radius.setAngle(210);//point to cerulean :)
    QPointF bl = radius.p2();
    QPointF tl = QPoint(bl.x(),t.y());
    QRect r(tl.toPoint(), br.toPoint());
    return r;
}

QPointF KisVisualEllipticalSelectorShape::convertShapeCoordinateToWidgetCoordinate(QPointF coordinate)
{
    qreal x;
    qreal y;
    qreal a = (qreal)width()*0.5;
    QPointF center(a, a);
    QLineF line(center, QPoint((m_barWidth*0.5),a));
    qreal angle = coordinate.x()*360.0;
    angle = fmod(angle+180.0,360.0);
    angle = 180.0-angle;
    angle = angle+180.0;
    if (m_type==KisVisualEllipticalSelectorShape::borderMirrored) {
        angle = (coordinate.x()/2)*360.0;
        angle = fmod((angle+90.0), 360.0);
    }
    line.setAngle(angle);
    if (getDimensions()!=KisVisualColorSelectorShape::onedimensional) {
        line.setLength(coordinate.y()*a);
    }
    x = qRound(line.p2().x());
    y = qRound(line.p2().y());
    return QPointF(x,y);
}

QPointF KisVisualEllipticalSelectorShape::convertWidgetCoordinateToShapeCoordinate(QPoint coordinate)
{
    //default implementation:
    qreal x = 0.5;
    qreal y = 1.0;
    QRect total(0, 0, width(), height());
    QLineF line(total.center(), coordinate);
    qreal a = total.width()/2;
    qreal angle;

    if (m_type!=KisVisualEllipticalSelectorShape::borderMirrored){
        angle = fmod((line.angle()+180.0), 360.0);
        angle = 180.0-angle;
        angle = angle+180.0;
        x = angle/360.0;
        if (getDimensions()==KisVisualColorSelectorShape::twodimensional) {
            y = qBound(0.0,line.length()/a, 1.0);
        }

    } else {
        angle = fmod((line.angle()+270.0), 360.0);
        if (angle>180.0) {
            angle = 180.0-angle;
            angle = angle+180;
        }
        x = (angle/360.0)*2;
        if (getDimensions()==KisVisualColorSelectorShape::twodimensional) {
            y = line.length()/a;
        }
    }


    return QPointF(x, y);
}

QRegion KisVisualEllipticalSelectorShape::getMaskMap()
{
    QRegion mask = QRegion(0,0,width(),height(), QRegion::Ellipse);
    if (getDimensions()==KisVisualColorSelectorShape::onedimensional) {
        mask = mask.subtracted(QRegion(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2), QRegion::Ellipse));
    }
    return mask;
}

void KisVisualEllipticalSelectorShape::resizeEvent(QResizeEvent *)
{
    forceImageUpdate();
}

void KisVisualEllipticalSelectorShape::drawCursor()
{
    QPointF cursorPoint = convertShapeCoordinateToWidgetCoordinate(getCursorPosition());
    QImage fullSelector = getImageMap();
    QColor col = getColorFromConverter(getCurrentColor());
    QPainter painter;
    painter.begin(&fullSelector);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.save();
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    QPen pen;
    pen.setWidth(5);
    painter.setPen(pen);
    painter.drawEllipse(QRect(0,0,width(),height()));
    if (getDimensions()==KisVisualColorSelectorShape::onedimensional) {
        painter.drawEllipse(QRect(this->geometry().top()+m_barWidth, this->geometry().left()+m_barWidth, this->geometry().width()-(m_barWidth*2), this->geometry().height()-(m_barWidth*2)));
    }
    painter.restore();

    QBrush fill;
    fill.setStyle(Qt::SolidPattern);

    int cursorwidth = 5;
    QRect innerRect(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2));
    if(m_type==KisVisualEllipticalSelectorShape::borderMirrored){
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
        QPoint mirror(innerRect.center().x()+(innerRect.center().x()-cursorPoint.x()),cursorPoint.y());
        painter.drawEllipse(mirror, cursorwidth, cursorwidth);
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth-1, cursorwidth-1);
        painter.drawEllipse(mirror, cursorwidth-1, cursorwidth-1);

    } else {
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth-1.0, cursorwidth-1.0);
    }
    painter.end();
    setFullImage(fullSelector);
}

//----------------Triangle--------------------------//
KisVisualTriangleSelectorShape::KisVisualTriangleSelectorShape(QWidget *parent,
                                                                 Dimensions dimension,
                                                                 ColorModel model,
                                                                 const KoColorSpace *cs,
                                                                 int channel1, int channel2,
                                                                 const KoColorDisplayRendererInterface *displayRenderer,
                                                                 int borwidth)
    : KisVisualColorSelectorShape(parent, dimension, model, cs, channel1, channel2, displayRenderer)
{
    m_barWidth = borwidth;
    QRect total(0,0,width()*0.9,width()*0.9);
    setTriangle();
}

KisVisualTriangleSelectorShape::~KisVisualTriangleSelectorShape()
{

}

void KisVisualTriangleSelectorShape::setBorderWidth(int width)
{
    m_barWidth = width;
}

QRect KisVisualTriangleSelectorShape::getSpaceForSquare(QRect geom)
{
    return geom;
}

QRect KisVisualTriangleSelectorShape::getSpaceForCircle(QRect geom)
{
    return geom;
}

QRect KisVisualTriangleSelectorShape::getSpaceForTriangle(QRect geom)
{
    return geom;
}
void KisVisualTriangleSelectorShape::setTriangle()
{
    QPoint apex = QPoint (width()*0.5,0);
    QPolygon triangle;
    triangle<< QPoint(0,height()) << apex << QPoint(width(),height()) << QPoint(0,height());
    m_triangle = triangle;
    QLineF a(triangle.at(0),triangle.at(1));
    QLineF b(triangle.at(0),triangle.at(2));
    QLineF ap(triangle.at(2), a.pointAt(0.5));
    QLineF bp(triangle.at(1), b.pointAt(0.5));
    QPointF intersect;
    ap.intersect(bp,&intersect);
    m_center = intersect;
    QLineF r(triangle.at(0), intersect);
    m_radius = r.length();
}

QPointF KisVisualTriangleSelectorShape::convertShapeCoordinateToWidgetCoordinate(QPointF coordinate)
{
    qreal offset=7.0;//the offset is so we get a nice little border that allows selecting extreme colors better.
    qreal y = qMin(coordinate.y()*(height()-offset*2)+offset+5.0, (qreal)height()-offset);

    qreal triWidth = width();
    qreal horizontalLineLength = y*(2./sqrt(3.));
    qreal horizontalLineStart = triWidth/2.-horizontalLineLength/2.;
    qreal relativeX = coordinate.x()*(horizontalLineLength-offset*2);
    qreal x = qMin(relativeX + horizontalLineStart + offset, (qreal)width()-offset*2);
    if (y<offset){
        x = 0.5*width();
    }

    return QPointF(x,y);
}

QPointF KisVisualTriangleSelectorShape::convertWidgetCoordinateToShapeCoordinate(QPoint coordinate)
{
    //default implementation: gotten from the kotrianglecolorselector/kis_color_selector_triangle.
    qreal x = 0.5;
    qreal y = 0.5;
    qreal offset=7.0; //the offset is so we get a nice little border that allows selecting extreme colors better.

    y = qMax((qreal)coordinate.y()-offset,0.0)/(height()-offset*2);

    qreal triWidth = width();
    qreal horizontalLineLength = ((qreal)coordinate.y())*(2./sqrt(3.))-(offset*2);
    qreal horizontalLineStart = (triWidth*0.5)-(horizontalLineLength*0.5);

    qreal relativeX = qMax((qreal)coordinate.x()-offset,0.0)-horizontalLineStart;
    x = relativeX/horizontalLineLength;
    if (coordinate.y()<offset){
        x = 0.5;
    }
    return QPointF(x, y);
}

QRegion KisVisualTriangleSelectorShape::getMaskMap()
{
    QRegion mask = QRegion(m_triangle);
    //QRegion mask =  QRegion();
    //if (getDimensions()==KisVisualColorSelectorShape::onedimensional) {
    //    mask = mask.subtracted(QRegion(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2)));
    //}
    return mask;
}

void KisVisualTriangleSelectorShape::resizeEvent(QResizeEvent *)
{
    setTriangle();
    forceImageUpdate();
}

void KisVisualTriangleSelectorShape::drawCursor()
{
    QPointF cursorPoint = convertShapeCoordinateToWidgetCoordinate(getCursorPosition());
    QImage fullSelector = getImageMap();
    QColor col = getColorFromConverter(getCurrentColor());
    QPainter painter;
    painter.begin(&fullSelector);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.save();
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    QPen pen;
    pen.setWidth(10);
    painter.setPen(pen);
    painter.drawPolygon(m_triangle);
    painter.restore();

    //QPainterPath path;
    QBrush fill;
    fill.setStyle(Qt::SolidPattern);

    int cursorwidth = 5;
    QRect innerRect(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2));
    /*if(m_type==KisVisualTriangleSelectorShape::borderMirrored){
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
        QPoint mirror(innerRect.center().x()+(innerRect.center().x()-cursorPoint.x()),cursorPoint.y());
        painter.drawEllipse(mirror, cursorwidth, cursorwidth);
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth-1, cursorwidth-1);
        painter.drawEllipse(mirror, cursorwidth-1, cursorwidth-1);

    } else {*/
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth-1.0, cursorwidth-1.0);
    //}
    painter.end();
    setFullImage(fullSelector);
}
