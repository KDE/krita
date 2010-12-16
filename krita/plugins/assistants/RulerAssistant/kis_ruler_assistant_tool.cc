/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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
        : KisTool(canvas, KisCursor::arrowCursor()), m_canvas(dynamic_cast<KisCanvas2*>(canvas)), m_optionsWidget(0)
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
    return sqrt(p.x() * p.x() + p.y() * p.y());
}

void KisRulerAssistantTool::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION(event, KisTool::HOVER_MODE,
                       Qt::LeftButton, Qt::NoModifier)) {

        setMode(KisTool::PAINT_MODE);

        m_handleDrag = 0;
        foreach(const KisPaintingAssistantHandleSP handle, m_handles) {
            if (norm2(event->point - *handle) < 10) {
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                m_handleDrag = handle;
                return;
            }
        }
        foreach(KisPaintingAssistant* assistant, m_canvas->view()->paintingAssistantManager()->assistants()) {
            QPointF iconDeletePos = assistant->deletePosition();
            if (QRectF(iconDeletePos - QPointF(16, 16), iconDeletePos + QPointF(16, 16)).contains(event->point)) {
                m_canvas->view()->paintingAssistantManager()->removeAssistant(assistant);
                m_handles = m_canvas->view()->paintingAssistantManager()->handles();
                m_canvas->updateCanvas();
                return;
            }
        }
        event->ignore();
    }
    else {
        KisTool::mousePressEvent(event);
    }
}


void KisRulerAssistantTool::mouseMoveEvent(KoPointerEvent *event)
{
    if(MOVE_CONDITION(event, KisTool::PAINT_MODE)) {
        if (m_handleDrag) {
            *m_handleDrag = event->point;
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
            m_handleDrag = 0;
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

    foreach(const KisPaintingAssistantHandleSP handle, m_handles) {
        if (handle == m_handleDrag) {
            _gc.setPen(handlesColor);
            _gc.setBrush(handlesColor);
        } else {
            _gc.setPen(handlesColor);
            _gc.setBrush(Qt::transparent);
        }
        _gc.drawEllipse(QRectF(_converter.documentToView(*handle) -  QPointF(5, 5), QSizeF(10, 10)));
    }

    KIcon iconDelete("edit-delete");
    foreach(const KisPaintingAssistant* assistant, m_canvas->view()->paintingAssistantManager()->assistants()) {
        QPointF iconDeletePos = _converter.documentToView(assistant->deletePosition());
        _gc.drawPixmap(iconDeletePos - QPointF(16, 16), iconDelete.pixmap(32, 32));
    }
}

void KisRulerAssistantTool::createNewAssistant()
{
    QString key = m_options.comboBox->model()->index( m_options.comboBox->currentIndex(), 0 ).data(Qt::UserRole).toString();
    dbgPlugins << ppVar(key) << m_options.comboBox->view()->currentIndex().row() << ppVar(m_options.comboBox->currentText());
    QRectF imageArea = QRectF(pixelToView(QPoint(0, 0)),
                              m_canvas->image()->pixelToDocument(QPoint(m_canvas->image()->width(), m_canvas->image()->height())));
    KisPaintingAssistant* assistant = KisPaintingAssistantFactoryRegistry::instance()->get(key)->paintingAssistant(imageArea);
    m_canvas->view()->paintingAssistantManager()->addAssistant(assistant);
    m_handles = m_canvas->view()->paintingAssistantManager()->handles();
    m_canvas->updateCanvas();
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
        m_options.toolButton->setIcon(KIcon("document-new"));
        foreach(const QString& key, KisPaintingAssistantFactoryRegistry::instance()->keys()) {
            QString name = KisPaintingAssistantFactoryRegistry::instance()->get(key)->name();
            m_options.comboBox->addItem(name, key);
        }
        connect(m_options.toolButton, SIGNAL(clicked()), SLOT(createNewAssistant()));
        connect(m_options.deleteButton, SIGNAL(clicked()), SLOT(removeAllAssistants()));
    }
    return m_optionsWidget;
}

#include "kis_ruler_assistant_tool.moc"
