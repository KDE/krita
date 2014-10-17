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


#include <QTransform>

#include <kaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <klocale.h>

#include "kis_coordinates_converter.h"

#include "kis_config.h"
#include "kis_grid_painter_configuration.h"
#include "kis_image.h"
#include "kis_view2.h"
#include "kis_doc2.h"

KisGridManager::KisGridManager(KisView2 * parent)
        : KisCanvasDecoration("grid", parent), m_view(parent)
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
    toggleGrid->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Apostrophe));
    collection->addAction("view_grid", toggleGrid);
    connect(toggleGrid, SIGNAL(triggered()), this, SLOT(toggleVisibility()));

    m_toggleSnapToGrid  = new KToggleAction(i18n("Snap To Grid"), this);
    m_toggleSnapToGrid->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Semicolon));
    collection->addAction("view_snap_to_grid", m_toggleSnapToGrid);
    connect(m_toggleSnapToGrid, SIGNAL(triggered()), this, SLOT(toggleSnapToGrid()));

    // Fast grid config
    m_gridFastConfig1x1  = new KAction(i18n("1x1"), this);
    collection->addAction("view_fast_grid_1x1", m_gridFastConfig1x1);
    connect(m_gridFastConfig1x1, SIGNAL(triggered()), this, SLOT(fastConfig1x1()));

    m_gridFastConfig2x2  = new KAction(i18n("2x2"), this);
    collection->addAction("view_fast_grid_2x2", m_gridFastConfig2x2);
    connect(m_gridFastConfig2x2, SIGNAL(triggered()), this, SLOT(fastConfig2x2()));

    m_gridFastConfig4x4  = new KAction(i18n("4x4"), this);
    collection->addAction("view_fast_grid_4x4", m_gridFastConfig4x4);
    connect(m_gridFastConfig4x4, SIGNAL(triggered()), this, SLOT(fastConfig4x4()));
    
    m_gridFastConfig5x5  = new KAction(i18n("5x5"), this);
    collection->addAction("view_fast_grid_5x5", m_gridFastConfig5x5);
    connect(m_gridFastConfig5x5, SIGNAL(triggered()), this, SLOT(fastConfig5x5()));

    m_gridFastConfig8x8  = new KAction(i18n("8x8"), this);
    collection->addAction("view_fast_grid_8x8", m_gridFastConfig8x8);
    connect(m_gridFastConfig8x8, SIGNAL(triggered()), this, SLOT(fastConfig8x8()));
        
    m_gridFastConfig10x10  = new KAction(i18n("10x10"), this);
    collection->addAction("view_fast_grid_10x10", m_gridFastConfig10x10);
    connect(m_gridFastConfig10x10, SIGNAL(triggered()), this, SLOT(fastConfig10x10()));

    m_gridFastConfig16x16  = new KAction(i18n("16x16"), this);
    collection->addAction("view_fast_grid_16x16", m_gridFastConfig16x16);
    connect(m_gridFastConfig16x16, SIGNAL(triggered()), this, SLOT(fastConfig16x16()));

    m_gridFastConfig20x20  = new KAction(i18n("20x20"), this);
    collection->addAction("view_fast_grid_20x20", m_gridFastConfig20x20);
    connect(m_gridFastConfig20x20, SIGNAL(triggered()), this, SLOT(fastConfig20x20()));

    m_gridFastConfig32x32  = new KAction(i18n("32x32"), this);
    collection->addAction("view_fast_grid_32x32", m_gridFastConfig32x32);
    connect(m_gridFastConfig32x32, SIGNAL(triggered()), this, SLOT(fastConfig32x32()));
    
    m_gridFastConfig40x40  = new KAction(i18n("40x40"), this);
    collection->addAction("view_fast_grid_40x40", m_gridFastConfig40x40);
    connect(m_gridFastConfig40x40, SIGNAL(triggered()), this, SLOT(fastConfig40x40()));
    
    m_gridFastConfig64x64  = new KAction(i18n("64x64"), this);
    collection->addAction("view_fast_grid_64x64", m_gridFastConfig64x64);
    connect(m_gridFastConfig64x64, SIGNAL(triggered()), this, SLOT(fastConfig64x64()));
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

void KisGridManager::fastConfig4x4()
{
    KisConfig cfg;
    cfg.setGridHSpacing(4);
    cfg.setGridVSpacing(4);
    m_view->canvas()->update();
}

void KisGridManager::fastConfig8x8()
{
    KisConfig cfg;
    cfg.setGridHSpacing(8);
    cfg.setGridVSpacing(8);
    m_view->canvas()->update();
}

void KisGridManager::fastConfig16x16()
{
    KisConfig cfg;
    cfg.setGridHSpacing(16);
    cfg.setGridVSpacing(16);
    m_view->canvas()->update();
}

void KisGridManager::fastConfig32x32()
{
    KisConfig cfg;
    cfg.setGridHSpacing(32);
    cfg.setGridVSpacing(32);
    m_view->canvas()->update();
}

void KisGridManager::fastConfig64x64()
{
    KisConfig cfg;
    cfg.setGridHSpacing(64);
    cfg.setGridVSpacing(64);
    m_view->canvas()->update();
}

void KisGridManager::drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter, KisCanvas2 *canvas)
{
    KisConfig cfg;

    int offsetx = cfg.getGridOffsetX();
    int offsety = cfg.getGridOffsetY();
    int hspacing = cfg.getGridHSpacing();
    int vspacing = cfg.getGridVSpacing();
    int subdivision = cfg.getGridSubdivisions() - 1;
    bool useEmergencyPen = false;

    QPen mainPen = KisGridPainterConfiguration::mainPen();
    QPen subdivisionPen = KisGridPainterConfiguration::subdivisionPen();
    QPen emergencyPen(subdivisionPen.color(), 0, Qt::DotLine);

    qreal x1, y1, x2, y2;
    QRectF imageRect =
        converter->documentToImage(updateRect) & canvas->image()->bounds();
    imageRect.getCoords(&x1, &y1, &x2, &y2);

    QTransform transform = converter->imageToWidgetTransform();

    bool gridIsPaintable = true;
    const qreal gridThreshold = 8.0;
    do {
        QRectF widgetRect = converter->imageToWidget(QRectF(0, 0, hspacing, vspacing));
        gridIsPaintable = widgetRect.width() >= gridThreshold &&
            widgetRect.height() >= gridThreshold;

        if (!gridIsPaintable) {
            useEmergencyPen = true;
            hspacing *= qMax(2, subdivision + 1);
            vspacing *= qMax(2, subdivision + 1);
        }
    } while (!gridIsPaintable);

    gc.save();
    gc.setTransform(transform);

    int i;

    // Draw vertical line
    i = subdivision - (offsetx / hspacing) % (subdivision + 1);
    double x = offsetx % hspacing;
    while (x <= x2) {
        if (useEmergencyPen) {
            gc.setPen(emergencyPen);
        } else if (i == subdivision) {
            gc.setPen(mainPen);
            i = 0;
        } else {
            gc.setPen(subdivisionPen);
            i++;
        }
        if (x >= x1) {
            // Always draw the full line otherwise the line stippling varies
            // with the location of area and we get glitchy patterns.
            gc.drawLine(QPointF(x, y1),QPointF(x, y2));
        }
        x += hspacing;
    }

    // Draw horizontal line
    i = subdivision - (offsety / vspacing) % (subdivision + 1);
    qreal y = offsety % vspacing;
    while (y <= y2) {
        if (useEmergencyPen) {
            gc.setPen(emergencyPen);
        } else if (i == subdivision) {
            gc.setPen(mainPen);
            i = 0;
        } else {
            gc.setPen(subdivisionPen);
            i++;
        }
        if (y >= y1) {
            gc.drawLine(QPointF(x1, y), QPointF(x2, y));
        }
        y += vspacing;
    }

    gc.restore();
}

#include "kis_grid_manager.moc"
