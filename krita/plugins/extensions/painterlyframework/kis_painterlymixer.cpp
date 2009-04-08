/* This file is part of the KDE project

   Copyright (C) 2007 Emanuele Tamponi <emanuele@valinor.it>

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

#include "kis_painterlymixer.h"

#include "colorspot.h"
#include "kis_ksf32_colorspace.h"
#include "mixercanvas.h"
#include "mixertool.h"

#include <kis_paintop.h>
#include <kis_canvas_resource_provider.h>
#include <kis_view2.h>
#include <KoColorSpace.h>
#include <KoToolProxy.h>
#include <QButtonGroup>

KisPainterlyMixer::KisPainterlyMixer(QWidget *parent, KisView2 *view)
        : QWidget(parent), m_view(view), m_resources(view->resourceProvider()), m_tool(0), m_colorspace(0)
{
    setupUi(this);

    QString csid = KisKSF32ColorSpace<3>::ColorSpaceId().id();
    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();

    // TODO Illuminant and colorspace has to be chosen at runtime
    QList<const KoColorProfile*> profiles = f->profilesFor(csid);
    if (profiles.count()) { // don't crash if the profile is not available.
        const KoColorProfile* testp = profiles.at(0);
        m_colorspace = f->colorSpace(csid, testp);
        Q_ASSERT(m_colorspace);
    } else {
        setEnabled(false);
        return;
    }

    initCanvas();
    initTool();
    initSpots();

    m_bErase->setIcon(KIcon("edit-delete"));
    connect(m_bErase, SIGNAL(clicked()), m_canvas, SLOT(slotClear()));
}

KisPainterlyMixer::~KisPainterlyMixer()
{
    if (m_tool)
        delete m_tool;
}

void KisPainterlyMixer::initCanvas()
{
    m_canvas->setLayer(m_colorspace);
    m_canvas->setToolProxy(new KoToolProxy(m_canvas));
    m_canvas->setResources(m_resources);
}

void KisPainterlyMixer::initTool()
{
    Q_ASSERT(m_canvas);
    m_tool = new MixerTool(m_canvas, m_resources);
    m_canvas->toolProxy()->setActiveTool(m_tool);
    m_tool->activate();
}

#define ROWS 2
#define COLS 4

void KisPainterlyMixer::initSpots()
{
    int row, col;
    QGridLayout *l = new QGridLayout(m_spotsFrame);

    m_bgColors = new QButtonGroup(m_spotsFrame);
    loadColors();

    l->setSpacing(5);
    for (row = 0; row < ROWS; row++) {
        for (col = 0; col < COLS; col++) {
            int index = row * COLS + col;
            QToolButton *curr = new ColorSpot(m_spotsFrame, m_vColors[index]);
            l->addWidget(curr, row, col);

            m_bgColors->addButton(curr, index);
        }
    }

    l->setColumnStretch(col++, 1);

    connect(m_bgColors, SIGNAL(buttonClicked(int)), this, SLOT(slotChangeColor(int)));
}

void KisPainterlyMixer::loadColors()
{
    // TODO We need to handle save/load of user-defined colors in the spots.
    const KoColorSpace *cs = m_colorspace;
    m_vColors.append(KoColor(QColor("#FF0000"), cs)); // Red
    m_vColors.append(KoColor(QColor("#00FF00"), cs)); // Green
    m_vColors.append(KoColor(QColor("#0000FF"), cs)); // Blue
    m_vColors.append(KoColor(QColor("#006464"), cs)); // Seablue
    m_vColors.append(KoColor(QColor("#FFFF00"), cs)); // Yellow
    m_vColors.append(KoColor(QColor("#700070"), cs)); // Violet
    m_vColors.append(KoColor(QColor("#FFFFFF"), cs)); // White
    m_vColors.append(KoColor(QColor("#101010"), cs)); // Black
}

void KisPainterlyMixer::slotChangeColor(int index)
{
    m_resources->setFGColor(m_vColors[index]);
}

#include "kis_painterlymixer.moc"
