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

#include <QPainter>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <kis_debug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kio/jobuidelegate.h>

#include <KoIcon.h>

#include <KoViewConverter.h>
#include <KoPointerEvent.h>

#include <canvas/kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_view2.h>

#include <kis_abstract_perspective_grid.h>
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
    m_canvas->updateCanvas();
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
                addAssistant();
            } else {
                m_newAssistant->addHandle(new KisPaintingAssistantHandle(event->point));
            }
            m_canvas->updateCanvas();
            return;
        }

        m_handleDrag = 0;
        double minDist = 81.0;

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
                m_handleDrag->uncache();
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
                removeAssistant(assistant);
                m_canvas->updateCanvas();
                return;
            }
        }

        // create new assistant
        QString key = m_options.comboBox->model()->index( m_options.comboBox->currentIndex(), 0 ).data(Qt::UserRole).toString();

        m_newAssistant = KisPaintingAssistantFactoryRegistry::instance()->get(key)->createPaintingAssistant();
        m_newAssistant->addHandle(new KisPaintingAssistantHandle(event->point));
        if (m_newAssistant->numHandles() <= 1) {
            addAssistant();
        } else {
            m_newAssistant->addHandle(new KisPaintingAssistantHandle(event->point));
        }
        m_canvas->updateCanvas();
    } else {
        KisTool::mousePressEvent(event);
    }
}


void KisRulerAssistantTool::addAssistant()
{
    m_canvas->view()->paintingAssistantManager()->addAssistant(m_newAssistant);
    m_handles = m_canvas->view()->paintingAssistantManager()->handles();
    KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(m_newAssistant);
    if (grid) {
        m_canvas->view()->resourceProvider()->addPerspectiveGrid(grid);
    }
    m_newAssistant = 0;
}


void KisRulerAssistantTool::removeAssistant(KisPaintingAssistant* assistant)
{
    KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(assistant);
    if (grid) {
        m_canvas->view()->resourceProvider()->removePerspectiveGrid(grid);
    }
    m_canvas->view()->paintingAssistantManager()->removeAssistant(assistant);
    m_handles = m_canvas->view()->paintingAssistantManager()->handles();
}


void KisRulerAssistantTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_newAssistant) {
        *m_newAssistant->handles().back() = event->point;
        m_canvas->updateCanvas();
    } else if(MOVE_CONDITION(event, KisTool::PAINT_MODE)) {
        if (m_handleDrag) {
            *m_handleDrag = event->point;
            m_handleDrag->uncache();

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
                m_handleCombine->uncache();
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
        m_newAssistant->drawAssistant(_gc, QRectF(QPointF(0, 0), QSizeF(m_canvas->image()->size())), m_canvas->coordinatesConverter(), false);
        foreach(const KisPaintingAssistantHandleSP handle, m_newAssistant->handles()) {
            QPainterPath path;
            path.addEllipse(QRectF(_converter.documentToView(*handle) -  QPointF(6, 6), QSizeF(12, 12)));
            KisPaintingAssistant::drawPath(_gc, path);
        }
    }

    foreach(const KisPaintingAssistantHandleSP handle, m_handles) {
        QRectF ellipse(_converter.documentToView(*handle) -  QPointF(6, 6), QSizeF(12, 12));
        if (handle == m_handleDrag || handle == m_handleCombine) {
            _gc.save();
            _gc.setPen(Qt::transparent);
            _gc.setBrush(handlesColor);
            _gc.drawEllipse(ellipse);
            _gc.restore();
        }
        QPainterPath path;
        path.addEllipse(ellipse);
        KisPaintingAssistant::drawPath(_gc, path);        
    }
    
    QPixmap iconDelete = koIcon("edit-delete").pixmap(16, 16);
    QPixmap iconMove = koIcon("transform-move").pixmap(32, 32);
    foreach(const KisPaintingAssistant* assistant, m_canvas->view()->paintingAssistantManager()->assistants()) {
        QPointF iconDeletePos = _converter.documentToView(assistant->buttonPosition());
        _gc.drawPixmap(iconDeletePos - QPointF(32, 32), iconDelete);
        _gc.drawPixmap(iconDeletePos - QPointF(16, 16), iconMove);
    }
}

void KisRulerAssistantTool::removeAllAssistants()
{
    m_canvas->view()->resourceProvider()->clearPerspectiveGrids();
    m_canvas->view()->paintingAssistantManager()->removeAll();
    m_handles = m_canvas->view()->paintingAssistantManager()->handles();
    m_canvas->updateCanvas();
}

void KisRulerAssistantTool::loadAssistants()
{
    KUrl file = KFileDialog::getOpenUrl(KUrl(), QString("*.krassistants"));
    if (file.isEmpty()) return;
    KIO::StoredTransferJob* job = KIO::storedGet(file);
    connect(job, SIGNAL(result(KJob*)), SLOT(openFinish(KJob*)));
    job->start();
}

void KisRulerAssistantTool::saveAssistants()
{
    QByteArray data;
    QXmlStreamWriter xml(&data);
    xml.writeStartDocument();
    xml.writeStartElement("paintingassistant");
    xml.writeStartElement("handles");
    QMap<KisPaintingAssistantHandleSP, int> handleMap;
    foreach(const KisPaintingAssistantHandleSP handle, m_handles) {
        int id = handleMap.size();
        handleMap.insert(handle, id);
        xml.writeStartElement("handle");
        xml.writeAttribute("id", QString::number(id));
        xml.writeAttribute("x", QString::number(double(handle->x()), 'f', 3));
        xml.writeAttribute("y", QString::number(double(handle->y()), 'f', 3));
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeStartElement("assistants");
    foreach(const KisPaintingAssistant* assistant, m_canvas->view()->paintingAssistantManager()->assistants()) {
        xml.writeStartElement("assistant");
        xml.writeAttribute("type", assistant->id());
        xml.writeStartElement("handles");
        foreach(const KisPaintingAssistantHandleSP handle, assistant->handles()) {
            xml.writeStartElement("handle");
            xml.writeAttribute("ref", QString::number(handleMap.value(handle)));
            xml.writeEndElement();
        }
        xml.writeEndElement();
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndDocument();

    KUrl file = KFileDialog::getSaveUrl(KUrl(), QString("*.krassistants"));
    if (file.isEmpty()) return;
    KIO::StoredTransferJob* job = KIO::storedPut(data, file, -1);
    connect(job, SIGNAL(result(KJob*)), SLOT(saveFinish(KJob*)));
    job->start();
}

void KisRulerAssistantTool::openFinish(KJob* job)
{
    job->deleteLater();
    if (job->error()) {
        dynamic_cast<KIO::Job*>(job)->ui()->showErrorMessage();
        return;
    }
    QByteArray data = dynamic_cast<KIO::StoredTransferJob*>(job)->data();
    QXmlStreamReader xml(data);
    QMap<int, KisPaintingAssistantHandleSP> handleMap;
    KisPaintingAssistant* assistant = 0;
    bool errors = false;
    while (!xml.atEnd()) {
        switch (xml.readNext()) {
        case QXmlStreamReader::StartElement:
            if (xml.name() == "handle") {
                if (assistant && !xml.attributes().value("ref").isEmpty()) {
                    KisPaintingAssistantHandleSP handle = handleMap.value(xml.attributes().value("ref").toString().toInt());
                    if (handle) {
                        assistant->addHandle(handle);
                    } else {
                        errors = true;
                    }
                } else {
                    QString strId = xml.attributes().value("id").toString(),
                            strX = xml.attributes().value("x").toString(),
                            strY = xml.attributes().value("y").toString();
                    if (!strId.isEmpty() && !strX.isEmpty() && !strY.isEmpty()) {
                        int id = strId.toInt();
                        double x = strX.toDouble(),
                               y = strY.toDouble();
                        if (!handleMap.contains(id)) {
                            handleMap.insert(id, new KisPaintingAssistantHandle(x, y));
                        } else {
                            errors = true;
                        }
                    } else {
                        errors = true;
                    }
                }
            } else if (xml.name() == "assistant") {
                const KisPaintingAssistantFactory* factory = KisPaintingAssistantFactoryRegistry::instance()->get(xml.attributes().value("type").toString());
                if (factory) {
                    if (assistant) {
                        errors = true;
                        delete assistant;
                    }
                    assistant = factory->createPaintingAssistant();
                } else {
                    errors = true;
                }
            }
            break;
        case QXmlStreamReader::EndElement:
            if (xml.name() == "assistant") {
                if (assistant) {
                    if (assistant->handles().size() == assistant->numHandles()) {
                        m_canvas->view()->paintingAssistantManager()->addAssistant(assistant);
                        KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(assistant);
                        if (grid) {
                            m_canvas->view()->resourceProvider()->addPerspectiveGrid(grid);
                        }
                    } else {
                        errors = true;
                        delete assistant;
                    }
                    assistant = 0;
                }
            }
            break;
        default:
            break;
        }
    }
    if (assistant) {
        errors = true;
        delete assistant;
    }
    if (xml.hasError()) {
        KMessageBox::sorry(0, xml.errorString());
    }
    if (errors) {
        KMessageBox::sorry(0, i18n("Errors were encountered. Not all assistants were successfully loaded."));
    }
    m_handles = m_canvas->view()->paintingAssistantManager()->handles();
    m_canvas->updateCanvas();
}

void KisRulerAssistantTool::saveFinish(KJob* job)
{
    if (job->error()) {
        dynamic_cast<KIO::Job*>(job)->ui()->showErrorMessage();
    }
    job->deleteLater();
}

QWidget *KisRulerAssistantTool::createOptionWidget()
{
    if (!m_optionsWidget) {
        m_optionsWidget = new QWidget;
        m_options.setupUi(m_optionsWidget);
        m_options.loadButton->setIcon(koIcon("document-open"));
        m_options.saveButton->setIcon(koIcon("document-save"));
        m_options.deleteButton->setIcon(koIcon("edit-delete"));
        foreach(const QString& key, KisPaintingAssistantFactoryRegistry::instance()->keys()) {
            QString name = KisPaintingAssistantFactoryRegistry::instance()->get(key)->name();
            m_options.comboBox->addItem(name, key);
        }
        connect(m_options.saveButton, SIGNAL(clicked()), SLOT(saveAssistants()));
        connect(m_options.loadButton, SIGNAL(clicked()), SLOT(loadAssistants()));
        connect(m_options.deleteButton, SIGNAL(clicked()), SLOT(removeAllAssistants()));
    }
    return m_optionsWidget;
}

#include "kis_ruler_assistant_tool.moc"
