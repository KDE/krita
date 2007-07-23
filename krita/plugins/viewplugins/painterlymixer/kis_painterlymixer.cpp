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
#include <QList>
#include <QPalette>
#include <QSizePolicy>
#include <QString>
#include <QStringList>
#include <QToolButton>
#include <QWidget>

#include <KComponentData>
#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>

#include "KoCanvasResourceProvider.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorSpace.h"
#include "KoColor.h"
#include "KoColorProfile.h"
#include "KoToolProxy.h"

#include "kis_canvas2.h"
#include "kis_painter.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_resource_provider.h"
#include "kis_view2.h"

#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace.h"

#include "colorspot.h"
#include "mixertool.h"

#include "kis_painterlymixer.h"

KisPainterlyMixer::KisPainterlyMixer(QWidget *parent, KisView2 *view)
    : QWidget(parent), m_view(view), m_resources(view->canvasBase()->resourceProvider())
{
    setupUi(this);

	KGlobal::mainComponent().dirs()->addResourceType("kis_illuminants",
                                                     "data", "krita/profiles/");

    m_illuminants += KGlobal::mainComponent().dirs()->findAllResources("kis_illuminants", "*.ill");

	m_illuminant = new KisIlluminantProfile(m_illuminants[0]);
	m_colorspace = new KisKSColorSpace(m_illuminant);
    m_canvas->setDevice(m_colorspace);
    initTool();
    initSpots();
}

KisPainterlyMixer::~KisPainterlyMixer()
{
    if (m_tool)
        delete m_tool;
	delete m_illuminant;
	delete m_colorspace;
}

void KisPainterlyMixer::initTool()
{
    m_tool = new MixerTool(m_canvas, m_canvas->device(), m_canvas->overlay(), m_resources);

    m_canvas->setToolProxy(new KoToolProxy(m_canvas));
    m_canvas->toolProxy()->setActiveTool(m_tool);

    updateInformationLabel();
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
            QToolButton *curr = new ColorSpot(m_spotsFrame, m_vColors[index]);
            l->addWidget(curr, row, col);

            m_bgColors->addButton(curr, index);
        }
    }

    l->setColumnStretch(col++, 1);

    m_bWet = new QToolButton(m_spotsFrame);
    m_bWet->setText("W"); // TODO Icon for the "Wet spot"
    m_bWet->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_bWet->setAutoRepeat(true);
    m_bDry = new QToolButton(m_spotsFrame);
    m_bDry->setText("D"); // TODO Icon for the "Dry spot"
    m_bDry->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_bDry->setAutoRepeat(true);

    l->addWidget(m_bWet, 0, col++, ROWS, 1);
    l->addWidget(m_bDry, 0, col, ROWS, 1);

    connect(m_bgColors, SIGNAL(buttonClicked(int)), this, SLOT(slotChangeColor(int)));
    connect(m_bWet, SIGNAL(pressed()), this, SLOT(slotIncreaseWetness()));
    connect(m_bDry, SIGNAL(pressed()), this, SLOT(slotDecreaseWetness()));
}

void KisPainterlyMixer::loadColors()
{
    // TODO We need to handle save/load of user-defined colors in the spots.
	KoColorSpace *cs = m_canvas->device()->colorSpace();
    m_vColors.append(KoColor(QColor(0xFFFF0000), cs)); // Red
    m_vColors.append(KoColor(QColor(0xFF00FF00), cs)); // Green
    m_vColors.append(KoColor(QColor(0xFF0000FF), cs)); // Blue
    m_vColors.append(KoColor(QColor(0xFF0FA311), cs)); // Whatever :)
    m_vColors.append(KoColor(QColor(0xFFFFFF00), cs)); // Yellow
    m_vColors.append(KoColor(QColor(0xFFFF00FF), cs)); // Violet
    m_vColors.append(KoColor(QColor(0xFFFFFFFF), cs)); // White
    m_vColors.append(KoColor(QColor(0xFF000000), cs)); // Black
}

// TODO Load/Save of wetness steps
#define MINIMUM_PV 120
#define PV_STEP 2.5
#define WET_DRY_STEP 0.025

void KisPainterlyMixer::slotChangeColor(int index)
{
    if (m_resources->foregroundColor().toQColor().rgba() == m_vColors[index].toQColor().rgba()) {
        float pc = m_tool->bristleInformation("PaintVolume");
        if ((pc + PV_STEP) < 255.0)
            m_tool->setBristleInformation("PaintVolume", PV_STEP, KPI_RELATIVE);
        else
            m_tool->setBristleInformation("PaintVolume", 255.0, KPI_ABSOLUTE);
    } else {
        m_resources->setForegroundColor(m_vColors[index]);
        m_tool->setBristleInformation("PaintVolume", MINIMUM_PV, KPI_ABSOLUTE);
    }

    updateInformationLabel();
}

void KisPainterlyMixer::slotIncreaseWetness()
{
    if (isCurrentPaintOpPainterly()) {
        // TODO Do something to change wetness in painterly paintops.
    } else {
        float wetness = m_tool->bristleInformation("Wetness");
        if ((wetness + WET_DRY_STEP) < 1.0)
            m_tool->setBristleInformation("Wetness", WET_DRY_STEP, KPI_RELATIVE);
        else
            m_tool->setBristleInformation("Wetness", 1.0, KPI_ABSOLUTE);
    }
    updateInformationLabel();
}

void KisPainterlyMixer::slotDecreaseWetness()
{
    // TODO Load/Save of wetness steps

    if (isCurrentPaintOpPainterly()) {
        // TODO Do something to change wetness in painterly paintops.
    } else {
        float wetness = m_tool->bristleInformation("Wetness");
        if (wetness > 0.1 + WET_DRY_STEP)
            m_tool->setBristleInformation("Wetness", -WET_DRY_STEP, KPI_RELATIVE);
    }
    updateInformationLabel();
}

bool KisPainterlyMixer::isCurrentPaintOpPainterly()
{
    // Is there a cleaner way to do this?!
    KisPainter painter(m_canvas->device());
    KisPaintOp *current = KisPaintOpRegistry::instance()->paintOp(
                          m_resources->resource(KisResourceProvider::CurrentPaintop).value<KoID>().id(),
                          static_cast<KisPaintOpSettings*>(m_resources->resource(KisResourceProvider::CurrentPaintopSettings).value<void*>()),
                          &painter, 0);
    painter.setPaintOp(current);

    return current->painterly();
}

void KisPainterlyMixer::updateInformationLabel()
{
    QString text;
    KisPainterlyInformation info = m_tool->bristleInformation();

    text.sprintf("W: %.2f V: %.2f PC: %.2f", info["Wetness"], info["PaintVolume"], info["PigmentConcentration"]);

    m_information->setText(text);
}


#include "kis_painterlymixer.moc"
