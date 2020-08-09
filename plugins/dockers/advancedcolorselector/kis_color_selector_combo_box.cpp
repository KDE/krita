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
#include <QApplication>

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

        //dbgKrita<<"Created list";

        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Triangle, KisColorSelectorConfiguration::Ring, KisColorSelectorConfiguration::SL , KisColorSelectorConfiguration::H), this), 0,0);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square,   KisColorSelectorConfiguration::Ring, KisColorSelectorConfiguration::SV , KisColorSelectorConfiguration::H), this), 0,1);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square,   KisColorSelectorConfiguration::Ring, KisColorSelectorConfiguration::SV2, KisColorSelectorConfiguration::H), this), 0,2);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Wheel, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::VH, KisColorSelectorConfiguration::hsvS), this), 0,3);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Wheel, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::hsvSH, KisColorSelectorConfiguration::V), this), 0,4);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::SV2, KisColorSelectorConfiguration::H), this), 1,0);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::SV, KisColorSelectorConfiguration::H), this), 1,1);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::VH, KisColorSelectorConfiguration::hsvS), this), 1,2);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::hsvSH, KisColorSelectorConfiguration::V), this), 1,3);



        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square,   KisColorSelectorConfiguration::Ring, KisColorSelectorConfiguration::SL , KisColorSelectorConfiguration::H), this), 0,1);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Wheel, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::LH, KisColorSelectorConfiguration::hslS), this), 0,2);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Wheel, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::hslSH, KisColorSelectorConfiguration::L), this), 0,3);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::SL, KisColorSelectorConfiguration::H), this), 1,0);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::LH, KisColorSelectorConfiguration::hslS), this), 1,1);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::hslSH, KisColorSelectorConfiguration::L), this), 1,2);


        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square,   	KisColorSelectorConfiguration::Ring, KisColorSelectorConfiguration::SI , KisColorSelectorConfiguration::H), this), 0,1);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Wheel, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::IH, KisColorSelectorConfiguration::hsiS), this), 0,2);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Wheel, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::hsiSH, KisColorSelectorConfiguration::I), this), 0,3);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::SI, KisColorSelectorConfiguration::H), this), 1,0);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::IH, KisColorSelectorConfiguration::hsiS), this), 1,1);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::hsiSH, KisColorSelectorConfiguration::I), this), 1,2);

        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Ring, KisColorSelectorConfiguration::SY, KisColorSelectorConfiguration::Hluma), this), 0,1);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Wheel, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::YH, KisColorSelectorConfiguration::hsyS), this), 0,2);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Wheel, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::hsySH, KisColorSelectorConfiguration::Y), this), 0,3);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::SY, KisColorSelectorConfiguration::Hluma), this), 1,0);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::YH, KisColorSelectorConfiguration::hsyS), this), 1,1);
        layout->addWidget(new KisColorSelector(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::hsySH, KisColorSelectorConfiguration::Y), this), 1,2);

        setList(0);

        for(int i=0; i<this->layout()->count(); i++) {
            KisColorSelector* item = dynamic_cast<KisColorSelector*>(this->layout()->itemAt(i)->widget());
            Q_ASSERT(item);
            if(item!=0) {
                item->setMaximumSize(selectorSize, selectorSize);
                item->setMinimumSize(selectorSize, selectorSize);
                item->setMouseTracking(true);
                item->setEnabled(false);
                item->setColor(KoColor(QColor(255,0,0), item->colorSpace()));
                item->setDisplayBlip(false);
            }
        }


    }


    void setList(int model){
            for(int i=1; i<layout()->count(); i++) {
        layout()->itemAt(i)->widget()->hide();
        }

        if  (model==0){
            for(int i=1; i<9; i++) {
            layout()->itemAt(i)->widget()->show();
            }
        }

        if  (model==1){
            for(int i=9; i<15; i++) {
            layout()->itemAt(i)->widget()->show();
            }
        }

        if  (model==2){
            for(int i=15; i<21; i++) {
            layout()->itemAt(i)->widget()->show();
            }
        }

        if  (model==3){
            for(int i=21; i<layout()->count(); i++) {
            layout()->itemAt(i)->widget()->show();
            }
        }
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.fillRect(0,0,width(), height(), QColor(128,128,128));
        painter.fillRect(highlightArea, palette().highlight());
    }

    void mouseMoveEvent(QMouseEvent * e) override
    {
        if(rect().contains(e->pos())) {
            for(int i=0; i<layout()->count(); i++) {

                KisColorSelector* item = dynamic_cast<KisColorSelector*>(layout()->itemAt(i)->widget());
                Q_ASSERT(item);


                if(layout()->itemAt(i)->widget()->isVisible()==true && item->geometry().adjusted(-spacing/2, -spacing/2, spacing/2, spacing/2).contains(e->pos())) {
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

    void mousePressEvent(QMouseEvent* e) override
    {
        if(rect().contains(e->pos())) {
            KisColorSelectorComboBox* parent = dynamic_cast<KisColorSelectorComboBox*>(this->parent());
            Q_ASSERT(parent);
            parent->setConfiguration(m_lastActiveConfiguration);
            //setList(parent->m_model);
        }
        //dbgKrita<<"mousepress";
        hide();
        e->accept();
    }
    KisColorSelectorConfiguration m_lastActiveConfiguration;

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
    m_currentSelector.setColor(KoColor(QColor(255,0,0), m_currentSelector.colorSpace()));

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

KisColorSelectorConfiguration KisColorSelectorComboBox::configuration() const
{
    return m_configuration;
}

void KisColorSelectorComboBox::paintEvent(QPaintEvent *e)
{
    QComboBox::paintEvent(e);
}

void KisColorSelectorComboBox::setConfiguration(KisColorSelectorConfiguration conf)
{
    m_configuration=conf;
    m_currentSelector.setConfiguration(conf);
    m_currentSelector.setColor(KoColor(QColor(255,0,0), m_currentSelector.colorSpace()));
    update();
}

void KisColorSelectorComboBox::setList(int model) {

    m_private->setList(model);
}
