/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "onionskin_dock.h"
#include <kis_view2.h>
#include <QLabel>
#include <klocale.h>
#include <kis_animation.h>
#include <kis_canvas2.h>
#include <kis_animation_doc.h>
#include <kis_animation_part.h>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QColor>

OnionSkinDock::OnionSkinDock() : QDockWidget(i18n("Onion Skin")), m_canvas(0), m_animation(0)
{
    this->setMinimumSize(300,160);
    QLabel* m_activeLabel = new QLabel(this);
    m_activeLabel->setText("Active: ");
    m_activeCheckBox = new QCheckBox(this);
    m_activeLabel->setGeometry(100,20,50,20);
    m_activeCheckBox->setGeometry(150, 20, 20, 20);
    QLabel* m_frameLabel = new QLabel(this);
    m_frameLabel->setText("Frames");
    m_previousFramesInput = new QSpinBox(this);
    m_nextFramesInput = new QSpinBox(this);
    m_previousFramesInput->setRange(0,9999);
    m_nextFramesInput->setRange(0,9999);
    m_previousFramesInput->setValue(3);
    m_nextFramesInput->setValue(3);
    m_frameLabel->setGeometry(130, 40, 50, 20);
    m_nextFramesInput->setGeometry(200, 40, 50, 20);
    m_previousFramesInput->setGeometry(60,40,50,20);
    QLabel* m_colorLabel = new QLabel(this);
    m_colorLabel->setText("Colors");
    m_previousFramesColor = new KColorButton(this);
    m_nextFramesColor = new KColorButton(this);
    m_previousFramesColor->setColor(QColor(Qt::red));
    m_nextFramesColor->setColor(QColor(Qt::blue));
    m_colorLabel->setGeometry(130, 60, 50, 20);
    m_previousFramesColor->setGeometry(60, 60,50, 20);
    m_nextFramesColor->setGeometry(200, 60, 50, 20);
    QLabel* m_opacityLabel = new QLabel(this);
    m_opacityLabel->setText("Opacity");
    m_opacityLabel->setGeometry(130, 80, 50, 20);

    m_previousOpacitySelectorView = new KisOpacitySelectorView(this);
    m_previousOpacitySelectorView->setNumberOfFrames(m_previousFramesInput->value());
    m_previousOpacitySelectorView->setGeometry(20,80,105,75);
    m_nextOpacitySelectorView = new KisOpacitySelectorView(this);
    m_nextOpacitySelectorView->setNumberOfFrames(m_nextFramesInput->value());
    m_nextOpacitySelectorView->setGeometry(180, 80, 105, 75);

    connect(m_previousFramesInput, SIGNAL(valueChanged(int)), m_previousOpacitySelectorView, SLOT(setNumberOfFrames(int)));
    connect(m_nextFramesInput, SIGNAL(valueChanged(int)), m_nextOpacitySelectorView, SLOT(setNumberOfFrames(int)));
}

void OnionSkinDock::setCanvas(KoCanvasBase *canvas){
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    if(m_canvas && m_canvas->view() && m_canvas->view()->document() && m_canvas->view()->document()->documentPart()){
        m_animation = dynamic_cast<KisAnimationPart*>(m_canvas->view()->document()->documentPart())->animation();
        if(m_animation){

        }
    }
}
