/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
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

#include <kis_ruler_assistant_tool.h>

#include <qpainter.h>

#include <kis_debug.h>
#include <klocale.h>

#include <KoViewConverter.h>
#include <KoPointerEvent.h>

#include <canvas/kis_canvas2.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_view2.h>

#include <kis_painting_assistants_manager.h>

KisRulerAssistantTool::KisRulerAssistantTool(KoCanvasBase * canvas)
        : KisTool(canvas, KisCursor::arrowCursor()), m_canvas(dynamic_cast<KisCanvas2*>(canvas)),
        m_assistantDrag(0), m_newAssistant(0), m_optionsWidget(0)
{
    Q_ASSERT(m_canvas);
    setObjectName("tool_rulerassistanttool");
}

KisRulerAssistantTool::~KisRulerAssistantTool()
{
}

QPointF adjustPointF(const QPointF& _pt, const QRectF& _rc)
{
    return QPointF(qBound(_rc.left(), _pt.x(), _rc.right()), qBound(_rc.top(), _pt.y(), _rc.bottom()));
}

void KisRulerAssistantTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    // Add code here to initialize your tool when it got activated
    KisTool::activate(toolActivation, shapes);

    m_handles = m_canvas->view()->paintingAssistantManager()->handles();
    m_canvas->view()->paintingAssistantManager()->setVisible(true);
    m_canvas->updateCanvas();
    m_handleDrag = 0;
}

void KisRulerAssistantTool::deactivate()
{
    // Add code here to initialize your tool when it got deactivated
    KisTool::deactivate();
}


inline double norm2(const QPointF& p)
{
    return p.x() * p.x() + p.y() * p.y();
}

void KisRulerAssistantTool::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                       Qt::LeftButton, Qt::ShiftModifier)) {

        setMode(KisTool::PAINT_MODE);

        if (m_newAssistant) {
            *m_newAssistant->handles().back() = event->point;
            if (m_newAssistant->handles().size() == m_newAssistant->numHandles()) {
                m_canvas->view()->paintingAssistantManager()->addAssistant(m_newAssistant);
                m_handles = m_canvas->view()->paintingAssistantManager()->handles();
                m_newAssistant = 0;
            } else {
                m_newAssistant->addHandle(new KisPaintingAssistantHandle(event->point));
            }
            m_canvas->updateCanvas();
            return;
        }

        m_handleDrag = 0;
        double minDist = 49.0;

        QPointF mousePos = m_canvas->viewConverter()->documentToView(event->point);

        foreach(const KisPaintingAssistantHandleSP handle, m_handles) {
            double dist = norm2(mousePos - m_canvas->viewConverter()->documentToView(*handle));
            if (dist < minDist) {
                minDist = dist;
                m_handleDrag = handle;
            }
        }
        if (m_handleDrag) {
            if (event->modifiers() & Qt::ShiftModifier) {
                m_handleDrag = m_handleDrag->split()[0];
                m_handles = m_canvas->view()->paintingAssistantManager()->handles();
            }
            m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
            return;
        }
        
        m_assistantDrag = 0;
        foreach(KisPaintingAssistant* assistant, m_canvas->view()->paintingAssistantManager()->assistants()) {
            QPointF iconPosition = m_canvas->viewConverter()->documentToView(assistant->buttonPosition());
            QRectF deleteRect(iconPosition - QPointF(32, 32), QSizeF(16, 16));
            QRectF moveRect(iconPosition - QPointF(16, 16), QSizeF(32, 32));
            if (moveRect.contains(mousePos)) {
                m_assistantDrag = assistant;
                m_mousePosition = event->point;
                return;
            }
            if (deleteRect.contains(mousePos)) {
                m_canvas->view()->paintingAssistantManager()->removeAssistant(assistant);
                m_handles = m_canvas->view()->paintingAssistantManager()->handles();
                m_canvas->updateCanvas();
                return;
            }
        }

        // create new assistant
        QString key = m_options.comboBox->model()->index( m_options.comboBox->currentIndex(), 0 ).data(Qt::UserRole).toString();
        QRectF imageArea = QRectF(pixelToView(QPoint(0, 0)),
                                  m_canvas->image()->pixelToDocument(QPoint(m_canvas->image()->width(), m_canvas->image()->height())));
        m_newAssistant = KisPaintingAssistantFactoryRegistry::instance()->get(key)->paintingAssistant(imageArea);
        m_newAssistant->addHandle(new KisPaintingAssistantHandle(event->point));
        if (m_newAssistant->numHandles() <= 1) {
            m_canvas->view()->paintingAssistantManager()->addAssistant(m_newAssistant);
            m_handles = m_canvas->view()->paintingAssistantManager()->handles();
        } else {
            m_newAssistant->addHandle(new KisPaintingAssistantHandle(event->point));
        }
        m_canvas->updateCanvas();
    } else {
        KisTool::mousePressEvent(event);
    }
}


void KisRulerAssistantTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_newAssistant) {
        *m_newAssistant->handles().back() = event->point;
        m_canvas->updateCanvas();
    } else if(MOVE_CONDITION(event, KisTool::PAINT_MODE)) {
        if (m_handleDrag) {
            *m_handleDrag = event->point;

            m_handleCombine = 0;
            if (!(event->modifiers() & Qt::ShiftModifier)) {
                double minDist = 49.0;
                QPointF mousePos = m_canvas->viewConverter()->documentToView(event->point);
                foreach(const KisPaintingAssistantHandleSP handle, m_handles) {
                    if (handle == m_handleDrag) continue;
                    double dist = norm2(mousePos - m_canvas->viewConverter()->documentToView(*handle));
                    if (dist < minDist) {
                        minDist = dist;
                        m_handleCombine = handle;
                    }
                }
            }

            m_canvas->updateCanvas();
        } else if (m_assistantDrag) {
            QPointF adjust = event->point - m_mousePosition;
            foreach(KisPaintingAssistantHandleSP handle, m_assistantDrag->handles()) {
                *handle += adjust;
            }
            m_mousePosition = event->point;
            m_canvas->updateCanvas();
        } else {
            event->ignore();
        }
    }
    else {
        KisTool::mouseMoveEvent(event);
    }
}

void KisRulerAssistantTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);

        if (m_handleDrag) {
            if (!(event->modifiers() & Qt::ShiftModifier) && m_handleCombine) {
                m_handleCombine->mergeWith(m_handleDrag);
                m_handles = m_canvas->view()->paintingAssistantManager()->handles();
            }
            m_handleDrag = m_handleCombine = 0;
            m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
        } else if (m_assistantDrag) {
            m_assistantDrag = 0;
            m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
        } else {
            event->ignore();
        }
    }
    else {
        KisTool::mouseReleaseEvent(event);
    }
}

void KisRulerAssistantTool::paint(QPainter& _gc, const KoViewConverter &_converter)
{
    QColor handlesColor(0, 0, 0, 125);

    if (m_newAssistant) {
        m_newAssistant->drawAssistant(_gc, QRectF(QPointF(0, 0), QSizeF(m_canvas->image()->size())), m_canvas->coordinatesConverter());
        _gc.setPen(handlesColor);
        _gc.setBrush(Qt::transparent);
        foreach(const KisPaintingAssistantHandleSP handle, m_newAssistant->handles()) {
            _gc.drawEllipse(QRectF(_converter.documentToView(*handle) -  QPointF(6, 6), QSizeF(12, 12)));
        }
    }

    foreach(const KisPaintingAssistantHandleSP handle, m_handles) {
        if (handle == m_handleDrag || handle == m_handleCombine) {
            _gc.setPen(handlesColor);
            _gc.setBrush(handlesColor);
        } else {
            _gc.setPen(handlesColor);
            _gc.setBrush(Qt::transparent);
        }
        _gc.drawEllipse(QRectF(_converter.documentToView(*handle) -  QPointF(6, 6), QSizeF(12, 12)));
    }
    
    QPixmap iconDelete = KIcon("edit-delete").pixmap(16, 16);
    QPixmap iconMove = KIcon("transform-move").pixmap(32, 32);
    foreach(const KisPaintingAssistant* assistant, m_canvas->view()->paintingAssistantManager()->assistants()) {
        QPointF iconDeletePos = _converter.documentToView(assistant->buttonPosition());
        _gc.drawPixmap(iconDeletePos - QPointF(32, 32), iconDelete);
        _gc.drawPixmap(iconDeletePos - QPointF(16, 16), iconMove);
    }
}

void KisRulerAssistantTool::removeAllAssistants()
{
    m_canvas->view()->paintingAssistantManager()->removeAll();
    m_handles = m_canvas->view()->paintingAssistantManager()->handles();
    m_canvas->updateCanvas();
}

QWidget *KisRulerAssistantTool::createOptionWidget()
{
    if (!m_optionsWidget) {
        m_optionsWidget = new QWidget;
        m_options.setupUi(m_optionsWidget);
        foreach(const QString& key, KisPaintingAssistantFactoryRegistry::instance()->keys()) {
            QString name = KisPaintingAssistantFactoryRegistry::instance()->get(key)->name();
            m_options.comboBox->addItem(name, key);
        }
        connect(m_options.deleteButton, SIGNAL(clicked()), SLOT(removeAllAssistants()));
    }
    return m_optionsWidget;
}

#include "kis_ruler_assistant_tool.moc"
