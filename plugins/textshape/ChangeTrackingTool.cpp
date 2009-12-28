/* This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ChangeTrackingTool.h"

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoTextEditor.h>
#include <KoTextShapeData.h>
#include <KoViewConverter.h>
#include "TextShape.h"

#include "commands/AcceptChangeCommand.h"
#include "commands/ShowChangesCommand.h"
#include "dialogs/TrackedChangeModel.h"
#include "dialogs/TrackedChangeManager.h"

#include <KLocale>
#include <KAction>

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QPushButton>
#include <QTextBlock>

ChangeTrackingTool::ChangeTrackingTool(KoCanvasBase* canvas): KoTool(canvas),
    m_textEditor(0),
    m_textShapeData(0),
    m_textShape(0),
    m_model(0)
{
    KAction *action;
    action = new KAction(i18n("Tracked change manager"), this);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_T);
    addAction("show_changeManager", action);
    connect(action, SIGNAL(triggered()), this, SLOT(showTrackedChangeManager()));
}

ChangeTrackingTool::~ChangeTrackingTool()
{

}

void ChangeTrackingTool::mouseReleaseEvent(KoPointerEvent* event)
{
    event->ignore();
}

void ChangeTrackingTool::mouseMoveEvent(KoPointerEvent* event)
{
    event->ignore();
}

void ChangeTrackingTool::mousePressEvent(KoPointerEvent* event)
{
    event->ignore();
}

void ChangeTrackingTool::paint(QPainter& painter, const KoViewConverter& converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
/*    if (m_canvas->canvasWidget()->hasFocus() && !m_caretTimer.isActive()) // make sure we blink
        m_caretTimer.start();
    QTextBlock block = m_textEditor->block();
    if (! block.layout()) // not layouted yet.  The Shape paint method will trigger a layout
        return;
    if (m_textShapeData == 0)
        return;

    int selectStart = m_textEditor->position();
    int selectEnd = m_textEditor->anchor();
    if (selectEnd < selectStart)
        qSwap(selectStart, selectEnd);
    QList<TextShape *> shapesToPaint;
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    if (lay) {
        foreach(KoShape *shape, lay->shapes()) {
            TextShape *ts = dynamic_cast<TextShape*>(shape);
            if (! ts)
                continue;
            KoTextShapeData *data = ts->textShapeData();
            // check if shape contains some of the selection, if not, skip
            if (!( (data->endPosition() >= selectStart && data->position() <= selectEnd)
                || (data->position() <= selectStart && data->endPosition() >= selectEnd)) )
                continue;
            if (painter.hasClipping()) {
                QRect rect = converter.documentToView(ts->boundingRect()).toRect();
                if (painter.clipRegion().intersect(QRegion(rect)).isEmpty())
                    continue;
            }
            shapesToPaint << ts;
        }
    }
    if (shapesToPaint.isEmpty()) // quite unlikely, though ;)
    return;

    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);

    QAbstractTextDocumentLayout::PaintContext pc;
    QAbstractTextDocumentLayout::Selection selection;
    selection.cursor = *(m_textEditor->cursor());
    selection.format.setBackground(m_canvas->canvasWidget()->palette().brush(QPalette::Highlight));
    selection.format.setForeground(m_canvas->canvasWidget()->palette().brush(QPalette::HighlightedText));
    pc.selections.append(selection);
    foreach(TextShape *ts, shapesToPaint) {
        KoTextShapeData *data = ts->textShapeData();
        Q_ASSERT(data);
        if (data->endPosition() == -1)
            continue;

        painter.save();
        QMatrix shapeMatrix = ts->absoluteTransformation(&converter);
        shapeMatrix.scale(zoomX, zoomY);
        painter.setMatrix(shapeMatrix * painter.matrix());
        painter.setClipRect(QRectF(QPointF(), ts->size()), Qt::IntersectClip);
        painter.translate(0, -data->documentOffset());
        if ((data->endPosition() >= selectStart && data->position() <= selectEnd)
            || (data->position() <= selectStart && data->endPosition() >= selectEnd)) {
            QRectF clip = textRect(qMax(data->position(), selectStart), qMin(data->endPosition(), selectEnd));
        painter.save();
        painter.setClipRect(clip, Qt::IntersectClip);
        data->document()->documentLayout()->draw(&painter, pc);
        painter.restore();
        }
        if ((data == m_textShapeData) && m_caretTimerState) {
            // paint caret
            QPen caretPen(Qt::black);
            if (! m_textShape->hasTransparency()) {
                KoColorBackground * fill = dynamic_cast<KoColorBackground*>(m_textShape->background());
                if (fill) {
                    QColor bg = fill->color();
                    QColor invert = QColor(255 - bg.red(), 255 - bg.green(), 255 - bg.blue());
                    caretPen.setColor(invert);
                }
            }
            painter.setPen(caretPen);
            int posInParag = m_textEditor->position() - block.position();
            if (posInParag <= block.layout()->preeditAreaPosition())
                posInParag += block.layout()->preeditAreaText().length();
            block.layout()->drawCursor(&painter, QPointF(), posInParag);
        }

        painter.restore();
    }
*/
}

void ChangeTrackingTool::keyPressEvent(QKeyEvent* event)
{
    KoTool::keyPressEvent(event);
}

void ChangeTrackingTool::activate(bool temporary)
{
    Q_UNUSED(temporary);
    KoSelection *selection = m_canvas->shapeManager()->selection();
    foreach(KoShape *shape, selection->selectedShapes()) {
        m_textShape = dynamic_cast<TextShape*>(shape);
        if (m_textShape)
            break;
    }
    if (m_textShape == 0) { // none found
        emit done();
        return;
    }
    foreach(KoShape *shape, selection->selectedShapes()) {
        // deselect others.
        if (m_textShape == shape) continue;
        selection->deselect(shape);
    }
    setShapeData(static_cast<KoTextShapeData*>(m_textShape->userData()));
    useCursor(Qt::ArrowCursor);


    m_textShape->update();
}

void ChangeTrackingTool::setShapeData(KoTextShapeData *data)
{
    bool docChanged = data == 0 || m_textShapeData == 0 || m_textShapeData->document() != data->document();
    if (m_textShapeData) {
        disconnect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
        if (lay)
            disconnect(lay, SIGNAL(shapeAdded(KoShape*)), this, SLOT(shapeAddedToDoc(KoShape*)));
    }
    m_textShapeData = data;
    if (m_textShapeData == 0)
        return;
    connect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
    if (docChanged) {
        if (m_textEditor)
            disconnect(m_textEditor, SIGNAL(isBidiUpdated()), this, SLOT(isBidiUpdated()));
        m_textEditor = KoTextDocument(m_textShapeData->document()).textEditor();
        Q_ASSERT(m_textEditor);
        connect(m_textEditor, SIGNAL(isBidiUpdated()), this, SLOT(isBidiUpdated()));

        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
        if (lay) {
            connect(lay, SIGNAL(shapeAdded(KoShape*)), this, SLOT(shapeAddedToDoc(KoShape*)));
        }
    }
    m_textEditor->updateDefaultTextDirection(m_textShapeData->pageDirection());
    if (!KoTextDocument(m_textShapeData->document()).changeTracker()->displayChanges()) {
        m_disableShowChangesOnExit = true;
        ShowChangesCommand *command = new ShowChangesCommand(true, m_textShapeData->document());
        m_textEditor->addCommand(command);
    }
    if (m_model)
        delete m_model;
    m_model = new TrackedChangeModel(m_textShapeData->document());
}

void ChangeTrackingTool::deactivate()
{
    m_textShape = 0;
    setShapeData(0);
}

QWidget* ChangeTrackingTool::createOptionWidget()
{
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout;
    QPushButton *accept = new QPushButton(i18n("Accept"));
    QPushButton *reject = new QPushButton(i18n("Reject"));
    layout->addWidget(accept);
    layout->addWidget(reject);
    widget->setLayout(layout);

    connect(accept, SIGNAL(clicked(bool)), this, SLOT(acceptChange()));
    connect(reject, SIGNAL(clicked(bool)), this, SLOT(rejectChange()));
    return widget;
}

void ChangeTrackingTool::acceptChange()
{
    kDebug(32500) << "acceptChange";
    if (m_currentHighlightedChange.isValid()) {
        kDebug(32500) << "highlighted change accepted. id: " << m_model->changeItemData(m_currentHighlightedChange).changeId;
        kDebug(32500) << "change start: " << m_model->changeItemData(m_currentHighlightedChange).changeStart;
        kDebug(32500) << "change end: " << m_model->changeItemData(m_currentHighlightedChange).changeEnd;
        AcceptChangeCommand *command = new AcceptChangeCommand(m_model->changeItemData(m_currentHighlightedChange).changeId,
                                                               m_model->changeItemData(m_currentHighlightedChange).changeStart,
                                                               m_model->changeItemData(m_currentHighlightedChange).changeEnd,
                                                               m_textShapeData->document());
        m_textEditor->addCommand(command);
    }
}

void ChangeTrackingTool::rejectChange()
{
    if (m_currentHighlightedChange.isValid()) {
        kDebug(32500) << "highlighted change accepted. id: " << m_model->changeItemData(m_currentHighlightedChange).changeId;
        kDebug(32500) << "change start: " << m_model->changeItemData(m_currentHighlightedChange).changeStart;
        kDebug(32500) << "change end: " << m_model->changeItemData(m_currentHighlightedChange).changeEnd;
    }
}

void ChangeTrackingTool::selectedChangeChanged(QModelIndex item)
{
    m_currentHighlightedChange = item;
}

void ChangeTrackingTool::showTrackedChangeManager()
{
    Q_ASSERT(!m_model);
    TrackedChangeManager *manager = new TrackedChangeManager();
    manager->setModel(m_model);
    connect(manager, SIGNAL(currentChanged(QModelIndex)), this, SLOT(selectedChangeChanged(QModelIndex)));
    manager->show();
    //    view.setModel(&model);
    //    view.setWindowTitle("testTracked");
    //    view.show();
    //    TrackedChangeManager *dia = new TrackedChangeManager(m_textShapeData->document());
    //    dia->show();
}

#include "ChangeTrackingTool.moc"
