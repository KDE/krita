/* This file is part of the KDE project
 * Copyright (C) 2009-2010 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "ReviewTool.h"

#include <KoCanvasBase.h>
#include <KoTextLayoutRootArea.h>
#include <KoChangeTracker.h>
#include <KoPointerEvent.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoTextEditor.h>
#include <KoTextShapeData.h>
#include <KoViewConverter.h>
#include <KoGlobal.h>

#include "TextShape.h"

#include "commands/AcceptChangeCommand.h"
#include "commands/RejectChangeCommand.h"
#include "commands/ShowChangesCommand.h"
#include "dialogs/TrackedChangeModel.h"
#include "dialogs/TrackedChangeManager.h"
#include "dialogs/AcceptRejectChangeDialog.h"
#include "dialogs/ChangeConfigureDialog.h"

#include <KLocale>
#include <KAction>
#include <KUser>

#include <QHBoxLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QKeyEvent>
#include <QModelIndex>
#include <QPainter>
#include <QPushButton>
#include <QTextBlock>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVector>
#include <QLabel>

ReviewTool::ReviewTool(KoCanvasBase* canvas): KoToolBase(canvas),
    m_textEditor(0),
    m_textShapeData(0),
    m_canvas(canvas),
    m_textShape(0),
    m_model(0),
    m_trackedChangeManager(0),
    m_changesTreeView(0)
{
    KAction *action;
    action = new KAction(i18n("Tracked change manager"), this);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_T);
    addAction("show_changeManager", action);
    connect(action, SIGNAL(triggered()), this, SLOT(showTrackedChangeManager()));

    m_actionShowChanges = new KAction(i18n("Show Changes"), this);
    m_actionShowChanges->setCheckable(true);
    addAction("edit_show_changes", m_actionShowChanges);
    connect(m_actionShowChanges, SIGNAL(triggered(bool)), this, SLOT(toggleShowChanges(bool)));

    m_actionRecordChanges = new KAction(i18n("Record Changes"), this);
    m_actionRecordChanges->setCheckable(true);
    addAction("edit_record_changes", m_actionRecordChanges);
    connect(m_actionRecordChanges, SIGNAL(triggered(bool)), this, SLOT(toggleRecordChanges(bool)));

    m_configureChangeTracking = new KAction(i18n("Configure Change Tracking..."), this);
    addAction("configure_change_tracking", m_configureChangeTracking);
    connect(m_configureChangeTracking, SIGNAL(triggered()), this, SLOT(configureChangeTracking()));
}

ReviewTool::~ReviewTool()
{
    delete m_trackedChangeManager;
    delete m_model;
}

void ReviewTool::mouseReleaseEvent(KoPointerEvent* event)
{
    event->ignore();
}

void ReviewTool::mouseMoveEvent(KoPointerEvent* event)
{
    updateSelectedShape(event->point);
    int position = pointToPosition(event->point);
    QTextCursor cursor(m_textShapeData->document());
    cursor.setPosition(position);

    int changeId = cursor.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
    if (changeId) {
        m_canvas->setCursor(QCursor(Qt::PointingHandCursor));
        QModelIndex index = m_model->indexForChangeId(changeId);
        m_changesTreeView->setCurrentIndex(index);
    } else {
        m_canvas->setCursor(QCursor(Qt::ArrowCursor));
        m_changesTreeView->setCurrentIndex(QModelIndex());
    }
}

void ReviewTool::mousePressEvent(KoPointerEvent* event)
{
    int position = pointToPosition(event->point);
    QTextCursor cursor(m_textShapeData->document());
    cursor.setPosition(position);

    int changeId = cursor.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
    if (changeId) {
        AcceptRejectChangeDialog acceptDialog(KoTextDocument(m_textShapeData->document()).changeTracker(), changeId);
        if (int result = acceptDialog.exec()) {
            if (result == (int)(AcceptRejectChangeDialog::eChangeAccepted)) {
                acceptChange();
            } else if (result == (int)(AcceptRejectChangeDialog::eChangeRejected)) {
                rejectChange();
            }
        }
    }
}

void ReviewTool::updateSelectedShape(const QPointF &point)
{
    if (! m_textShape->boundingRect().contains(point)) {
        QRectF area(point, QSizeF(1, 1));
        foreach(KoShape *shape, canvas()->shapeManager()->shapesAt(area, true)) {
            TextShape *textShape = dynamic_cast<TextShape*>(shape);
            if (textShape) {
                KoTextShapeData *d = static_cast<KoTextShapeData*>(textShape->userData());
                const bool sameDocument = d->document() == m_textShapeData->document();
                    m_textShape = textShape;
                if (sameDocument)
                    break; // stop looking.
            }
        }
        setShapeData(static_cast<KoTextShapeData*>(m_textShape->userData()));
    }
}

int ReviewTool::pointToPosition(const QPointF & point) const
{
    QPointF p = m_textShape->convertScreenPos(point);
    int caretPos = m_textShapeData->rootArea()->hitTest(p, Qt::FuzzyHit).position;
    return caretPos;
}

void ReviewTool::paint(QPainter& painter, const KoViewConverter& converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    QTextBlock block = m_textEditor->block();
    if (! block.layout()) // not layouted yet.  The Shape paint method will trigger a layout
        return;
    if (m_textShapeData == 0)
        return;

    if (!m_changesTreeView->currentIndex().isValid())
        return;

    QList<QPair<int, int> > changeRanges = m_model->changeItemData(m_changesTreeView->currentIndex()).changeRanges;

    for (int i = 0; i < changeRanges.size(); ++i) {
        int start = changeRanges.at(i).first;
        int end = changeRanges.at(i).second;
        if (end < start)
            qSwap(start, end);
        QTextCursor cursor;
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        QList<TextShape *> shapesToPaint;
        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
        if (lay) {
            foreach(KoShape *shape, lay->shapes()) {
                TextShape *ts = dynamic_cast<TextShape*>(shape);
                if (! ts)
                    continue;
                KoTextShapeData *data = ts->textShapeData();

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

        foreach(TextShape *ts, shapesToPaint) {
            KoTextShapeData *data = ts->textShapeData();
            Q_ASSERT(data);
            if (data->isDirty())
                continue;

            painter.save();
            QTransform shapeMatrix = ts->absoluteTransformation(&converter);
            shapeMatrix.scale(zoomX, zoomY);
            painter.setTransform(shapeMatrix * painter.transform());
            painter.setClipRect(QRectF(QPointF(), ts->size()), Qt::IntersectClip);
            painter.translate(0, -data->documentOffset());
#if 0 //FIXME refactor to new textlayout
            if (data->isCursorVisible(&cursor)) {
                QVector<QRectF> *clipVec = textRect(cursor);
                QRectF clip;
                foreach(clip, *clipVec) {
                    painter.save();
                    painter.setClipRect(clip, Qt::IntersectClip);
                    painter.fillRect(clip, QBrush(QColor(0,0,0,90)));
                    painter.restore();
                }
                delete clipVec;
            }
#endif

            painter.restore();
        }
    }
}


QRectF ReviewTool::textRect(QTextCursor &cursor) const
{
    if (!m_textShapeData)
        return QRectF();
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    return lay->selectionBoundingBox(cursor);
}

void ReviewTool::keyPressEvent(QKeyEvent* event)
{
    KoToolBase::keyPressEvent(event);
}

void ReviewTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    Q_UNUSED(toolActivation);
    foreach(KoShape *shape, shapes) {
        m_textShape = dynamic_cast<TextShape*>(shape);
        if (m_textShape)
            break;
    }
    if (m_textShape == 0) { // none found
        emit done();
        return;
    }
    setShapeData(static_cast<KoTextShapeData*>(m_textShape->userData()));
    useCursor(Qt::ArrowCursor);


    m_textShape->update();

    readConfig();
}

void ReviewTool::setShapeData(KoTextShapeData *data)
{
    bool docChanged = data == 0 || m_textShapeData == 0 || m_textShapeData->document() != data->document();
/*
    if (m_textShapeData) {
//        disconnect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
//        if (lay)
//            disconnect(lay, SIGNAL(shapeAdded(KoShape*)), this, SLOT(shapeAddedToDoc(KoShape*)));
    }
*/
    m_textShapeData = data;
    if (m_textShapeData == 0)
        return;
//    connect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
    if (docChanged) {
//        if (m_textEditor)
//            disconnect(m_textEditor, SIGNAL(isBidiUpdated()), this, SLOT(isBidiUpdated()));
        m_textEditor = KoTextDocument(m_textShapeData->document()).textEditor();
        Q_ASSERT(m_textEditor);
//        connect(m_textEditor, SIGNAL(isBidiUpdated()), this, SLOT(isBidiUpdated()));

        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
        if (lay) {
//            connect(lay, SIGNAL(shapeAdded(KoShape*)), this, SLOT(shapeAddedToDoc(KoShape*)));
        }
    }

    if (m_model) {
        disconnect(m_changesTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectedChangeChanged(QModelIndex,QModelIndex)));
        delete m_model;
    }
    m_model = new TrackedChangeModel(m_textShapeData->document());
    if (m_changesTreeView) {
        QItemSelectionModel *m = m_changesTreeView->selectionModel();
        m_changesTreeView->setModel(m_model);
        delete m;
        connect(m_changesTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectedChangeChanged(QModelIndex,QModelIndex)));
        m_changesTreeView->reset();
    }

    m_changeTracker = KoTextDocument(m_textShapeData->document()).changeTracker();
}

void ReviewTool::deactivate()
{
    m_textShape = 0;
    setShapeData(0);
    canvas()->canvasWidget()->setFocus();
}

QList<QWidget*> ReviewTool::createOptionWidgets()
{
    QList<QWidget *> widgets;
    QWidget *widget = new QWidget();
    widget->setObjectName("hmm");

    m_changesTreeView = new QTreeView(widget);
    m_changesTreeView->setModel(m_model);
    m_changesTreeView->setColumnHidden(0, true);
    connect(m_changesTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectedChangeChanged(QModelIndex,QModelIndex)));

    QVBoxLayout *vLayout = new QVBoxLayout(widget);
    vLayout->addWidget(m_changesTreeView);
    QHBoxLayout *hLayout = new QHBoxLayout;
    QPushButton *accept = new QPushButton(i18n("Accept"));
    QPushButton *reject = new QPushButton(i18n("Reject"));
    hLayout->addWidget(accept);
    hLayout->addWidget(reject);
    vLayout->addLayout(hLayout);
    QCheckBox *showChanges = new QCheckBox(i18n("Show Changes"));
    vLayout->addWidget(showChanges);
    QCheckBox *recordChanges = new QCheckBox(i18n("Record Changes"));
    vLayout->addWidget(recordChanges);
    QToolButton *configureTracing = new QToolButton();
    configureTracing->setDefaultAction(action("configure_change_tracking"));
    vLayout->addWidget(configureTracing);

    connect(m_actionShowChanges, SIGNAL(triggered(bool)), showChanges, SLOT(setChecked(bool)));
    connect(m_actionRecordChanges, SIGNAL(triggered(bool)), recordChanges, SLOT(setChecked(bool)));
    connect(showChanges, SIGNAL(clicked(bool)), this, SLOT(toggleShowChanges(bool)));
    connect(recordChanges, SIGNAL(clicked(bool)), this, SLOT(toggleRecordChanges(bool)));
    connect(accept, SIGNAL(clicked(bool)), this, SLOT(acceptChange()));
    connect(reject, SIGNAL(clicked(bool)), this, SLOT(rejectChange()));

    widget->setWindowTitle(i18n("Changes"));
    widgets.append(widget);
    QWidget *dummy = new QWidget();
    dummy->setObjectName("dummy1");
    dummy->setWindowTitle(i18n("Spell checking"));
    widgets.append(dummy);
    dummy = new QWidget();
    dummy->setObjectName("dummy2");
    dummy->setWindowTitle(i18n("Comments"));
    widgets.append(dummy);
    return widgets;
}

void ReviewTool::acceptChange()
{
    if (m_changesTreeView->currentIndex().isValid()) {
        AcceptChangeCommand *command = new AcceptChangeCommand(m_model->changeItemData(m_changesTreeView->currentIndex()).changeId,
                                                               m_model->changeItemData(m_changesTreeView->currentIndex()).changeRanges,
                                                                m_textShapeData->document());
        connect(command, SIGNAL(acceptRejectChange()), m_model, SLOT(setupModel()));
        m_textEditor->addCommand(command);
    }
}

void ReviewTool::rejectChange()
{
    if (m_changesTreeView->currentIndex().isValid()) {
        RejectChangeCommand *command = new RejectChangeCommand(m_model->changeItemData(m_changesTreeView->currentIndex()).changeId,
                                                               m_model->changeItemData(m_changesTreeView->currentIndex()).changeRanges,
                                                               m_textShapeData->document());
        connect(command, SIGNAL(acceptRejectChange()), m_model, SLOT(setupModel()));
        m_textEditor->addCommand(command);
    }
}

void ReviewTool::selectedChangeChanged(QModelIndex newItem, QModelIndex previousItem)
{
    Q_UNUSED(newItem);
    Q_UNUSED(previousItem);
    canvas()->updateCanvas(m_textShape->boundingRect());
}

void ReviewTool::toggleShowChanges(bool on)//TODO transfer this in KoTextEditor
{
    m_actionShowChanges->setChecked(on);
    ShowChangesCommand *command = new ShowChangesCommand(on, m_textShapeData->document(), this->canvas());
    connect(command, SIGNAL(toggledShowChange(bool)), m_actionShowChanges, SLOT(setChecked(bool)));
    m_textEditor->addCommand(command);
}

void ReviewTool::toggleRecordChanges(bool on)
{
    m_actionRecordChanges->setChecked(on);
    if (m_changeTracker)
        m_changeTracker->setRecordChanges(on);
}

void ReviewTool::configureChangeTracking()
{
    if (m_changeTracker) {
        QColor insertionBgColor, deletionBgColor, formatChangeBgColor;
        insertionBgColor = m_changeTracker->getInsertionBgColor();
        deletionBgColor = m_changeTracker->getDeletionBgColor();
        formatChangeBgColor = m_changeTracker->getFormatChangeBgColor();
        QString authorName = m_changeTracker->authorName();
        KoChangeTracker::ChangeSaveFormat changeSaveFormat = m_changeTracker->saveFormat();

        ChangeConfigureDialog changeDialog(insertionBgColor, deletionBgColor, formatChangeBgColor, authorName, changeSaveFormat, canvas()->canvasWidget());

        if (changeDialog.exec()) {
            m_changeTracker->setInsertionBgColor(changeDialog.getInsertionBgColor());
            m_changeTracker->setDeletionBgColor(changeDialog.getDeletionBgColor());
            m_changeTracker->setFormatChangeBgColor(changeDialog.getFormatChangeBgColor());
            m_changeTracker->setAuthorName(changeDialog.authorName());
            m_changeTracker->setSaveFormat(changeDialog.saveFormat());
            writeConfig();
        }
    }
}


void ReviewTool::readConfig()
{
    if (m_changeTracker) {
        QColor bgColor, defaultColor;
        QString changeAuthor;
        int changeSaveFormat = KoChangeTracker::DELTAXML;
        KConfigGroup interface = KoGlobal::calligraConfig()->group("Change-Tracking");
        if (interface.exists()) {
            bgColor = interface.readEntry("insertionBgColor", defaultColor);
            m_changeTracker->setInsertionBgColor(bgColor);
            bgColor = interface.readEntry("deletionBgColor", defaultColor);
            m_changeTracker->setDeletionBgColor(bgColor);
            bgColor = interface.readEntry("formatChangeBgColor", defaultColor);
            m_changeTracker->setFormatChangeBgColor(bgColor);
            changeAuthor = interface.readEntry("changeAuthor", changeAuthor);
            if (changeAuthor == "") {
                KUser user(KUser::UseRealUserID);
                m_changeTracker->setAuthorName(user.property(KUser::FullName).toString());
            } else {
                m_changeTracker->setAuthorName(changeAuthor);
            }
            changeSaveFormat = interface.readEntry("changeSaveFormat", changeSaveFormat);
            m_changeTracker->setSaveFormat((KoChangeTracker::ChangeSaveFormat)(changeSaveFormat));
        }
    }
}

void ReviewTool::writeConfig()
{
    if (m_changeTracker) {
        KConfigGroup interface = KoGlobal::calligraConfig()->group("Change-Tracking");
        interface.writeEntry("insertionBgColor", m_changeTracker->getInsertionBgColor());
        interface.writeEntry("deletionBgColor", m_changeTracker->getDeletionBgColor());
        interface.writeEntry("formatChangeBgColor", m_changeTracker->getFormatChangeBgColor());
        KUser user(KUser::UseRealUserID);
        QString changeAuthor = m_changeTracker->authorName();
        if (changeAuthor != user.property(KUser::FullName).toString()) {
            interface.writeEntry("changeAuthor", changeAuthor);
        }
        interface.writeEntry("changeSaveFormat", (int)(m_changeTracker->saveFormat()));
    }
}


void ReviewTool::showTrackedChangeManager()
{
/*    Q_ASSERT(m_model);
    m_trackedChangeManager = new TrackedChangeManager();
    m_trackedChangeManager->setModel(m_model);
    connect(m_trackedChangeManager, SIGNAL(currentChanged(QModelIndex)), this, SLOT(selectedChangeChanged(QModelIndex)));
    m_trackedChangeManager->show();
*/    //    view.setModel(&model);
    //    view.setWindowTitle("testTracked");
    //    view.show();
    //    TrackedChangeManager *dia = new TrackedChangeManager(m_textShapeData->document());
    //    dia->show();
}

#include <ReviewTool.moc>
