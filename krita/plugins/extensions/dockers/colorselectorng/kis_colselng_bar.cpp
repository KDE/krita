/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

#include "kis_colselng_bar.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <KIcon>

KisColSelNgBar::KisColSelNgBar(QWidget *parent) :
    QWidget(parent)
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    setMinimumSize(40, 24);
    setMaximumSize(40, 24);

//    QPushButton* colorSelButton = new QPushButton(this);
//    QPushButton* myPaintSelButton = new QPushButton(this);
//    QPushButton* patchesButton = new QPushButton(this);
    QPushButton* pipetteButton = new QPushButton(this);
    QPushButton* settingsButton = new QPushButton(this);

//    colorSelButton->setIcon(KIcon("kis_colselng_color_triangle"));
//    myPaintSelButton->setIcon(KIcon("kis_colselng_my_paint_shade_selector"));
//    patchesButton->setIcon(KIcon("kis_colselng_color_patches"));
    pipetteButton->setIcon(KIcon("krita_tool_color_picker"));
    pipetteButton->setFlat(true);
//    pipetteButton->setAutoFillBackground(true);
    settingsButton->setIcon(KIcon("configure"));
    settingsButton->setFlat(true);
//    settingsButton->setAutoFillBackground(true);

//    colorSelButton->setMaximumWidth(24);
//    myPaintSelButton->setMaximumWidth(24);
//    patchesButton->setMaximumWidth(24);
    pipetteButton->setMaximumWidth(24);
    settingsButton->setMaximumWidth(24);

    QHBoxLayout* layout = new QHBoxLayout(this);
//    layout->addWidget(colorSelButton);
//    layout->addWidget(myPaintSelButton);
//    layout->addWidget(patchesButton);
    layout->addStretch(1);
    layout->addWidget(pipetteButton);
    layout->addWidget(settingsButton);

    connect(settingsButton, SIGNAL(clicked()), this, SIGNAL(openSettings()));

    m_settingsButton = settingsButton;
    m_pipetteButton = pipetteButton;
}
