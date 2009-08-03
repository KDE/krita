/*
 *  kis_tool_moutline.cc -- part of Krita
 *
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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

#include "kis_tool_moutline.h"


#include <math.h>
#include <set>

#include <QPainter>
#include <QLayout>
#include <QRect>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QPointF>
#include <QList>

#include <kis_debug.h>
#include <klocale.h>
#include <knuminput.h>
#include <kaction.h>
#include <kactioncollection.h>

#include "kis_global.h"
#include "kis_iterators_pixel.h"
//#include "kis_doc2.h"
#include "kis_painter.h"
#include "KoPointerEvent.h"
#include "kis_cursor.h"
#include "kis_vec.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selected_transaction.h"
#include "kis_paintop_registry.h"
#include "kis_convolution_painter.h"
#include "canvas/kis_canvas.h"

KisToolMagnetic::KisToolMagnetic()
        : KisToolCurve("Magnetic Outline Tool")
{
    setName("tool_moutline");
    setCursor(KisCursor::load("tool_moutline_cursor.png", 6, 6));

    m_editingMode = false;
    m_editingCursor = m_draggingCursor = false;

    m_mode = 0;
    m_curve = m_derived = 0;
//    m_current = m_previous = 0;

    m_distance = DEFAULTDIST;

    m_transactionMessage = i18n("Magnetic Outline Selection");
}

KisToolMagnetic::~KisToolMagnetic()
{
    m_curve = 0;
    delete m_derived;
}

void KisToolMagnetic::activate()
{
    KisToolCurve::activate();
    if (!m_derived) {
        m_derived = new KisCurveMagnetic(this);
        m_curve = m_derived;
    }
}

void KisToolMagnetic::deactivate()
{
    m_curve->endActionOptions();
    m_actionOptions = NOOPTIONS;
    m_dragging = false;
    m_drawPivots = true;
}

void KisToolMagnetic::keyPress(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) {
        draw(false);
        if (m_editingMode) {
            m_editingMode = false;
//            if (m_current != 0)
            m_curve->selectPivot(m_current, false);
            m_mode->setText(i18n("Automatic Mode"));
        } else {
            m_editingMode = true;
            m_mode->setText(i18n("Manual Mode"));
        }
        draw(false);
    } else if (event->key() == Qt::Key_Delete && m_curve->count()) {
        draw(false);
        m_dragging = false;
        if (m_curve->pivots().count() == 2)
            m_curve->clear();
        else {
            if ((*m_current) == m_curve->last() && !(m_editingMode)) {
                m_curve->deletePivot(m_current.previousPivot());
                m_previous = m_current.previousPivot();
            } else {
                m_editingMode = false;
                m_curve->deletePivot(m_current);
                m_previous = m_current = m_curve->selectPivot(m_curve->lastIterator());
                m_editingMode = true;
            }
        }
        draw(false);
    } else
        KisToolCurve::keyPress(event);
}

void KisToolMagnetic::buttonRelease(KoPointerEvent *event)
{
    if (m_editingMode) {
        draw(m_current);
        m_editingMode = false;
        if (!m_curve->isEmpty())
            m_curve->movePivot(m_current, m_currentPoint);
        m_editingMode = true;
        draw(m_current);
    }
    KisToolCurve::buttonRelease(event);
}

void KisToolMagnetic::buttonPress(KoPointerEvent *event)
{
    updateOptions(event->modifiers());
    if (!m_currentImage)
        return;
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_currentPoint = event->pos().toPointF();
        PointPair temp(m_curve->end(), false);
        if (m_editingMode)
            temp = pointUnderMouse(m_subject->canvasController()->windowToView(event->pos()).toPointF());
        if (temp.first == m_curve->end() && !(m_actionOptions)) {
            if (m_editingMode) {
                draw(true, true);
                m_curve->selectAll(false);
                draw(true, true);
            }
            draw(m_curve->end());
            if (!m_curve->isEmpty()) {
                m_previous = m_current;
                m_current = m_curve->pushPivot(event->pos().toPointF());
            } else {
                m_previous = m_current = m_curve->pushPivot(event->pos().toPointF());
            }
            if (m_curve->pivots().count() > 1)
                m_curve->calculateCurve(m_previous, m_current, m_current);
            if (m_editingMode)
                draw();
            else {
                if ((*m_previous).point() == (*m_current).point())
                    draw(m_curve->end());
                else
                    draw();
            }
        } else if (temp.first != m_curve->end() && m_editingMode) {
            if (temp.second) {
                draw(true, true);
                m_current = m_curve->selectPivot(temp.first);
                draw(true, true);
            } else {
                draw(false);
                m_current = selectByMouse(temp.first);
                draw(false);
            }
            if (!(*m_current).isSelected())
                m_dragging = false;
        }
    }
}

void KisToolMagnetic::move(KoPointerEvent *event)
{
    updateOptions(event->modifiers());
    if (m_currentPoint == event->pos().floorQPoint())
        return;
    if (m_editingMode) {
        PointPair temp = pointUnderMouse(m_subject->canvasController()->windowToView(event->pos()).toPointF());
        if (temp.first == m_curve->end() && !m_dragging) {
            if (m_editingCursor || m_draggingCursor) {
                setCursor(KisCursor::load("tool_moutline_cursor.png", 6, 6));
                m_editingCursor = m_draggingCursor = false;
            }
        } else {
            if (!m_draggingCursor && temp.second) {
                setCursor(KisCursor::load("tool_moutline_dragging.png", 6, 6));
                m_editingCursor = false;
                m_draggingCursor = true;
            }
            if (!m_editingCursor && !temp.second) {
                setCursor(KisCursor::load("tool_moutline_editing.png", 6, 6));
                m_editingCursor = true;
                m_draggingCursor = false;
            }
        }
        if (!m_dragging)
            return;
    } else {
        if (m_editingCursor || m_draggingCursor) {
            setCursor(KisCursor::load("tool_moutline_cursor.png", 6, 6));
            m_editingCursor = m_draggingCursor = false;
        }
    }
    if (m_curve->selectedPivots().isEmpty())
        return;

    QPointF trans = event->pos().toPointF() - m_currentPoint;
    QPointF dist;
    dist = (*m_current).point() - (*m_current.previousPivot()).point();
    if ((m_distance >= MINDIST && (fabs(dist.x()) + fabs(dist.y())) > m_distance && !(m_editingMode))
            || m_curve->pivots().count() == 1) {
        draw(m_curve->end());
        m_previous = m_current;
        m_current = m_curve->pushPivot(event->pos().toPointF());
    } else if ((*m_previous).point() == (*m_current).point() && (*m_previous).point() == m_curve->last().point())
        draw(m_curve->end());
    else
        draw(m_current);
    m_curve->movePivot(m_current, event->pos().toPointF());
    m_currentPoint = event->pos().floorQPoint();
    draw(m_current);
}

KisCurve::iterator KisToolMagnetic::selectByMouse(KisCurve::iterator it)
{
    KisCurve::iterator currPivot = m_curve->selectPivot(m_curve->addPivot(it, QPointF(0, 0)));
    m_curve->movePivot(currPivot, (*it).point());

    return currPivot;
}

void KisToolMagnetic::slotCommitCurve()
{
    if (!m_curve->isEmpty())
        commitCurve();
}

void KisToolMagnetic::slotSetDistance(int dist)
{
    m_distance = dist;
}

QWidget* KisToolMagnetic::createOptionWidget()
{
    m_optWidget = KisToolCurve::createOptionWidget(parent);
    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    QGridLayout *box = new QGridLayout(l, 2, 2, 3);
    box->setColStretch(0, 1);
    box->setColStretch(1, 1);
    Q_CHECK_PTR(box);

    m_mode = new QLabel(i18n("Automatic mode"), m_optWidget);
    m_lbDistance = new QLabel(i18n("Distance: "), m_optWidget);
    QPushButton *finish = new QPushButton(i18n("To Selection"), m_optWidget);
    m_slDistance = new QSlider(MINDIST, MAXDIST, PAGESTEP, m_distance, Qt::Horizontal, m_optWidget);

    connect(m_slDistance, SIGNAL(valueChanged(int)), this, SLOT(slotSetDistance(int)));
    connect(finish, SIGNAL(clicked()), this, SLOT(slotCommitCurve()));

    box->addWidget(m_lbDistance, 0, 0);
    box->addWidget(m_slDistance, 0, 1);
    box->addWidget(m_mode, 1, 0);
    box->addWidget(finish, 1, 1);

    return m_optWidget;
}

void KisToolMagnetic::setup(KActionCollection *collection)
{
    m_action = collection->action(objectName());

    if (m_action == 0) {
        m_action = new KAction(KIcon("tool_moutline"),
                               i18n("&Magnetic Outline Selection"),
                               collection,
                               objectName());
        Q_CHECK_PTR(m_action);
        connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
        m_action->setToolTip(i18n("Magnetic Selection: move around an edge to select it. Hit Ctrl to enter/quit manual mode, and double click to finish."));
        m_action->setActionGroup(actionGroup());

        m_ownAction = true;
    }
}

#include "kis_tool_moutline.moc"
