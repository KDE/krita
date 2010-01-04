/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_grid_manager.h"

#include <opengl/kis_opengl.h>

#ifdef HAVE_OPENGL
#include <qgl.h>
#endif

#include <QRadioButton>

#include <kaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <kdialog.h>
#include <klocale.h>

#include <KoViewConverter.h>

#include "kis_config.h"
#include "kis_grid_painter_configuration.h"
#include "kis_image.h"
#include "kis_view2.h"
#include "kis_doc2.h"

KisGridManager::KisGridManager(KisView2 * parent)
        : KisCanvasDecoration("grid", i18n("Grid"), parent), m_view(parent)
{

}

KisGridManager::~KisGridManager()
{

}

void KisGridManager::setup(KActionCollection * collection)
{
    //there is no grid by default
    m_view->document()->gridData().setShowGrid(false);

    KisConfig config;
    m_view->document()->gridData().setGrid(config.getGridHSpacing(), config.getGridVSpacing());

    toggleGrid = m_view->document()->gridData().gridToggleAction();
    collection->addAction("view_grid", toggleGrid);
    connect(toggleGrid, SIGNAL(triggered()), this, SLOT(toggleVisibility()));

    m_toggleSnapToGrid  = new KToggleAction(i18n("Snap To Grid"), this);
    collection->addAction("view_snap_to_grid", m_toggleSnapToGrid);
    connect(toggleGrid, SIGNAL(triggered()), this, SLOT(toggleSnapToGrid()));

    // Fast grid config
    m_gridFastConfig1x1  = new KAction(i18n("1x1"), this);
    collection->addAction("view_fast_grid_1x1", m_gridFastConfig1x1);
    connect(m_gridFastConfig1x1, SIGNAL(triggered()), this, SLOT(fastConfig1x1()));

    m_gridFastConfig2x2  = new KAction(i18n("2x2"), this);
    collection->addAction("view_fast_grid_2x2", m_gridFastConfig2x2);
    connect(m_gridFastConfig2x2, SIGNAL(triggered()), this, SLOT(fastConfig2x2()));

    m_gridFastConfig5x5  = new KAction(i18n("5x5"), this);
    collection->addAction("view_fast_grid_5x5", m_gridFastConfig5x5);
    connect(m_gridFastConfig5x5, SIGNAL(triggered()), this, SLOT(fastConfig5x5()));

    m_gridFastConfig10x10  = new KAction(i18n("10x10"), this);
    collection->addAction("view_fast_grid_10x10", m_gridFastConfig10x10);
    connect(m_gridFastConfig10x10, SIGNAL(triggered()), this, SLOT(fastConfig10x10()));

    m_gridFastConfig20x20  = new KAction(i18n("20x20"), this);
    collection->addAction("view_fast_grid_20x20", m_gridFastConfig20x20);
    connect(m_gridFastConfig20x20, SIGNAL(triggered()), this, SLOT(fastConfig20x20()));

    m_gridFastConfig40x40  = new KAction(i18n("40x40"), this);
    collection->addAction("view_fast_grid_40x40", m_gridFastConfig40x40);
    connect(m_gridFastConfig40x40, SIGNAL(triggered()), this, SLOT(fastConfig40x40()));
}

void KisGridManager::updateGUI()
{

}

void KisGridManager::checkVisibilityAction(bool check)
{
    toggleGrid->setChecked(check);
}

void KisGridManager::toggleSnapToGrid()
{
    m_view->document()->gridData().setSnapToGrid(m_toggleSnapToGrid->isChecked());
    m_view->canvas()->update();
}

void KisGridManager::fastConfig1x1()
{
    KisConfig cfg;
    cfg.setGridHSpacing(1);
    cfg.setGridVSpacing(1);
    m_view->canvas()->update();
}

void KisGridManager::fastConfig2x2()
{
    KisConfig cfg;
    cfg.setGridHSpacing(2);
    cfg.setGridVSpacing(2);
    m_view->canvas()->update();
}

void KisGridManager::fastConfig5x5()
{
    KisConfig cfg;
    cfg.setGridHSpacing(5);
    cfg.setGridVSpacing(5);
    m_view->canvas()->update();
}

void KisGridManager::fastConfig10x10()
{
    KisConfig cfg;
    cfg.setGridHSpacing(10);
    cfg.setGridVSpacing(10);
    m_view->canvas()->update();
}

void KisGridManager::fastConfig20x20()
{
    KisConfig cfg;
    cfg.setGridHSpacing(20);
    cfg.setGridVSpacing(20);
    m_view->canvas()->update();
}

void KisGridManager::fastConfig40x40()
{
    KisConfig cfg;
    cfg.setGridHSpacing(40);
    cfg.setGridVSpacing(40);
    m_view->canvas()->update();
}

#define pixelToView(point) \
    converter.documentToView( m_view->document()->image()->pixelToDocument(point))

#define viewToPixel(point) \
    m_view->document()->image()->documentToPixel( converter.viewToDocument( point))

void KisGridManager::drawDecoration(QPainter& gc, const QPoint& documentOffset, const QRect& area, const KoViewConverter &converter)
{
    dbgRender << "drawDecoration";
    KisConfig cfg;

    quint32 offsetx = cfg.getGridOffsetX();
    quint32 offsety = cfg.getGridOffsetY();
    quint32 hspacing = cfg.getGridHSpacing(); // m_doc->gridData().gridX(); // use koffice grid when KOffice grid is on par with Krita grid, and is configurable by whatever mean Krita has to manipulate the grid
    quint32 vspacing = cfg.getGridVSpacing(); // m_doc->gridData().gridY(); // use koffice grid when KOffice grid is on par with Krita grid, and is configurable by whatever mean Krita has to manipulate the grid
    quint32 subdivision = cfg.getGridSubdivisions() - 1;

    // Draw vertical line
    QPen mainPen = KisGridPainterConfiguration::mainPen();
    QPen subdivisionPen = KisGridPainterConfiguration::subdivisionPen();
    quint32 i = subdivision - (offsetx / hspacing) % (subdivision + 1);

    QPointF bottomRight = viewToPixel(area.bottomRight() + documentOffset);
    QPointF topLeft = viewToPixel(area.topLeft() + documentOffset);

    dbgRender << "area = " << area << "topLeft = " << topLeft << " bottomRight = " << bottomRight << " documentOffset = " << documentOffset;

    double x = offsetx % hspacing;
    dbgRender << " x = " << x << " hspacing = " << hspacing;
    while (x <= bottomRight.x()) {
        if (i == subdivision) {
            gc.setPen(mainPen);
            i = 0;
        } else {
            gc.setPen(subdivisionPen);
            i++;
        }
        if (x >= topLeft.x()) {
            // Always draw the full line otherwise the line stippling varies
            // with the location of area and we get glitchy patterns.
            dbgRender << pixelToView(QPointF(x, topLeft.y())) << " " << pixelToView(QPointF(x, bottomRight.y())) << (bottomRight.y());
            gc.drawLine(pixelToView(QPointF(x, topLeft.y())), pixelToView(QPointF(x, bottomRight.y())));
        }
        x += hspacing;
    }
    // Draw horizontal line
    i = subdivision - (offsety / vspacing) % (subdivision + 1);
    double y = offsety % vspacing;
    dbgRender << " y = " << x << " vspacing = " << vspacing;
    while (y <= bottomRight.y()) {
        if (i == subdivision) {
            gc.setPen(mainPen);
            i = 0;
        } else {
            gc.setPen(subdivisionPen);
            i++;
        }
        if (y >= topLeft.y()) {
            gc.drawLine(pixelToView(QPointF(topLeft.x(), y)), pixelToView(QPointF(bottomRight.x(), y)));
        }
        y += vspacing;
    }

}

#include "kis_grid_manager.moc"
