/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_guides_manager.h"
#include "kis_guides_decoration.h"
#include <KoRuler.h>
#include <KoGuidesData.h>
#include "kis_action_manager.h"
#include "kis_action.h"
#include "kis_signals_blocker.h"
#include "kis_input_manager.h"
#include "kis_coordinates_converter.h"
#include "kis_zoom_manager.h"
#include "kis_signal_auto_connection.h"
#include "KisViewManager.h"


struct KisGuidesManager::Private
{
    Private(KisGuidesManager *_q)
        : q(_q),
          decoration(0),
          invalidGuide(Qt::Horizontal, -1),
          currentGuide(invalidGuide),
          cursorSwitched(false),
          dragStartGuidePos(0) {}

    KisGuidesManager *q;

    KisGuidesDecoration *decoration;
    KoGuidesData guidesData;
    QPointer<KisView> view;

    typedef QPair<Qt::Orientation, int> GuideHandle;

    GuideHandle findGuide(const QPointF &docPos);
    bool isGuideValid(const GuideHandle &h);
    qreal guideValue(const GuideHandle &h);
    void setGuideValue(const GuideHandle &h, qreal value);
    void deleteGuide(const GuideHandle &h);
    const GuideHandle invalidGuide;

    bool updateCursor(const QPointF &docPos);
    bool mouseMoveHandler(const QPointF &docPos);
    bool mouseReleaseHandler(const QPointF &docPos);

    GuideHandle currentGuide;

    bool cursorSwitched;
    QCursor oldCursor;

    QPointF dragStartDoc;
    qreal dragStartGuidePos;

    KisSignalAutoConnectionsStore viewConnections;
};

KisGuidesManager::KisGuidesManager(QObject *parent)
    : QObject(parent),
      m_d(new Private(this))
{
}

KisGuidesManager::~KisGuidesManager()
{
}

KisCanvasDecoration* KisGuidesManager::decoration() const
{
    return m_d->decoration;
}

void KisGuidesManager::setGuidesDataImpl(const KoGuidesData &value)
{
    if (m_d->decoration && value != m_d->decoration->guidesData()) {
        m_d->decoration->setVisible(value.showGuides());
        m_d->decoration->setGuidesData(value);
    }

    const bool shouldFilterEvent =
        value.showGuides() && !value.lockGuides() && value.hasGuides();

    attachEventFilterImpl(shouldFilterEvent);
    syncActionsStatus();
}

void KisGuidesManager::attachEventFilterImpl(bool value)
{
    if (!m_d->view) return;

    KisInputManager *inputManager = m_d->view->globalInputManager();
    if (inputManager) {
        if (value) {
            inputManager->attachPriorityEventFilter(this);
        } else {
            inputManager->detachPriorityEventFilter(this);
        }
    }
}

void KisGuidesManager::syncActionsStatus()
{
    if (!m_d->view) return;

    KisActionManager *actionManager = m_d->view->viewManager()->actionManager();
    KisAction *showGuidesAction = actionManager->actionByName("new_show_guides");
    KisAction *lockGuidesAction = actionManager->actionByName("new_lock_guides");
    KisAction *snapToGuidesAction = actionManager->actionByName("new_snap_to_guides");

    KisSignalsBlocker l(showGuidesAction, lockGuidesAction, snapToGuidesAction);
    showGuidesAction->setChecked(m_d->guidesData.showGuides());
    lockGuidesAction->setChecked(m_d->guidesData.lockGuides());
    snapToGuidesAction->setChecked(m_d->guidesData.snapToGuides());
}

bool KisGuidesManager::showGuides() const
{
    return m_d->guidesData.showGuides();
}

void KisGuidesManager::setShowGuides(bool value)
{
    m_d->guidesData.setShowGuides(value);
    setGuidesDataImpl(m_d->guidesData);
}

bool KisGuidesManager::lockGuides() const
{
    return m_d->guidesData.lockGuides();
}

void KisGuidesManager::setLockGuides(bool value)
{
    m_d->guidesData.setLockGuides(value);
    setGuidesDataImpl(m_d->guidesData);
}

bool KisGuidesManager::snapToGuides() const
{
    return m_d->guidesData.snapToGuides();
}

void KisGuidesManager::setSnapToGuides(bool value)
{
    m_d->guidesData.setSnapToGuides(value);
    setGuidesDataImpl(m_d->guidesData);
}

void KisGuidesManager::setup(KisActionManager *actionManager)
{
    KisAction *action = 0;

    action = actionManager->createAction("new_show_guides");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(setShowGuides(bool)));

    action = actionManager->createAction("new_lock_guides");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(setLockGuides(bool)));

    action = actionManager->createAction("new_snap_to_guides");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(setSnapToGuides(bool)));

    syncActionsStatus();
}

void KisGuidesManager::setView(QPointer<KisView> view)
{
    if (m_d->view) {
        m_d->decoration = 0;
        m_d->viewConnections.clear();
        attachEventFilterImpl(false);
    }

    m_d->view = view;

    if (m_d->view) {
        KisGuidesDecoration* decoration = qobject_cast<KisGuidesDecoration*>(m_d->view->canvasBase()->decoration(GUIDES_DECORATION_ID));
        if (!decoration) {
            decoration = new KisGuidesDecoration(m_d->view);
            m_d->view->canvasBase()->addDecoration(decoration);
        }
        m_d->decoration = decoration;
        setGuidesDataImpl(m_d->guidesData);

        m_d->viewConnections.addUniqueConnection(
            m_d->view->zoomManager()->horizontalRuler(), SIGNAL(guideCreationInProgress(Qt::Orientation, const QPoint&)),
            this, SLOT(slotGuideCreationInProgress(Qt::Orientation, const QPoint&)));

        m_d->viewConnections.addUniqueConnection(
            m_d->view->zoomManager()->horizontalRuler(), SIGNAL(guideCreationFinished(Qt::Orientation, const QPoint&)),
            this, SLOT(slotGuideCreationFinished(Qt::Orientation, const QPoint&)));

        m_d->viewConnections.addUniqueConnection(
            m_d->view->zoomManager()->verticalRuler(), SIGNAL(guideCreationInProgress(Qt::Orientation, const QPoint&)),
            this, SLOT(slotGuideCreationInProgress(Qt::Orientation, const QPoint&)));

        m_d->viewConnections.addUniqueConnection(
            m_d->view->zoomManager()->verticalRuler(), SIGNAL(guideCreationFinished(Qt::Orientation, const QPoint&)),
            this, SLOT(slotGuideCreationFinished(Qt::Orientation, const QPoint&)));
    }
}

KisGuidesManager::Private::GuideHandle
KisGuidesManager::Private::findGuide(const QPointF &docPos)
{
    const int snapRadius = 16;

    GuideHandle nearestGuide = invalidGuide;
    qreal nearestRadius = std::numeric_limits<int>::max();

    for (int i = 0; i < guidesData.horizontalGuideLines().size(); i++) {
        const qreal guide = guidesData.horizontalGuideLines()[i];
        const qreal radius = qAbs(docPos.y() - guide);
        if (radius < snapRadius && radius < nearestRadius) {
            nearestGuide = GuideHandle(Qt::Horizontal, i);
            nearestRadius = radius;
        }
    }

    for (int i = 0; i < guidesData.verticalGuideLines().size(); i++) {
        const qreal guide = guidesData.verticalGuideLines()[i];
        const qreal radius = qAbs(docPos.x() - guide);
        if (radius < snapRadius && radius < nearestRadius) {
            nearestGuide = GuideHandle(Qt::Vertical, i);
            nearestRadius = radius;
        }
    }

    return nearestGuide;
}

bool KisGuidesManager::Private::isGuideValid(const GuideHandle &h)
{
    return h.second >= 0;
}

qreal KisGuidesManager::Private::guideValue(const GuideHandle &h)
{
    return h.first == Qt::Horizontal ?
        guidesData.horizontalGuideLines()[h.second] :
        guidesData.verticalGuideLines()[h.second];
}

void KisGuidesManager::Private::setGuideValue(const GuideHandle &h, qreal value)
{
    if (h.first == Qt::Horizontal) {
        QList<qreal> guides = guidesData.horizontalGuideLines();
        guides[h.second] = value;
        guidesData.setHorizontalGuideLines(guides);
    } else {
        QList<qreal> guides = guidesData.verticalGuideLines();
        guides[h.second] = value;
        guidesData.setVerticalGuideLines(guides);
    }
}

void KisGuidesManager::Private::deleteGuide(const GuideHandle &h)
{
    if (h.first == Qt::Horizontal) {
        QList<qreal> guides = guidesData.horizontalGuideLines();
        guides.removeAt(h.second);
        guidesData.setHorizontalGuideLines(guides);
    } else {
        QList<qreal> guides = guidesData.verticalGuideLines();
        guides.removeAt(h.second);
        guidesData.setVerticalGuideLines(guides);
    }
}

bool KisGuidesManager::Private::updateCursor(const QPointF &docPos)
{
    KisCanvas2 *canvas = view->canvasBase();

    const GuideHandle guide = findGuide(docPos);
    const bool guideValid = isGuideValid(guide);

    if (guideValid && !cursorSwitched) {
        oldCursor = canvas->canvasWidget()->cursor();
    }

    if (guideValid) {
        cursorSwitched = true;
        QCursor newCursor = guide.first == Qt::Horizontal ?
            Qt::SizeVerCursor : Qt::SizeHorCursor;
        canvas->canvasWidget()->setCursor(newCursor);
    }

    if (!guideValid && cursorSwitched) {
        canvas->canvasWidget()->setCursor(oldCursor);
    }

    return guideValid;
}

bool KisGuidesManager::Private::mouseMoveHandler(const QPointF &docPos)
{
    if (isGuideValid(currentGuide)) {
        const QPointF offset = docPos - dragStartDoc;
        const qreal newValue = dragStartGuidePos +
            (currentGuide.first == Qt::Horizontal ?
             offset.y() : offset.x());

        setGuideValue(currentGuide, newValue);
        q->setGuidesDataImpl(guidesData);

    }

    return updateCursor(docPos);
}

bool KisGuidesManager::Private::mouseReleaseHandler(const QPointF &docPos)
{
    KisCanvas2 *canvas = view->canvasBase();
    const KisCoordinatesConverter *converter = canvas->coordinatesConverter();

    if (isGuideValid(currentGuide)) {
        const QRectF docRect = converter->imageRectInDocumentPixels();
        if (!docRect.contains(docPos)) {
            deleteGuide(currentGuide);
            q->setGuidesDataImpl(guidesData);
        }

        currentGuide = invalidGuide;
        dragStartDoc = QPointF();
        dragStartGuidePos = 0;
    }

    return updateCursor(docPos);
}

bool KisGuidesManager::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != m_d->view->canvasBase()->canvasWidget()) return false;
    KisCanvas2 *canvas = m_d->view->canvasBase();
    const KisCoordinatesConverter *converter = canvas->coordinatesConverter();

    bool retval = false;

    switch (event->type()) {
    case QEvent::Enter:
    case QEvent::Leave:
    case QEvent::MouseMove: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        const QPointF docPos = converter->widgetToDocument(mouseEvent->pos());

        retval = m_d->mouseMoveHandler(docPos);

        break;
    }
    case QEvent::MouseButtonPress: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        const QPointF docPos = converter->widgetToDocument(mouseEvent->pos());
        const Private::GuideHandle guide = m_d->findGuide(docPos);
        const bool guideValid = m_d->isGuideValid(guide);

        if (guideValid) {
            m_d->currentGuide = guide;
            m_d->dragStartDoc = docPos;
            m_d->dragStartGuidePos = m_d->guideValue(guide);
        }

        retval = m_d->updateCursor(docPos);

        break;
    }
    case QEvent::MouseButtonRelease: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        const QPointF docPos = converter->widgetToDocument(mouseEvent->pos());

        retval = m_d->mouseReleaseHandler(docPos);

        break;
    }
    default:
        break;
    }

    return !retval ? QObject::eventFilter(obj, event) : true;
}

void KisGuidesManager::slotGuideCreationInProgress(Qt::Orientation orientation, const QPoint &globalPos)
{
    if (m_d->guidesData.lockGuides()) return;

    KisCanvas2 *canvas = m_d->view->canvasBase();
    const KisCoordinatesConverter *converter = canvas->coordinatesConverter();
    const QPointF widgetPos = canvas->canvasWidget()->mapFromGlobal(globalPos);
    const QPointF docPos = converter->widgetToDocument(widgetPos);

    if (m_d->isGuideValid(m_d->currentGuide)) {
        m_d->mouseMoveHandler(docPos);
    } else {
        m_d->guidesData.setShowGuides(true);

        if (orientation == Qt::Horizontal) {
            QList<qreal> guides = m_d->guidesData.horizontalGuideLines();
            guides.append(docPos.y());
            m_d->currentGuide.first = orientation;
            m_d->currentGuide.second = guides.size() - 1;
            m_d->guidesData.setHorizontalGuideLines(guides);
        } else {
            QList<qreal> guides = m_d->guidesData.verticalGuideLines();
            guides.append(docPos.x());
            m_d->currentGuide.first = orientation;
            m_d->currentGuide.second = guides.size() - 1;
            m_d->guidesData.setVerticalGuideLines(guides);
        }

        setGuidesDataImpl(m_d->guidesData);
    }
}

void KisGuidesManager::slotGuideCreationFinished(Qt::Orientation orientation, const QPoint &globalPos)
{
    Q_UNUSED(orientation);
    if (m_d->guidesData.lockGuides()) return;

    KisCanvas2 *canvas = m_d->view->canvasBase();
    const KisCoordinatesConverter *converter = canvas->coordinatesConverter();
    const QPointF widgetPos = canvas->canvasWidget()->mapFromGlobal(globalPos);
    const QPointF docPos = converter->widgetToDocument(widgetPos);

    m_d->mouseReleaseHandler(docPos);
}
