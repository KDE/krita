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
    bool updateSelf {false};
    bool updateLonesome {false}; // for modal dialogs.
    bool circular {false};
    const KoColorDisplayRendererInterface *displayRenderer {0};
    KisColorSelectorConfiguration acs_config;
    KisSignalCompressor *updateTimer {0};
};

KisVisualColorSelector::KisVisualColorSelector(QWidget *parent)
    : KisColorSelectorInterface(parent)
    , m_d(new Private)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    QVBoxLayout *layout = new QVBoxLayout;
    this->setLayout(layout);

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
    if (m_d->updateSelf == false) {
        m_d->currentcolor = c;
        if (m_d->currentCS != c.colorSpace()) {
            slotsetColorSpace(c.colorSpace());
        }
    }
    updateSelectorElements(QObject::sender());
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

    qDeleteAll(children());
    m_d->widgetlist.clear();
    QLayout *layout = new QHBoxLayout;
    //redraw all the widgets.
    int sizeValue = qMin(width(), height());
    int borderWidth = qMax(sizeValue*0.1, 20.0);

    if (m_d->currentCS->colorChannelCount() == 1) {

        KisVisualColorSelectorShape *bar;

        if (m_d->circular==false) {
            bar = new KisVisualRectangleSelectorShape(this, KisVisualColorSelectorShape::onedimensional,KisVisualColorSelectorShape::Channel, m_d->currentCS, 0, 0,m_d->displayRenderer, borderWidth);
            bar->setMaximumWidth(width()*0.1);
            bar->setMaximumHeight(height());
        }
        else {
            bar = new KisVisualEllipticalSelectorShape(this, KisVisualColorSelectorShape::onedimensional,KisVisualColorSelectorShape::Channel, m_d->currentCS, 0, 0,m_d->displayRenderer, borderWidth, KisVisualEllipticalSelectorShape::borderMirrored);
            layout->setMargin(0);
        }

        connect (bar, SIGNAL(sigNewColor(KoColor)), this, SLOT(updateFromWidgets(KoColor)));
        layout->addWidget(bar);
        m_d->widgetlist.append(bar);
    }
    else if (m_d->currentCS->colorChannelCount() == 3) {
        QRect newrect(0,0, this->geometry().width(), this->geometry().height());

        KisVisualColorSelectorShape::ColorModel modelS = KisVisualColorSelectorShape::HSV;
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
            modelS = KisVisualColorSelectorShape::HSY;
            channel2 = 0;
            channel3 = 1;
            break;
        case KisColorSelectorConfiguration::hsiSH:
            modelS = KisVisualColorSelectorShape::HSI;
            channel2 = 0;
            channel3 = 1;
            break;
        case KisColorSelectorConfiguration::hslSH:
            modelS = KisVisualColorSelectorShape::HSL;
            channel2 = 0;
            channel3 = 1;
            break;
        case KisColorSelectorConfiguration::hsvSH:
            modelS = KisVisualColorSelectorShape::HSV;
            channel2 = 0;
            channel3 = 1;
            break;
        case KisColorSelectorConfiguration::YH:
            modelS = KisVisualColorSelectorShape::HSY;
            channel2 = 0;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::LH:
            modelS = KisVisualColorSelectorShape::HSL;
            channel2 = 0;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::IH:
            modelS = KisVisualColorSelectorShape::HSL;
            channel2 = 0;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::VH:
            modelS = KisVisualColorSelectorShape::HSV;
            channel2 = 0;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::SY:
            modelS = KisVisualColorSelectorShape::HSY;
            channel2 = 1;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::SI:
            modelS = KisVisualColorSelectorShape::HSI;
            channel2 = 1;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::SL:
            modelS = KisVisualColorSelectorShape::HSL;
            channel2 = 1;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::SV:
        case KisColorSelectorConfiguration::SV2:
            modelS = KisVisualColorSelectorShape::HSV;
            channel2 = 1;
            channel3 = 2;
            break;
        default:
            Q_ASSERT_X(false, "", "Invalid acs_config.mainTypeParameter");
        }
        if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Triangle) {
            modelS = KisVisualColorSelectorShape::HSV;
            //Triangle only really works in HSV mode.
        }
        KisVisualColorSelectorShape *bar;
        if (m_d->acs_config.subType == KisColorSelectorConfiguration::Ring) {
            bar = new KisVisualEllipticalSelectorShape(this,
                                                       KisVisualColorSelectorShape::onedimensional,
                                                       modelS,
                                                       m_d->currentCS, channel1, channel1,
                                                       m_d->displayRenderer, borderWidth,KisVisualEllipticalSelectorShape::border);
            bar->resize(sizeValue, sizeValue);
        }
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular == false) {
            bar = new KisVisualRectangleSelectorShape(this,
                                                      KisVisualColorSelectorShape::onedimensional,
                                                      modelS,
                                                      m_d->currentCS, channel1, channel1,
                                                      m_d->displayRenderer, borderWidth);
            bar->setMaximumWidth(borderWidth);
            bar->setMinimumWidth(borderWidth);
            bar->setMinimumHeight(sizeValue);
        }
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular == true) {
            bar = new KisVisualEllipticalSelectorShape(this,
                                                       KisVisualColorSelectorShape::onedimensional,
                                                       modelS,
                                                       m_d->currentCS, channel1, channel1,
                                                       m_d->displayRenderer, borderWidth, KisVisualEllipticalSelectorShape::borderMirrored);
            bar->resize(sizeValue, sizeValue);
        } else {
            // Accessing bar below would crash since it's not initialized.
            // Hopefully this can never happen.
            warnUI << "Invalid subType, cannot initialize KisVisualColorSelectorShape";
            Q_ASSERT_X(false, "", "Invalid subType, cannot initialize KisVisualColorSelectorShape");
            return;
        }

        bar->setColor(m_d->currentcolor);
        m_d->widgetlist.append(bar);

        KisVisualColorSelectorShape *block;
        if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Triangle) {
            block = new KisVisualTriangleSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                       modelS,
                                                       m_d->currentCS, channel2, channel3,
                                                       m_d->displayRenderer);
            block->setGeometry(bar->getSpaceForTriangle(newrect));
        }
        else if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Square) {
            block = new KisVisualRectangleSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                        modelS,
                                                        m_d->currentCS, channel2, channel3,
                                                        m_d->displayRenderer);
            block->setGeometry(bar->getSpaceForSquare(newrect));
        }
        else {
            block = new KisVisualEllipticalSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                         modelS,
                                                         m_d->currentCS, channel2, channel3,
                                                         m_d->displayRenderer);
            block->setGeometry(bar->getSpaceForCircle(newrect));

        }

        block->setColor(m_d->currentcolor);
        connect (bar, SIGNAL(sigNewColor(KoColor)), block, SLOT(setColorFromSibling(KoColor)));
        connect (block, SIGNAL(sigNewColor(KoColor)), SLOT(updateFromWidgets(KoColor)));
        connect (bar, SIGNAL(sigHSXchange()), SLOT(HSXwrangler()));
        connect (block, SIGNAL(sigHSXchange()), SLOT(HSXwrangler()));
        m_d->widgetlist.append(block);
    }
    else if (m_d->currentCS->colorChannelCount() == 4) {
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

void KisVisualColorSelector::updateSelectorElements(QObject *source)
{
    //first lock all elements from sending updates, then update all elements.
    Q_FOREACH (KisVisualColorSelectorShape *shape, m_d->widgetlist) {
        shape->blockSignals(true);
    }

    Q_FOREACH (KisVisualColorSelectorShape *shape, m_d->widgetlist) {
        if (shape!=source) {
            if (m_d->updateSelf) {
                shape->setColorFromSibling(m_d->currentcolor);
            } else {
                shape->setColor(m_d->currentcolor);
            }
        }
    }
    Q_FOREACH (KisVisualColorSelectorShape *shape, m_d->widgetlist) {
        shape->blockSignals(false);
    }

}

void KisVisualColorSelector::updateFromWidgets(KoColor c)
{
    m_d->currentcolor = c;
    m_d->updateSelf = true;
    if (m_d->updateLonesome) {
        slotSetColor(c);
        Q_EMIT sigNewColor(c);

    } else {
        Q_EMIT sigNewColor(c);
    }
}

void KisVisualColorSelector::leaveEvent(QEvent *)
{
    m_d->updateSelf = false;
}

void KisVisualColorSelector::resizeEvent(QResizeEvent *) {
    int sizeValue = qMin(width(), height());
    int borderWidth = qMax(sizeValue*0.1, 20.0);
    QRect newrect(0,0, this->geometry().width(), this->geometry().height());
    if (!m_d->currentCS) {
        slotsetColorSpace(m_d->currentcolor.colorSpace());
    }
    if (m_d->currentCS->colorChannelCount()==3) {

        if (m_d->acs_config.subType == KisColorSelectorConfiguration::Ring) {
            m_d->widgetlist.at(0)->resize(sizeValue,sizeValue);
        }
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular==false) {
            m_d->widgetlist.at(0)->setMaximumWidth(borderWidth);
            m_d->widgetlist.at(0)->setMinimumWidth(borderWidth);
            m_d->widgetlist.at(0)->setMinimumHeight(sizeValue);
            m_d->widgetlist.at(0)->setMaximumHeight(sizeValue);
        }
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular==true) {
            m_d->widgetlist.at(0)->resize(sizeValue,sizeValue);
        }
        m_d->widgetlist.at(0)->setBorderWidth(borderWidth);

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
    Q_FOREACH (KisVisualColorSelectorShape *shape, m_d->widgetlist) {
        shape->update();
    }
}

void KisVisualColorSelector::HSXwrangler()
{
    //qDebug() << this << "HSXWrangler";

    QVector<qreal> currentCoordinates = QVector<qreal>(3);

    QVector <qreal> w1 = m_d->widgetlist.at(0)->getHSX(currentCoordinates, true);
    QVector <qreal> w2 = m_d->widgetlist.at(1)->getHSX(currentCoordinates, true);
    QVector <int> ch(3);

    ch[0] = m_d->widgetlist.at(0)->getChannels().at(0);
    ch[1] = m_d->widgetlist.at(1)->getChannels().at(0);
    ch[2] = m_d->widgetlist.at(1)->getChannels().at(1);

    currentCoordinates[ch[0]] = w1[ch[0]];
    currentCoordinates[ch[1]] = w2[ch[1]];
    currentCoordinates[ch[2]] = w2[ch[2]];

    m_d->widgetlist.at(0)->setHSX(currentCoordinates, true);
    m_d->widgetlist.at(1)->setHSX(currentCoordinates, true);
}
