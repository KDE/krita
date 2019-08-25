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
#include "KisVisualColorSelector.h"

#include <QColor>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QVector>
#include <QVector3D>
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

#include "KisVisualColorSelectorShape.h"
#include "KisVisualRectangleSelectorShape.h"
#include "KisVisualTriangleSelectorShape.h"
#include "KisVisualEllipticalSelectorShape.h"

struct KisVisualColorSelector::Private
{
    KoColor currentcolor;
    const KoColorSpace *currentCS {0};
    QList<KisVisualColorSelectorShape*> widgetlist;
    bool updateLonesome {false}; // currently redundant; remove?
    bool circular {false};
    bool exposureSupported = false;
    bool isRGBA = false;
    int displayPosition[4]; // map channel index to storage index for display
    int colorChannelCount;
    QVector4D channelValues;
    ColorModel model;
    const KoColorDisplayRendererInterface *displayRenderer {0};
    KisColorSelectorConfiguration acs_config;
    KisSignalCompressor *updateTimer {0};
};

KisVisualColorSelector::KisVisualColorSelector(QWidget *parent)
    : KisColorSelectorInterface(parent)
    , m_d(new Private)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    m_d->acs_config = KisColorSelectorConfiguration::fromString(cfg.readEntry("colorSelectorConfiguration", KisColorSelectorConfiguration().toString()));
    m_d->updateTimer = new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE);
    connect(m_d->updateTimer, SIGNAL(timeout()), SLOT(slotRebuildSelectors()), Qt::UniqueConnection);
}

KisVisualColorSelector::~KisVisualColorSelector()
{
    delete m_d->updateTimer;
}

void KisVisualColorSelector::slotSetColor(const KoColor &c)
{
    m_d->currentcolor = c;
    if (m_d->currentCS != c.colorSpace()) {
        slotsetColorSpace(c.colorSpace());
    }
    else {
        m_d->channelValues = convertKoColorToShapeCoordinates(m_d->currentcolor);
        Q_FOREACH (KisVisualColorSelectorShape *shape, m_d->widgetlist) {
            shape->setChannelValues(m_d->channelValues, true);
        }
    }
}

void KisVisualColorSelector::slotsetColorSpace(const KoColorSpace *cs)
{
    if (m_d->currentCS != cs) {
        m_d->currentCS = cs;
        slotRebuildSelectors();
    }
}

void KisVisualColorSelector::setConfig(bool forceCircular, bool forceSelfUpdate)
{
    m_d->circular = forceCircular;
    m_d->updateLonesome = forceSelfUpdate;
}

KoColor KisVisualColorSelector::getCurrentColor() const
{
    return m_d->currentcolor;
}

QVector4D KisVisualColorSelector::getChannelValues() const
{
    return m_d->channelValues;
}

KoColor KisVisualColorSelector::convertShapeCoordsToKoColor(const QVector4D &coordinates) const
{
    KoColor c(m_d->currentCS);
    QVector4D baseValues(coordinates);
    QVector <float> channelValues(c.colorSpace()->channelCount());
    channelValues.fill(1.0);

    if (m_d->model != ColorModel::Channel && m_d->isRGBA == true) {

        if (m_d->model == ColorModel::HSV) {
            HSVToRGB(coordinates.x()*360, coordinates.y(), coordinates.z(), &baseValues[0], &baseValues[1], &baseValues[2]);
        }
        else if (m_d->model == ColorModel::HSL) {
            HSLToRGB(coordinates.x()*360, coordinates.y(), coordinates.z(), &baseValues[0], &baseValues[1], &baseValues[2]);
        }
        else if (m_d->model == ColorModel::HSI) {
            // why suddenly qreal?
            qreal temp[3];
            HSIToRGB(coordinates.x(), coordinates.y(), coordinates.z(), &temp[0], &temp[1], &temp[2]);
            baseValues.setX(temp[0]);
            baseValues.setY(temp[1]);
            baseValues.setZ(temp[2]);
        }
        else /*if (m_d->model == ColorModel::HSY)*/ {
            QVector <qreal> luma= m_d->currentCS->lumaCoefficients();
            qreal temp[3];
            HSYToRGB(coordinates.x(), coordinates.y(), coordinates.z(), &temp[0], &temp[1], &temp[2],
                    luma[0], luma[1], luma[2]);
            baseValues.setX(temp[0]);
            baseValues.setY(temp[1]);
            baseValues.setZ(temp[2]);
        }
    }

    for (int i=0; i<m_d->colorChannelCount; i++) {
        // TODO: proper exposure control
        channelValues[m_d->displayPosition[i]] = baseValues[i] /* *(maxvalue[i]) */;
    }

    c.colorSpace()->fromNormalisedChannelsValue(c.data(), channelValues);

    return c;

}

QVector4D KisVisualColorSelector::convertKoColorToShapeCoordinates(KoColor c) const
{
    if (c.colorSpace() != m_d->currentCS) {
        c.convertTo(m_d->currentCS);
    }
    QVector <float> channelValues (c.colorSpace()->channelCount());
    channelValues.fill(1.0);
    m_d->currentCS->normalisedChannelsValue(c.data(), channelValues);
    QVector4D channelValuesDisplay(0, 0, 0, 0), coordinates(0, 0, 0, 0);
    // TODO: proper exposure control
    // TODO: L*a*b is apparently not [0, 1]^3 as "normalized" values, needs extra transform (old bug)
    for (int i =0; i<m_d->colorChannelCount; i++) {
        channelValuesDisplay[i] = qBound(0.f, channelValues[m_d->displayPosition[i]], 1.f);
    }
    if (m_d->model != ColorModel::Channel && m_d->isRGBA == true) {
        if (m_d->isRGBA == true) {
            if (m_d->model == ColorModel::HSV){
                QVector3D hsv;
                // TODO: handle undefined hue case (returns -1)
                RGBToHSV(channelValuesDisplay[0], channelValuesDisplay[1], channelValuesDisplay[2], &hsv[0], &hsv[1], &hsv[2]);
                hsv[0] /= 360;
                coordinates = QVector4D(hsv, 0.f);
            } else if (m_d->model == ColorModel::HSL) {
                QVector3D hsl;
                RGBToHSL(channelValuesDisplay[0], channelValuesDisplay[1], channelValuesDisplay[2], &hsl[0], &hsl[1], &hsl[2]);
                hsl[0] /= 360;
                coordinates = QVector4D(hsl, 0.f);
            } else if (m_d->model == ColorModel::HSI) {
                qreal hsi[3];
                RGBToHSI(channelValuesDisplay[0], channelValuesDisplay[1], channelValuesDisplay[2], &hsi[0], &hsi[1], &hsi[2]);
                coordinates = QVector4D(hsi[0], hsi[1], hsi[2], 0.f);
            } else if (m_d->model == ColorModel::HSY) {
                QVector <qreal> luma = m_d->currentCS->lumaCoefficients();
                qreal hsy[3];
                RGBToHSY(channelValuesDisplay[0], channelValuesDisplay[1], channelValuesDisplay[2], &hsy[0], &hsy[1], &hsy[2], luma[0], luma[1], luma[2]);
                coordinates = QVector4D(hsy[0], hsy[1], hsy[2], 0.f);
            }
        }
    } else {
        for (int i=0; i<4; i++)
        {
            coordinates[i] = qBound(0.f, channelValuesDisplay[i], 1.f);
        }
    }
    return coordinates;
}

void KisVisualColorSelector::configurationChanged()
{
    if (m_d->updateTimer) {
        m_d->updateTimer->start();
    }
}

void KisVisualColorSelector::slotRebuildSelectors()
{
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    m_d->acs_config = KisColorSelectorConfiguration::fromString(cfg.readEntry("colorSelectorConfiguration", KisColorSelectorConfiguration().toString()));

    QList<KoChannelInfo *> channelList = m_d->currentCS->channels();
    int cCount = 0;
    Q_FOREACH(const KoChannelInfo *channel, channelList)
    {
        if (channel->channelType() != KoChannelInfo::ALPHA)
        {
            m_d->displayPosition[cCount] = channel->displayPosition();
            ++cCount;
        }
    }
    Q_ASSERT_X(cCount < 5, "", "unsupported channel count!");
    m_d->colorChannelCount = cCount;

    // TODO: The following is done because the IDs are actually strings. Ideally, in the future, we
    // refactor everything so that the IDs are actually proper enums or something faster.
    if (m_d->displayRenderer
            && (m_d->currentCS->colorDepthId() == Float16BitsColorDepthID
                || m_d->currentCS->colorDepthId() == Float32BitsColorDepthID
                || m_d->currentCS->colorDepthId() == Float64BitsColorDepthID)
            && m_d->currentCS->colorModelId() != LABAColorModelID
            && m_d->currentCS->colorModelId() != CMYKAColorModelID) {
        m_d->exposureSupported = true;
    } else {
        m_d->exposureSupported = false;
    }
    if (m_d->currentCS->colorModelId() == RGBAColorModelID) {
        m_d->isRGBA = true;
    } else {
        m_d->isRGBA = false;
    }

    qDeleteAll(children());
    m_d->widgetlist.clear();
    // TODO: Layout only used for monochrome selector currently, but always present
    QLayout *layout = new QHBoxLayout;
    //recreate all the widgets.
    m_d->model = KisVisualColorSelector::Channel;

    if (m_d->currentCS->colorChannelCount() == 1) {

        KisVisualColorSelectorShape *bar;

        if (m_d->circular==false) {
            bar = new KisVisualRectangleSelectorShape(this, KisVisualColorSelectorShape::onedimensional, m_d->currentCS, 0, 0,m_d->displayRenderer, 20);
            bar->setMaximumWidth(width()*0.1);
            bar->setMaximumHeight(height());
        }
        else {
            bar = new KisVisualEllipticalSelectorShape(this, KisVisualColorSelectorShape::onedimensional, m_d->currentCS, 0, 0,m_d->displayRenderer, 20, KisVisualEllipticalSelectorShape::borderMirrored);
            layout->setMargin(0);
        }

        connect(bar, SIGNAL(sigCursorMoved(QPointF)), SLOT(slotCursorMoved(QPointF)));
        layout->addWidget(bar);
        m_d->widgetlist.append(bar);
    }
    else if (m_d->currentCS->colorChannelCount() == 3) {
        KisVisualColorSelector::ColorModel modelS = KisVisualColorSelector::HSV;
        int channel1 = 0;
        int channel2 = 1;
        int channel3 = 2;

        switch(m_d->acs_config.subTypeParameter)
        {
        case KisColorSelectorConfiguration::H:
            channel1 = 0;
            break;
        case KisColorSelectorConfiguration::hsyS:
        case KisColorSelectorConfiguration::hsiS:
        case KisColorSelectorConfiguration::hslS:
        case KisColorSelectorConfiguration::hsvS:
            channel1 = 1;
            break;
        case KisColorSelectorConfiguration::V:
        case KisColorSelectorConfiguration::L:
        case KisColorSelectorConfiguration::I:
        case KisColorSelectorConfiguration::Y:
            channel1 = 2;
            break;
        default:
            Q_ASSERT_X(false, "", "Invalid acs_config.subTypeParameter");
        }

        switch(m_d->acs_config.mainTypeParameter)
        {
        case KisColorSelectorConfiguration::hsySH:
            modelS = KisVisualColorSelector::HSY;
            channel2 = 0;
            channel3 = 1;
            break;
        case KisColorSelectorConfiguration::hsiSH:
            modelS = KisVisualColorSelector::HSI;
            channel2 = 0;
            channel3 = 1;
            break;
        case KisColorSelectorConfiguration::hslSH:
            modelS = KisVisualColorSelector::HSL;
            channel2 = 0;
            channel3 = 1;
            break;
        case KisColorSelectorConfiguration::hsvSH:
            modelS = KisVisualColorSelector::HSV;
            channel2 = 0;
            channel3 = 1;
            break;
        case KisColorSelectorConfiguration::YH:
            modelS = KisVisualColorSelector::HSY;
            channel2 = 0;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::LH:
            modelS = KisVisualColorSelector::HSL;
            channel2 = 0;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::IH:
            modelS = KisVisualColorSelector::HSL;
            channel2 = 0;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::VH:
            modelS = KisVisualColorSelector::HSV;
            channel2 = 0;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::SY:
            modelS = KisVisualColorSelector::HSY;
            channel2 = 1;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::SI:
            modelS = KisVisualColorSelector::HSI;
            channel2 = 1;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::SL:
            modelS = KisVisualColorSelector::HSL;
            channel2 = 1;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::SV:
        case KisColorSelectorConfiguration::SV2:
            modelS = KisVisualColorSelector::HSV;
            channel2 = 1;
            channel3 = 2;
            break;
        default:
            Q_ASSERT_X(false, "", "Invalid acs_config.mainTypeParameter");
        }
        if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Triangle) {
            modelS = KisVisualColorSelector::HSV;
            //Triangle only really works in HSV mode.
        }

        m_d->model = modelS;
        KisVisualColorSelectorShape *bar;
        if (m_d->acs_config.subType == KisColorSelectorConfiguration::Ring) {
            bar = new KisVisualEllipticalSelectorShape(this,
                                                       KisVisualColorSelectorShape::onedimensional,
                                                       m_d->currentCS, channel1, channel1,
                                                       m_d->displayRenderer, 20,KisVisualEllipticalSelectorShape::border);
        }
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular == false) {
            bar = new KisVisualRectangleSelectorShape(this,
                                                      KisVisualColorSelectorShape::onedimensional,
                                                      m_d->currentCS, channel1, channel1,
                                                      m_d->displayRenderer, 20);
        }
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular == true) {
            bar = new KisVisualEllipticalSelectorShape(this,
                                                       KisVisualColorSelectorShape::onedimensional,
                                                       m_d->currentCS, channel1, channel1,
                                                       m_d->displayRenderer, 20, KisVisualEllipticalSelectorShape::borderMirrored);
        } else {
            // Accessing bar below would crash since it's not initialized.
            // Hopefully this can never happen.
            warnUI << "Invalid subType, cannot initialize KisVisualColorSelectorShape";
            Q_ASSERT_X(false, "", "Invalid subType, cannot initialize KisVisualColorSelectorShape");
            return;
        }

        m_d->widgetlist.append(bar);

        KisVisualColorSelectorShape *block;
        if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Triangle) {
            block = new KisVisualTriangleSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                       m_d->currentCS, channel2, channel3,
                                                       m_d->displayRenderer);
        }
        else if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Square) {
            block = new KisVisualRectangleSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                        m_d->currentCS, channel2, channel3,
                                                        m_d->displayRenderer);
        }
        else {
            block = new KisVisualEllipticalSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                         m_d->currentCS, channel2, channel3,
                                                         m_d->displayRenderer);
        }

        connect(bar, SIGNAL(sigCursorMoved(QPointF)), SLOT(slotCursorMoved(QPointF)));
        connect(block, SIGNAL(sigCursorMoved(QPointF)), SLOT(slotCursorMoved(QPointF)));
        m_d->widgetlist.append(block);
    }
    else if (m_d->currentCS->colorChannelCount() == 4) {
        KisVisualRectangleSelectorShape *block =  new KisVisualRectangleSelectorShape(this, KisVisualRectangleSelectorShape::twodimensional, m_d->currentCS, 0, 1);
        KisVisualRectangleSelectorShape *block2 =  new KisVisualRectangleSelectorShape(this, KisVisualRectangleSelectorShape::twodimensional, m_d->currentCS, 2, 3);
        connect(block, SIGNAL(sigCursorMoved(QPointF)), SLOT(slotCursorMoved(QPointF)));
        connect(block2, SIGNAL(sigCursorMoved(QPointF)), SLOT(slotCursorMoved(QPointF)));
        m_d->widgetlist.append(block);
        m_d->widgetlist.append(block2);
    }

    this->setLayout(layout);
    // make sure we call "our" resize function
    KisVisualColorSelector::resizeEvent(0);

    // finally recalculate channel values and update widgets
    m_d->channelValues = convertKoColorToShapeCoordinates(m_d->currentcolor);
    Q_FOREACH (KisVisualColorSelectorShape *shape, m_d->widgetlist) {
        shape->setChannelValues(m_d->channelValues, true);
        // if this widget is currently visible, new children are hidden by default
        shape->show();
    }
}

void KisVisualColorSelector::setDisplayRenderer (const KoColorDisplayRendererInterface *displayRenderer) {
    m_d->displayRenderer = displayRenderer;
    if (m_d->widgetlist.size()>0) {
        Q_FOREACH (KisVisualColorSelectorShape *shape, m_d->widgetlist) {
            shape->setDisplayRenderer(displayRenderer);
        }
    }
}

void KisVisualColorSelector::slotCursorMoved(QPointF pos)
{
    const KisVisualColorSelectorShape *shape = qobject_cast<KisVisualColorSelectorShape *>(sender());
    Q_ASSERT(shape);
    QVector<int> channels = shape->getChannels();
    m_d->channelValues[channels.at(0)] = pos.x();
    if (shape->getDimensions() == KisVisualColorSelectorShape::twodimensional)
    {
        m_d->channelValues[channels.at(1)] = pos.y();
    }
    KoColor newColor = convertShapeCoordsToKoColor(m_d->channelValues);
    if (newColor != m_d->currentcolor)
    {
        m_d->currentcolor = newColor;

        Q_FOREACH (KisVisualColorSelectorShape *widget, m_d->widgetlist) {
            if (widget != shape){
                widget->setChannelValues(m_d->channelValues, false);
            }
        }
        emit sigNewColor(m_d->currentcolor);
    }
}

void KisVisualColorSelector::resizeEvent(QResizeEvent *) {
    int sizeValue = qMin(width(), height());
    int borderWidth = qMax(sizeValue*0.1, 20.0);
    QRect newrect(0,0, this->geometry().width(), this->geometry().height());
    if (!m_d->currentCS) {
        slotsetColorSpace(m_d->currentcolor.colorSpace());
    }
    if (m_d->currentCS->colorChannelCount()==3) {
        // set border width first, else the resized painting may have happened already, and we'd have to re-render
        m_d->widgetlist.at(0)->setBorderWidth(borderWidth);
        if (m_d->acs_config.subType == KisColorSelectorConfiguration::Ring) {
            m_d->widgetlist.at(0)->resize(sizeValue,sizeValue);
        }
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular==false) {
            m_d->widgetlist.at(0)->resize(borderWidth, sizeValue);
        }
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular==true) {
            m_d->widgetlist.at(0)->resize(sizeValue,sizeValue);
        }

        if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Triangle) {
            m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForTriangle(newrect));
        }
        else if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Square) {
            m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForSquare(newrect));
        }
        else if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Wheel) {
            m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForCircle(newrect));
        }
    }
    else if (m_d->currentCS->colorChannelCount() == 4) {
        int sizeBlock = qMin(width()/2 - 8, height());
        m_d->widgetlist.at(0)->setGeometry(0, 0, sizeBlock, sizeBlock);
        m_d->widgetlist.at(1)->setGeometry(sizeBlock + 8, 0, sizeBlock, sizeBlock);
    }
}
