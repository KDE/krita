/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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
 */

#include "kis_color_selector_combo_box.h"
#include <QGridLayout>
#include <QPainter>

#include "kis_color_selector.h"
#include "kis_canvas2.h"

class KisColorSelectorComboBoxPrivate : public QWidget {
public:
    int spacing;
    int selectorSize;
    QRect highlightArea;

    KisColorSelectorComboBoxPrivate(QWidget* parent) :
            QWidget(parent, Qt::Popup),
            spacing(20),
            selectorSize(100),
            highlightArea(-1,-1,0,0)
    {
        setMouseTracking(true);

        QGridLayout* layout = new QGridLayout(this);
        layout->setSpacing(spacing);

        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Triangle, KisColorSelector::Ring, KisColorSelector::SL , KisColorSelector::H), this), 0,0);
        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Square,   KisColorSelector::Ring, KisColorSelector::SL , KisColorSelector::H), this), 0,1);
        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Square,   KisColorSelector::Ring, KisColorSelector::SV , KisColorSelector::H), this), 0,2);
        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Square,   KisColorSelector::Ring, KisColorSelector::SV2, KisColorSelector::H), this), 0,3);
        
        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::SV, KisColorSelector::H), this), 1,0);
        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::SL, KisColorSelector::H), this), 1,1);
        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::SV2, KisColorSelector::H), this), 1,2);

        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::VH, KisColorSelector::hsvS), this), 2,0);
        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::LH, KisColorSelector::hslS), this), 2,1);
        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::hsvSH, KisColorSelector::V), this), 2,2);
        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::hslSH, KisColorSelector::L), this), 2,3);

        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Wheel, KisColorSelector::Slider, KisColorSelector::VH, KisColorSelector::hsvS), this), 3,0);
        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Wheel, KisColorSelector::Slider, KisColorSelector::LH, KisColorSelector::hslS), this), 3,1);
        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Wheel, KisColorSelector::Slider, KisColorSelector::hsvSH, KisColorSelector::V), this), 3,2);
        layout->addWidget(new KisColorSelector(KisColorSelector::Configuration(KisColorSelector::Wheel, KisColorSelector::Slider, KisColorSelector::hslSH, KisColorSelector::L), this), 3,3);


        for(int i=0; i<this->layout()->count(); i++) {
            KisColorSelector* item = dynamic_cast<KisColorSelector*>(this->layout()->itemAt(i)->widget());
            Q_ASSERT(item);
            if(item!=0) {
                item->setMaximumSize(selectorSize, selectorSize);
                item->setMinimumSize(selectorSize, selectorSize);
                item->setMouseTracking(true);
                item->setEnabled(false);
                item->setColor(QColor(255,0,0));
                item->setDisplayBlip(false);
            }
        }
    }

protected:
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        painter.fillRect(0,0,width(), height(), QColor(128,128,128));
        painter.fillRect(highlightArea, palette().highlight());
    }

    void mouseMoveEvent(QMouseEvent * e)
    {
        if(rect().contains(e->pos())) {
            for(int i=0; i<layout()->count(); i++) {
                KisColorSelector* item = dynamic_cast<KisColorSelector*>(layout()->itemAt(i)->widget());
                Q_ASSERT(item);

                if(item->geometry().adjusted(-spacing/2, -spacing/2, spacing/2, spacing/2).contains(e->pos())) {
                    QRect oldArea=highlightArea;
                    highlightArea=item->geometry().adjusted(-spacing/2, -spacing/2, spacing/2, spacing/2);
                    m_lastActiveConfiguration=item->configuration();
                    update(highlightArea);
                    update(oldArea);
                }
            }
        }
        else {
            highlightArea.setRect(-1,-1,0,0);
        }
    }

    void mousePressEvent(QMouseEvent* e)
    {
        if(rect().contains(e->pos())) {
            KisColorSelectorComboBox* parent = dynamic_cast<KisColorSelectorComboBox*>(this->parent());
            Q_ASSERT(parent);
            parent->setConfiguration(m_lastActiveConfiguration);
        }
        hide();
        e->accept();
    }
    KisColorSelector::Configuration m_lastActiveConfiguration;
};

KisColorSelectorComboBox::KisColorSelectorComboBox(QWidget* parent) :
        QComboBox(parent),
        m_private(new KisColorSelectorComboBoxPrivate(this)),
        m_currentSelector(this)
{
    QLayout* layout = new QGridLayout(this);
    layout->addWidget(&m_currentSelector);
    m_currentSelector.setEnabled(false);
    m_currentSelector.setDisplayBlip(false);
    m_currentSelector.setColor(QColor(255,0,0));

    // 30 pixels for the arrow of the combobox
    setMinimumSize(m_private->selectorSize+m_private->spacing+30,m_private->selectorSize+m_private->spacing);
    m_currentSelector.setMaximumSize(m_private->selectorSize, m_private->selectorSize);
}

KisColorSelectorComboBox::~KisColorSelectorComboBox()
{
}

void KisColorSelectorComboBox::hidePopup()
{
    QComboBox::hidePopup();
    m_private->hide();
}

void KisColorSelectorComboBox::showPopup()
{
    // only show if this is not the popup
    QComboBox::showPopup();
    m_private->move(mapToGlobal(QPoint(0,0)));
    m_private->show();
}

void KisColorSelectorComboBox::setColorSpace(const KoColorSpace *colorSpace)
{
    //this is not the popup, but we should set the canvas for all popup selectors
    for(int i=0; i<m_private->layout()->count(); i++) {
        KisColorSelector* item = dynamic_cast<KisColorSelector*>(m_private->layout()->itemAt(i)->widget());
        Q_ASSERT(item);
        if(item!=0) {
            item->setColorSpace(colorSpace);
        }
    }
    m_currentSelector.setColorSpace(colorSpace);
    update();
}

KisColorSelector::Configuration KisColorSelectorComboBox::configuration() const
{
    return m_configuration;
}

void KisColorSelectorComboBox::paintEvent(QPaintEvent *e)
{
    QComboBox::paintEvent(e);
}

void KisColorSelectorComboBox::setConfiguration(KisColorSelector::Configuration conf)
{
    m_configuration=conf;
    m_currentSelector.setConfiguration(conf);
    m_currentSelector.setColor(QColor(255,0,0));
    update();
}
