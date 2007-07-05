/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QButtonGroup>
#include <QColor>
#include <QGridLayout>
#include <QPalette>
#include <QSizePolicy>
#include <QToolButton>
#include <QWidget>

#include <KoCanvasResourceProvider.h>
#include <KoToolProxy.h>

#include "kis_canvas2.h"
#include "kis_resource_provider.h"
#include "kis_view2.h"

#include "kis_painterlymixer.h"
#include "mixertool.h"

KisPainterlyMixer::KisPainterlyMixer(QWidget *parent, KisView2 *view)
    : QWidget(parent), m_view(view), m_resources(view->canvasBase()->resourceProvider())
{
    setupUi(this);

    m_canvas->setDevice(m_view->image()->colorSpace());
    initTool();
    initSpots();
}

KisPainterlyMixer::~KisPainterlyMixer()
{
    if (m_tool)
        delete m_tool;
}

void KisPainterlyMixer::initTool()
{
    m_tool = new MixerTool(m_canvas, m_canvas->device(), m_resources);

    m_canvas->setToolProxy(new KoToolProxy(m_canvas));
    m_canvas->toolProxy()->setActiveTool(m_tool);
}


#define ROWS 2
#define COLS 4
// TODO User should be able to add/remove spots. Should he? ... Ok, perhaps not...

void KisPainterlyMixer::initSpots()
{
    int row, col;
    QGridLayout *l = new QGridLayout(m_spotsFrame);

    m_bgColors = new QButtonGroup(m_spotsFrame);
    loadColors();

    l->setSpacing(5);
    for (row = 0; row < ROWS; row++) {
        for (col = 0; col < COLS; col++) {
            int index = row*COLS + col;
            QToolButton *curr = new QToolButton(m_spotsFrame);
            l->addWidget(curr, row, col);

            setupButton(curr, index);

            m_bgColors->addButton(curr, index);
        }
    }

    l->setColumnStretch(col++, 1);

    m_bWet = new QToolButton(m_spotsFrame);
    m_bWet->setText("W"); // TODO Icon for the "Wet spot"
    m_bWet->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_bDry = new QToolButton(m_spotsFrame);
    m_bDry->setText("D"); // TODO Icon for the "Dry spot"
    m_bDry->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    l->addWidget(m_bWet, 0, col++, ROWS, 1);
    l->addWidget(m_bDry, 0, col, ROWS, 1);

    connect(m_bgColors, SIGNAL(buttonClicked(int)), this, SLOT(changeColor(int)));
}

void KisPainterlyMixer::setupButton(QToolButton *button, int index)
{
//     button->setFixedSize(15, 15);
    button->setPalette(QPalette(m_vColors[index].rgba(), m_vColors[index].rgba()));
    button->setAutoFillBackground(true);
}

void KisPainterlyMixer::loadColors()
{
// TODO We need to handle save/load of user-defined colors in the spots.
    m_vColors.append(QColor(0xFFFF0000)); // Red
    m_vColors.append(QColor(0xFF00FF00)); // Green
    m_vColors.append(QColor(0xFF0000FF)); // Blue
    m_vColors.append(QColor(0xFF0FA311)); // Whatever :)
    m_vColors.append(QColor(0xFFFFFF00)); // Yellow
    m_vColors.append(QColor(0xFFFF00FF)); // Violet
    m_vColors.append(QColor(0xFFFFFFFF)); // White
    m_vColors.append(QColor(0xFF000000)); // Black
}

void KisPainterlyMixer::changeColor(int index)
{
    m_resources->setResource(KoCanvasResource::ForegroundColor, KoColor(m_vColors[index], m_canvas->device()->colorSpace()));
}


#include "kis_painterlymixer.moc"
