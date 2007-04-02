/*
 *  kis_tool_duplicate.cc - part of Krita
 *
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_tool_duplicate.h"

#include <QBitmap>
#include <QCheckBox>
#include <QLabel>
#include <QPainter>
#include <QSpinBox>

#include <kdebug.h>
#include <klocale.h>

#include "kis_brush.h"
#include "KoPointerEvent.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_perspective_grid.h"
#include "kis_vec.h"

#include "QPainter"
#include "kis_boundary_painter.h"

KisToolDuplicate::KisToolDuplicate(KoCanvasBase * canvas)
    : super(canvas, KisCursor::load("tool_duplicate_cursor.png", 6, 6), i18n("Duplicate Brush")),
      m_isOffsetNotUptodate(true),
      m_position(QPoint(-1,-1))
{
    setObjectName("tool_duplicate");
}

KisToolDuplicate::~KisToolDuplicate()
{
}

void KisToolDuplicate::activate()
{
    m_position = QPoint(-1,-1);
    super::activate();
    if( m_currentImage->perspectiveGrid()->countSubGrids() != 1 )
    {
        m_perspectiveCorrection->setEnabled( false );
        m_perspectiveCorrection->setChecked( false );
    } else {
        m_perspectiveCorrection->setEnabled( true );
    }
}

void KisToolDuplicate::mousePressEvent(KoPointerEvent *e)
{
    if (e->modifiers() == Qt::ShiftModifier) {
        m_position = convertToPixelCoord(e);
        m_isOffsetNotUptodate = true;
    } else {
        if (m_position != QPoint(-1, -1)) {
            super::mousePressEvent(e);
        }
    }
}

void KisToolDuplicate::initPaint(KoPointerEvent *e)
{
    if( m_position != QPoint(-1,-1))
    {
        if(m_isOffsetNotUptodate)
        {
            m_offset = convertToPixelCoord(e) - m_position;
            m_isOffsetNotUptodate = false;
        }
        m_paintIncremental = false;
        super::initPaint(e);
        m_painter->setDuplicateOffset( m_offset );
        KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp("duplicate", 0, m_painter);
        if (op && m_source) {
            op->setSource(m_source);
            m_painter->setPaintOp(op);
        }
        m_positionStartPainting = convertToPixelCoord(e);
        m_painter->setDuplicateStart( convertToPixelCoord(e) );
    }
}

void KisToolDuplicate::mouseMoveEvent(KoPointerEvent *e)
{
    QPointF pos = convertToPixelCoord(e);

    // Paint the outline where we will (or are) copying from
    if( m_position == QPoint(-1,-1) )
        return;

    QPoint srcPos;
    if (m_mode == PAINT) {
        // if we are in perspective correction mode, update the offset when moving
        if(m_perspectiveCorrection->isChecked())
        {
            double startM[3][3];
            double endM[3][3];
            for(int i = 0; i < 3; i++)
            {
                for(int j = 0; j < 3; j++)
                {
                    startM[i][j] = 0.;
                    endM[i][j] = 0.;
                }
                startM[i][i] = 1.;
                endM[i][i] = 1.;
            }
        
        // First look for the grid corresponding to the start point
            KisSubPerspectiveGrid* subGridStart = *m_currentImage->perspectiveGrid()->begin();//device->image()->perspectiveGrid()->gridAt(QPointF(srcPoint.x() +hotSpot.x(),srcPoint.y() +hotSpot.y()));
            QRect r = QRect(0,0, m_currentImage->width(), m_currentImage->height());
        
            if(subGridStart)
            {
                double* b = KisPerspectiveMath::computeMatrixTransfoFromPerspective( r, *subGridStart->topLeft(), *subGridStart->topRight(), *subGridStart->bottomLeft(), *subGridStart->bottomRight());
                for(int i = 0; i < 3; i++)
                {
                    for(int j = 0; j < 3; j++)
                    {
                        startM[i][j] = b[3*i+j];
                    }
                }

            }
        // Second look for the grid corresponding to the end point
            KisSubPerspectiveGrid* subGridEnd = *m_currentImage->perspectiveGrid()->begin();// device->image()->perspectiveGrid()->gridAt(pos);
            if(subGridEnd)
            {
                double* b = KisPerspectiveMath::computeMatrixTransfoToPerspective(*subGridEnd->topLeft(), *subGridEnd->topRight(), *subGridEnd->bottomLeft(), *subGridEnd->bottomRight(), r);
                for(int i = 0; i < 3; i++)
                {
                    for(int j = 0; j < 3; j++)
                    {
                        endM[i][j] = b[3*i+j];
                    }
                }
            }
        // Compute the translation in the perspective transformation space:
            QPointF translat;
            {
                QPointF positionStartPaintingT = KisPerspectiveMath::matProd(endM, m_positionStartPainting);
                QPointF currentPositionT = KisPerspectiveMath::matProd(endM, e->pos() );
                QPointF duplicateStartPoisitionT = KisPerspectiveMath::matProd(endM, m_positionStartPainting - m_offset);
                QPointF duplicateRealPosition = KisPerspectiveMath::matProd(startM, duplicateStartPoisitionT + (currentPositionT - positionStartPaintingT) );
                QPointF p = pos - duplicateRealPosition;
                srcPos = QPoint(static_cast<int>(pos.x()), static_cast<int>(pos.y()));
            }

        }else {
            srcPos = QPoint(static_cast<int>(m_painter->duplicateOffset().x()), static_cast<int>(m_painter->duplicateOffset().y()));
        }
    } else {
        if(m_isOffsetNotUptodate)
            srcPos = QPoint(static_cast<int>(pos.x()), static_cast<int>(pos.y())) - QPoint(static_cast<int>(m_position.x()), static_cast<int>(m_position.y()));
        else
            srcPos = QPoint(static_cast<int>(m_offset.x()), static_cast<int>(m_offset.y()));
    }

    qint32 x;
    qint32 y;

    // like KisPaintOp::splitCoordinate
    x = (qint32)((e->x() < 0) ? e->x() - 1 : e->x());
    y = (qint32)((e->y() < 0) ? e->y() - 1 : e->y());
    srcPos = QPoint(x - srcPos.x(), y - srcPos.y());

    paintOutline(srcPos);
    super::mouseMoveEvent(e);
}

void KisToolDuplicate::paintAt(const QPointF &pos,
                   const double pressure,
                   const double xtilt,
                   const double ytilt)
{
    if( m_position != QPoint(-1,-1))
    {
        if(m_isOffsetNotUptodate)
        {
            m_offset = pos - m_position;
            m_isOffsetNotUptodate = false;
        }
        m_painter->setDuplicateHealing( m_healing->isChecked() );
        m_painter->setDuplicateHealingRadius( m_healingRadius->value() );
        m_painter->setDuplicatePerspectiveCorrection( m_perspectiveCorrection->isChecked() );
        m_painter->paintAt( pos, pressure, xtilt, ytilt);
    }
}

QString KisToolDuplicate::quickHelp() const {
    return i18n("To start, shift-click on the place you want to duplicate from. Then you can start painting. An indication of where you are copying from will be displayed while drawing and moving the mouse.");
}

QWidget* KisToolDuplicate::createOptionWidget()
{
    QWidget* widget = KisToolPaint::createOptionWidget();
    m_healing = new QCheckBox(widget);
    m_healing->setChecked( false);
    addOptionWidgetOption(m_healing, new QLabel(i18n("Healing"), widget ));
    m_healingRadius = new QSpinBox(widget);

    int healingradius = 20;
    if( m_currentBrush )
    {
        healingradius = 2 * qMax(m_currentBrush->width(),m_currentBrush->height());
    }
    
    m_healingRadius->setValue( healingradius );
    addOptionWidgetOption(m_healingRadius, new QLabel(i18n("Healing radius"), widget ));
    m_perspectiveCorrection =  new QCheckBox(widget);
    addOptionWidgetOption(m_perspectiveCorrection, new QLabel(i18n("Correct the perspective"), widget ));
    return widget;
}

#include "kis_tool_duplicate.moc"
