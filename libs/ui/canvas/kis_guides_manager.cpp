/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_guides_manager.h"

#include <QMenu>
#include <QGuiApplication>
#include "kis_guides_decoration.h"
#include <KoRuler.h>
#include "kis_guides_config.h"
#include "kis_action_manager.h"
#include "kis_action.h"
#include "kis_signals_blocker.h"
#include "input/kis_input_manager.h"
#include "kis_coordinates_converter.h"
#include "kis_zoom_manager.h"
#include "kis_signal_auto_connection.h"
#include "KisViewManager.h"
#include "KisDocument.h"
#include "kis_algebra_2d.h"
#include <KoSnapGuide.h>
#include "kis_snap_line_strategy.h"
#include "kis_change_guides_command.h"
#include "kis_snap_config.h"
#include  "kis_canvas2.h"
#include "kis_signal_compressor.h"
#include "kis_floating_message.h"

struct KisGuidesManager::Private
{
    Private(KisGuidesManager *_q)
        : q(_q),
          decoration(0),
          invalidGuide(Qt::Horizontal, -1),
          currentGuide(invalidGuide),
          cursorSwitched(false),
          dragStartGuidePos(0),
          shouldSetModified(false) {}

    KisGuidesManager *q;

    KisGuidesDecoration *decoration;
    KisGuidesConfig guidesConfig;
    KisGuidesConfig oldGuidesConfig;
    KisSnapConfig snapConfig;
    QPointer<KisView> view;

    typedef QPair<Qt::Orientation, int> GuideHandle;

    GuideHandle findGuide(const QPointF &docPos);
    bool isGuideValid(const GuideHandle &h);
    qreal guideValue(const GuideHandle &h);
    void setGuideValue(const GuideHandle &h, qreal value);
    void deleteGuide(const GuideHandle &h);
    const GuideHandle invalidGuide;

    bool updateCursor(const QPointF &docPos, bool forceDisableCursor = false);

    void initDragStart(const GuideHandle &guide,
                       const QPointF &dragStart,
                       qreal guideValue,
                       bool snapToStart);
    bool mouseMoveHandler(const QPointF &docPos, Qt::KeyboardModifiers modifiers);
    bool mouseReleaseHandler(const QPointF &docPos);

    void updateSnappingStatus(const KisGuidesConfig &value);
    QPointF alignToPixels(const QPointF docPoint);
    QPointF getDocPointFromEvent(QEvent *event);
    Qt::MouseButton getButtonFromEvent(QEvent *event);
    QAction* createShortenedAction(const QString &text, const QString &parentId, QObject *parent);
    void syncAction(const QString &actionName, bool value);
    bool needsUndoCommand();
    void createUndoCommandIfNeeded();

    GuideHandle currentGuide;

    bool cursorSwitched;
    QCursor oldCursor;

    QPointF dragStartDoc;
    QPointF dragPointerOffset;
    qreal dragStartGuidePos;

    KisSignalAutoConnectionsStore viewConnections;

    bool shouldSetModified;
};

KisGuidesManager::KisGuidesManager(QObject *parent)
    : QObject(parent),
      m_d(new Private(this))
{
}

KisGuidesManager::~KisGuidesManager()
{
}

void KisGuidesManager::setGuidesConfig(const KisGuidesConfig &config)
{
    if (config == m_d->guidesConfig) return;
    setGuidesConfigImpl(config, !config.hasSamePositionAs(m_d->guidesConfig));
    slotUploadConfigToDocument();
}

void KisGuidesManager::slotDocumentRequestedConfig(const KisGuidesConfig &config)
{
    if (config == m_d->guidesConfig) return;
    setGuidesConfigImpl(config, false);
}

void KisGuidesManager::slotUploadConfigToDocument()
{
    const KisGuidesConfig &value = m_d->guidesConfig;

    KisDocument *doc = m_d->view ? m_d->view->document() : 0;
    if (doc) {
        KisSignalsBlocker b(doc);

        // we've made KisChangeGuidesCommand post-exec, so in all situations
        // we will replace the whole config
        doc->setGuidesConfig(value);

        value.saveStaticData();
    }

    m_d->shouldSetModified = false;
}

void KisGuidesManager::setGuidesConfigImpl(const KisGuidesConfig &value, bool emitModified)
{
    m_d->guidesConfig = value;

    if (m_d->decoration && value != m_d->decoration->guidesConfig()) {
        m_d->decoration->setVisible(value.showGuides());
        m_d->decoration->setGuidesConfig(value);
    }

    m_d->shouldSetModified |= emitModified;

    const bool shouldFilterEvent =
        value.showGuides() && !value.lockGuides() && value.hasGuides();

    attachEventFilterImpl(shouldFilterEvent);
    syncActionsStatus();

    if (!m_d->isGuideValid(m_d->currentGuide)) {
        m_d->updateSnappingStatus(value);
    }

    if (m_d->view) {
        m_d->view->document()->setUnit(KoUnit(m_d->guidesConfig.unitType()));
        m_d->view->viewManager()->actionManager()->actionByName("ruler_pixel_multiple2")->setChecked(value.rulersMultiple2());
    }

    emit sigRequestUpdateGuidesConfig(m_d->guidesConfig);
}

void KisGuidesManager::attachEventFilterImpl(bool value)
{
    if (!m_d->view) return;

    KisInputManager *inputManager = m_d->view->globalInputManager();
    if (inputManager) {
        if (value) {
            inputManager->attachPriorityEventFilter(this, 100);
        } else {
            inputManager->detachPriorityEventFilter(this);
        }
    }
}

void KisGuidesManager::Private::syncAction(const QString &actionName, bool value)
{
    KisActionManager *actionManager = view->viewManager()->actionManager();
    KisAction *action = actionManager->actionByName(actionName);
    KIS_ASSERT_RECOVER_RETURN(action);
    KisSignalsBlocker b(action);
    action->setChecked(value);
}

bool KisGuidesManager::Private::needsUndoCommand()
{
    return !(oldGuidesConfig.hasSamePositionAs(guidesConfig));
}

void KisGuidesManager::Private::createUndoCommandIfNeeded()
{
    KisDocument *doc = view ? view->document() : 0;
    if (doc && needsUndoCommand()) {
        KUndo2Command *cmd = new KisChangeGuidesCommand(doc, oldGuidesConfig, guidesConfig);
        doc->addCommand(cmd);
    }
}

void KisGuidesManager::syncActionsStatus()
{
    if (!m_d->view) return;

    m_d->syncAction("view_show_guides", m_d->guidesConfig.showGuides());
    m_d->syncAction("view_lock_guides", m_d->guidesConfig.lockGuides());
    m_d->syncAction("view_snap_to_guides", m_d->guidesConfig.snapToGuides());

    m_d->syncAction("view_snap_orthogonal", m_d->snapConfig.orthogonal());
    m_d->syncAction("view_snap_node", m_d->snapConfig.node());
    m_d->syncAction("view_snap_extension", m_d->snapConfig.extension());
    m_d->syncAction("view_snap_intersection", m_d->snapConfig.intersection());
    m_d->syncAction("view_snap_bounding_box", m_d->snapConfig.boundingBox());
    m_d->syncAction("view_snap_image_bounds", m_d->snapConfig.imageBounds());
    m_d->syncAction("view_snap_image_center", m_d->snapConfig.imageCenter());
    m_d->syncAction("view_snap_to_pixel",m_d->snapConfig.toPixel());
}

void KisGuidesManager::Private::updateSnappingStatus(const KisGuidesConfig &value)
{
    if (!view) return;

    KoSnapGuide *snapGuide = view->canvasBase()->snapGuide();
    KisSnapLineStrategy *guidesSnap = 0;

    if (value.snapToGuides()) {
        guidesSnap = new KisSnapLineStrategy(KoSnapGuide::GuideLineSnapping);
        guidesSnap->setHorizontalLines(value.horizontalGuideLines());
        guidesSnap->setVerticalLines(value.verticalGuideLines());
    }

    snapGuide->overrideSnapStrategy(KoSnapGuide::GuideLineSnapping, guidesSnap);
    snapGuide->enableSnapStrategy(KoSnapGuide::GuideLineSnapping, guidesSnap);

    snapGuide->enableSnapStrategy(KoSnapGuide::OrthogonalSnapping, snapConfig.orthogonal());
    snapGuide->enableSnapStrategy(KoSnapGuide::NodeSnapping, snapConfig.node());
    snapGuide->enableSnapStrategy(KoSnapGuide::ExtensionSnapping, snapConfig.extension());
    snapGuide->enableSnapStrategy(KoSnapGuide::IntersectionSnapping, snapConfig.intersection());
    snapGuide->enableSnapStrategy(KoSnapGuide::BoundingBoxSnapping, snapConfig.boundingBox());
    snapGuide->enableSnapStrategy(KoSnapGuide::DocumentBoundsSnapping, snapConfig.imageBounds());
    snapGuide->enableSnapStrategy(KoSnapGuide::DocumentCenterSnapping, snapConfig.imageCenter());
    snapGuide->enableSnapStrategy(KoSnapGuide::PixelSnapping, snapConfig.toPixel());

    snapConfig.saveStaticData();
}

bool KisGuidesManager::showGuides() const
{
    return m_d->guidesConfig.showGuides();
}

void KisGuidesManager::setShowGuides(bool value)
{
    m_d->guidesConfig.setShowGuides(value);
    setGuidesConfigImpl(m_d->guidesConfig);
    slotUploadConfigToDocument();
}

bool KisGuidesManager::lockGuides() const
{
    return m_d->guidesConfig.lockGuides();
}

void KisGuidesManager::setLockGuides(bool value)
{
    m_d->guidesConfig.setLockGuides(value);
    setGuidesConfigImpl(m_d->guidesConfig);
    slotUploadConfigToDocument();
}

bool KisGuidesManager::snapToGuides() const
{
    return m_d->guidesConfig.snapToGuides();
}

void KisGuidesManager::setSnapToGuides(bool value)
{
    m_d->guidesConfig.setSnapToGuides(value);
    setGuidesConfigImpl(m_d->guidesConfig);
    slotUploadConfigToDocument();
}

bool KisGuidesManager::rulersMultiple2() const
{
    return m_d->guidesConfig.rulersMultiple2();
}

void KisGuidesManager::setRulersMultiple2(bool value)
{
    m_d->guidesConfig.setRulersMultiple2(value);
    setGuidesConfigImpl(m_d->guidesConfig);
    slotUploadConfigToDocument();
}

KoUnit::Type KisGuidesManager::unitType() const
{
    return m_d->guidesConfig.unitType();
}

void KisGuidesManager::setUnitType(const KoUnit::Type type)
{
    m_d->guidesConfig.setUnitType(type);
    setGuidesConfigImpl(m_d->guidesConfig, false);
    slotUploadConfigToDocument();
}

void KisGuidesManager::setup(KisActionManager *actionManager)
{
    KisAction *action = 0;

    action = actionManager->createAction("view_show_guides");
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setShowGuides(bool)));

    action = actionManager->createAction("view_lock_guides");
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setLockGuides(bool)));

    action = actionManager->createAction("view_snap_to_guides");
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setSnapToGuides(bool)));

    action = actionManager->createAction("show_snap_options_popup");
    connect(action, SIGNAL(triggered()), this, SLOT(slotShowSnapOptions()));

    action = actionManager->createAction("view_snap_orthogonal");
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setSnapOrthogonal(bool)));

    action = actionManager->createAction("view_snap_node");
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setSnapNode(bool)));

    action = actionManager->createAction("view_snap_extension");
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setSnapExtension(bool)));

    action = actionManager->createAction("view_snap_intersection");
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setSnapIntersection(bool)));

    action = actionManager->createAction("view_snap_bounding_box");
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setSnapBoundingBox(bool)));

    action = actionManager->createAction("view_snap_image_bounds");
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setSnapImageBounds(bool)));

    action = actionManager->createAction("view_snap_image_center");
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setSnapImageCenter(bool)));

    action = actionManager->createAction("view_snap_to_pixel");
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setSnapToPixel(bool)));

    m_d->updateSnappingStatus(m_d->guidesConfig);
    syncActionsStatus();
}

void KisGuidesManager::setView(QPointer<KisView> view)
{
    if (m_d->view) {
        KoSnapGuide *snapGuide = m_d->view->canvasBase()->snapGuide();
        snapGuide->overrideSnapStrategy(KoSnapGuide::GuideLineSnapping, 0);
        snapGuide->enableSnapStrategy(KoSnapGuide::GuideLineSnapping, false);

        slotUploadConfigToDocument();

        m_d->decoration = 0;
        m_d->viewConnections.clear();
        attachEventFilterImpl(false);
    }

    m_d->view = view;

    if (m_d->view) {
        KisGuidesDecoration* decoration = qobject_cast<KisGuidesDecoration*>(m_d->view->canvasBase()->decoration(GUIDES_DECORATION_ID).data());
        if (!decoration) {
            decoration = new KisGuidesDecoration(m_d->view);
            m_d->view->canvasBase()->addDecoration(decoration);
        }
        m_d->decoration = decoration;

        m_d->guidesConfig = m_d->view->document()->guidesConfig();
        setGuidesConfigImpl(m_d->guidesConfig, false);

        m_d->viewConnections.addUniqueConnection(
            m_d->view->zoomManager()->horizontalRuler(), SIGNAL(guideCreationInProgress(Qt::Orientation,QPoint)),
            this, SLOT(slotGuideCreationInProgress(Qt::Orientation,QPoint)));

        m_d->viewConnections.addUniqueConnection(
            m_d->view->zoomManager()->horizontalRuler(), SIGNAL(guideCreationFinished(Qt::Orientation,QPoint)),
            this, SLOT(slotGuideCreationFinished(Qt::Orientation,QPoint)));

        m_d->viewConnections.addUniqueConnection(
            m_d->view->zoomManager()->verticalRuler(), SIGNAL(guideCreationInProgress(Qt::Orientation,QPoint)),
            this, SLOT(slotGuideCreationInProgress(Qt::Orientation,QPoint)));

        m_d->viewConnections.addUniqueConnection(
            m_d->view->zoomManager()->verticalRuler(), SIGNAL(guideCreationFinished(Qt::Orientation,QPoint)),
            this, SLOT(slotGuideCreationFinished(Qt::Orientation,QPoint)));

        m_d->viewConnections.addUniqueConnection(
            m_d->view->document(), SIGNAL(sigGuidesConfigChanged(KisGuidesConfig)),
            this, SLOT(slotDocumentRequestedConfig(KisGuidesConfig)));
    }
}

KisGuidesManager::Private::GuideHandle
KisGuidesManager::Private::findGuide(const QPointF &docPos)
{
    const int snapRadius = 16;
    const KoViewConverter *converter = view->canvasBase()->viewConverter();
    const QPointF docPosView = converter->documentToView(docPos);

    GuideHandle nearestGuide = invalidGuide;
    qreal nearestRadius = std::numeric_limits<int>::max();


    for (int i = 0; i < guidesConfig.horizontalGuideLines().size(); i++) {
        const QPointF guideCoord = {0, guidesConfig.horizontalGuideLines()[i]};
        const qreal guide = converter->documentToView(guideCoord).y();
        const qreal radius = qAbs(docPosView.y() - guide);
        if (radius < snapRadius && radius < nearestRadius) {
            nearestGuide = GuideHandle(Qt::Horizontal, i);
            nearestRadius = radius;
        }
    }

    for (int i = 0; i < guidesConfig.verticalGuideLines().size(); i++) {
        const QPointF guideCoord = {guidesConfig.verticalGuideLines()[i], 0};
        const qreal guide = converter->documentToView(guideCoord).x();
        const qreal radius = qAbs(docPosView.x() - guide);
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
        guidesConfig.horizontalGuideLines()[h.second] :
        guidesConfig.verticalGuideLines()[h.second];
}

void KisGuidesManager::Private::setGuideValue(const GuideHandle &h, qreal value)
{
    if (h.first == Qt::Horizontal) {
        QList<qreal> guides = guidesConfig.horizontalGuideLines();
        guides[h.second] = value;
        guidesConfig.setHorizontalGuideLines(guides);
    } else {
        QList<qreal> guides = guidesConfig.verticalGuideLines();
        guides[h.second] = value;
        guidesConfig.setVerticalGuideLines(guides);
    }
}

void KisGuidesManager::Private::deleteGuide(const GuideHandle &h)
{
    if (h.first == Qt::Horizontal) {
        QList<qreal> guides = guidesConfig.horizontalGuideLines();
        guides.removeAt(h.second);
        guidesConfig.setHorizontalGuideLines(guides);
    } else {
        QList<qreal> guides = guidesConfig.verticalGuideLines();
        guides.removeAt(h.second);
        guidesConfig.setVerticalGuideLines(guides);
    }
}

bool KisGuidesManager::Private::updateCursor(const QPointF &docPos, bool forceDisableCursor)
{
    KisCanvas2 *canvas = view->canvasBase();

    const GuideHandle guide = findGuide(docPos);
    const bool guideValid = isGuideValid(guide) && !forceDisableCursor;

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
        cursorSwitched = false;
    }

    return guideValid;
}

void KisGuidesManager::Private::initDragStart(const GuideHandle &guide,
                                              const QPointF &dragStart,
                                              qreal guideValue,
                                              bool snapToStart)
{
    currentGuide = guide;
    dragStartDoc = dragStart;
    dragStartGuidePos = guideValue;
    dragPointerOffset =
        guide.first == Qt::Horizontal ?
        QPointF(0, dragStartGuidePos - dragStartDoc.y()) :
        QPointF(dragStartGuidePos - dragStartDoc.x(), 0);

    KoSnapGuide *snapGuide = view->canvasBase()->snapGuide();
    snapGuide->reset();

    if (snapToStart) {
        KisSnapLineStrategy *strategy = new KisSnapLineStrategy();
        strategy->addLine(guide.first, guideValue);
        snapGuide->addCustomSnapStrategy(strategy);
    }
}

QPointF KisGuidesManager::Private::alignToPixels(const QPointF docPoint)
{
    KisCanvas2 *canvas = view->canvasBase();
    const KisCoordinatesConverter *converter = canvas->coordinatesConverter();
    QPoint imagePoint = converter->documentToImage(docPoint).toPoint();
    return converter->imageToDocument(imagePoint);
}

bool KisGuidesManager::Private::mouseMoveHandler(const QPointF &docPos, Qt::KeyboardModifiers modifiers)
{
    if (isGuideValid(currentGuide)) {
        KoSnapGuide *snapGuide = view->canvasBase()->snapGuide();
        const QPointF snappedPos = snapGuide->snap(docPos, dragPointerOffset, modifiers);
        const QPointF offset = snappedPos - dragStartDoc;
        const qreal newValue = dragStartGuidePos +
            (currentGuide.first == Qt::Horizontal ?
             offset.y() : offset.x());

        setGuideValue(currentGuide, newValue);
        q->setGuidesConfigImpl(guidesConfig);

        const KisCoordinatesConverter *converter = view->canvasBase()->coordinatesConverter();
        if(currentGuide.first == Qt::Horizontal) {
            view->canvasBase()->viewManager()->showFloatingMessage(
                    i18n("Y: %1 px", converter->documentToImage(docPos).toPoint().y()), QIcon(), 1000
                        , KisFloatingMessage::High, Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignVCenter);
        }
        else {
            view->canvasBase()->viewManager()->showFloatingMessage(
                    i18n("X: %1 px",  converter->documentToImage(docPos).toPoint().x()), QIcon(), 1000
                        , KisFloatingMessage::High, Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignVCenter);
        }
    }

    return updateCursor(docPos);
}

bool KisGuidesManager::Private::mouseReleaseHandler(const QPointF &docPos)
{
    bool result = false;
    KisCanvas2 *canvas = view->canvasBase();
    const KisCoordinatesConverter *converter = canvas->coordinatesConverter();

    if (isGuideValid(currentGuide)) {
        const QRectF docRect = converter->imageRectInDocumentPixels();
        // TODO: enable work rect after we fix painting guides
        //       outside canvas in openGL mode
        const QRectF workRect = KisAlgebra2D::blowRect(docRect, 0 /*0.2*/);
        if (!workRect.contains(docPos)) {
            deleteGuide(currentGuide);
            q->setGuidesConfigImpl(guidesConfig);

            /**
             * When we delete a guide, it might happen that we are
             * deleting the last guide. Therefore we should eat the
             * corresponding event so that the event filter would stop
             * the filter processing.
             */
            result = true;
        }

        currentGuide = invalidGuide;
        dragStartDoc = QPointF();
        dragPointerOffset = QPointF();
        dragStartGuidePos = 0;

        KoSnapGuide *snapGuide = view->canvasBase()->snapGuide();
        snapGuide->reset();

        updateSnappingStatus(guidesConfig);
    }

    q->slotUploadConfigToDocument();
    createUndoCommandIfNeeded();

    return updateCursor(docPos) | result;
}

QPointF KisGuidesManager::Private::getDocPointFromEvent(QEvent *event)
{
    QPointF result;

    KisCanvas2 *canvas = view->canvasBase();
    const KisCoordinatesConverter *converter = canvas->coordinatesConverter();

    if (event->type() == QEvent::Enter) {
        QEnterEvent *enterEvent = static_cast<QEnterEvent*>(event);
        result = alignToPixels(converter->widgetToDocument(enterEvent->pos()));
    } else if (event->type() == QEvent::MouseMove ||
        event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease) {

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        result = alignToPixels(converter->widgetToDocument(mouseEvent->pos()));

    } else if (event->type() == QEvent::TabletMove ||
               event->type() == QEvent::TabletPress ||
               event->type() == QEvent::TabletRelease) {

        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        result = alignToPixels(converter->widgetToDocument(tabletEvent->pos()));
    } else {
        // we shouldn't silently return QPointF(0,0), higher level code may
        // snap to some unexpected guide
        KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "event type is not supported!");
    }

    return result;
}

Qt::MouseButton KisGuidesManager::Private::getButtonFromEvent(QEvent *event)
{
    Qt::MouseButton button = Qt::NoButton;

    if (event->type() == QEvent::MouseMove ||
        event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease) {

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        button = mouseEvent->button();

    } else if (event->type() == QEvent::TabletMove ||
               event->type() == QEvent::TabletPress ||
               event->type() == QEvent::TabletRelease) {

        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        button = tabletEvent->button();
    }

    return button;
}

bool KisGuidesManager::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_d->view || obj != m_d->view->canvasBase()->canvasWidget()) return false;

    bool retval = false;

    switch (event->type()) {
    case QEvent::Leave:
        m_d->updateCursor(QPointF(), true);
        break;
    case QEvent::Enter:
    case QEvent::TabletMove:
    case QEvent::MouseMove: {
        const QPointF docPos = m_d->getDocPointFromEvent(event);
        const Qt::KeyboardModifiers modifiers = qApp->keyboardModifiers();

        // we should never eat Enter events, input manager may get crazy about it
        retval = m_d->mouseMoveHandler(docPos, modifiers) && event->type() != QEvent::Enter;

        break;
    }
    case QEvent::TabletPress:
    case QEvent::MouseButtonPress: {
        if (m_d->getButtonFromEvent(event) != Qt::LeftButton) break;

        const QPointF docPos = m_d->getDocPointFromEvent(event);
        const Private::GuideHandle guide = m_d->findGuide(docPos);
        const bool guideValid = m_d->isGuideValid(guide);

        if (guideValid) {
            m_d->oldGuidesConfig = m_d->guidesConfig;
            m_d->initDragStart(guide, docPos, m_d->guideValue(guide), true);
        }

        retval = m_d->updateCursor(docPos);

        break;
    }
    case QEvent::TabletRelease:
    case QEvent::MouseButtonRelease: {
        if (m_d->getButtonFromEvent(event) != Qt::LeftButton) break;

        const QPointF docPos = m_d->getDocPointFromEvent(event);
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
    if (m_d->guidesConfig.lockGuides()) return;

    KisCanvas2 *canvas = m_d->view->canvasBase();
    const KisCoordinatesConverter *converter = canvas->coordinatesConverter();
    const QPointF widgetPos = canvas->canvasWidget()->mapFromGlobal(globalPos);
    const QPointF docPos = m_d->alignToPixels(converter->widgetToDocument(widgetPos));

    if (m_d->isGuideValid(m_d->currentGuide)) {
        const Qt::KeyboardModifiers modifiers = qApp->keyboardModifiers();
        m_d->mouseMoveHandler(docPos, modifiers);
    } else {
        m_d->guidesConfig.setShowGuides(true);

        m_d->oldGuidesConfig = m_d->guidesConfig;

        if (orientation == Qt::Horizontal) {
            QList<qreal> guides = m_d->guidesConfig.horizontalGuideLines();
            guides.append(docPos.y());
            m_d->currentGuide.first = orientation;
            m_d->currentGuide.second = guides.size() - 1;
            m_d->guidesConfig.setHorizontalGuideLines(guides);
            m_d->initDragStart(m_d->currentGuide, docPos, docPos.y(), false);
        } else {
            QList<qreal> guides = m_d->guidesConfig.verticalGuideLines();
            guides.append(docPos.x());
            m_d->currentGuide.first = orientation;
            m_d->currentGuide.second = guides.size() - 1;
            m_d->guidesConfig.setVerticalGuideLines(guides);
            m_d->initDragStart(m_d->currentGuide, docPos, docPos.x(), false);
        }

        setGuidesConfigImpl(m_d->guidesConfig);
    }
}

void KisGuidesManager::slotGuideCreationFinished(Qt::Orientation orientation, const QPoint &globalPos)
{
    Q_UNUSED(orientation);
    if (m_d->guidesConfig.lockGuides()) return;

    KisCanvas2 *canvas = m_d->view->canvasBase();
    const KisCoordinatesConverter *converter = canvas->coordinatesConverter();
    const QPointF widgetPos = canvas->canvasWidget()->mapFromGlobal(globalPos);
    const QPointF docPos = m_d->alignToPixels(converter->widgetToDocument(widgetPos));

    m_d->mouseReleaseHandler(docPos);
}

QAction* KisGuidesManager::Private::createShortenedAction(const QString &text, const QString &parentId, QObject *parent)
{
    KisActionManager *actionManager = view->viewManager()->actionManager();
    QAction *action = 0;
    KisAction *parentAction = 0;

    action = new QAction(text, parent);
    action->setCheckable(true);
    parentAction = actionManager->actionByName(parentId);
    action->setChecked(parentAction->isChecked());
    connect(action, SIGNAL(toggled(bool)), parentAction, SLOT(setChecked(bool)));

    return action;
}

void KisGuidesManager::slotShowSnapOptions()
{
    const QPoint pos = QCursor::pos();
    QMenu menu;

    menu.addSection(i18n("Snap to:"));
    menu.addAction(m_d->createShortenedAction(i18n("Grid"), "view_snap_to_grid", &menu));
    menu.addAction(m_d->createShortenedAction(i18n("Guides"), "view_snap_to_guides", &menu));
    menu.addAction(m_d->createShortenedAction(i18n("Pixel"), "view_snap_to_pixel", &menu));
    menu.addAction(m_d->createShortenedAction(i18n("Orthogonal"), "view_snap_orthogonal", &menu));

    menu.addAction(m_d->createShortenedAction(i18n("Node"), "view_snap_node", &menu));
    menu.addAction(m_d->createShortenedAction(i18n("Extension"), "view_snap_extension", &menu));
    menu.addAction(m_d->createShortenedAction(i18n("Intersection"), "view_snap_intersection", &menu));

    menu.addAction(m_d->createShortenedAction(i18n("Bounding Box"), "view_snap_bounding_box", &menu));
    menu.addAction(m_d->createShortenedAction(i18n("Image Bounds"), "view_snap_image_bounds", &menu));
    menu.addAction(m_d->createShortenedAction(i18n("Image Center"), "view_snap_image_center", &menu));

    menu.exec(pos);
}

void KisGuidesManager::setSnapOrthogonal(bool value)
{
    m_d->snapConfig.setOrthogonal(value);
    m_d->updateSnappingStatus(m_d->guidesConfig);
}

void KisGuidesManager::setSnapNode(bool value)
{
    m_d->snapConfig.setNode(value);
    m_d->updateSnappingStatus(m_d->guidesConfig);
}

void KisGuidesManager::setSnapExtension(bool value)
{
    m_d->snapConfig.setExtension(value);
    m_d->updateSnappingStatus(m_d->guidesConfig);
}

void KisGuidesManager::setSnapIntersection(bool value)
{
    m_d->snapConfig.setIntersection(value);
    m_d->updateSnappingStatus(m_d->guidesConfig);
}

void KisGuidesManager::setSnapBoundingBox(bool value)
{
    m_d->snapConfig.setBoundingBox(value);
    m_d->updateSnappingStatus(m_d->guidesConfig);
}

void KisGuidesManager::setSnapImageBounds(bool value)
{
    m_d->snapConfig.setImageBounds(value);
    m_d->updateSnappingStatus(m_d->guidesConfig);
}

void KisGuidesManager::setSnapImageCenter(bool value)
{
    m_d->snapConfig.setImageCenter(value);
    m_d->updateSnappingStatus(m_d->guidesConfig);
}

void KisGuidesManager::setSnapToPixel(bool value)
{
    m_d->snapConfig.setToPixel(value);
    m_d->updateSnappingStatus(m_d->guidesConfig);
}
