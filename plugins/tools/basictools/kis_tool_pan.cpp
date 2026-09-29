/*
 *  SPDX-FileCopyrightText: 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QKeyEvent>

#include "kis_tool_pan.h"

#include "kis_action_registry.h"
#include "kis_cursor.h"
#include "kis_canvas2.h"

#include <KoCanvasController.h>

#include <KoIcon.h>
#include <klocalizedstring.h>


KisToolPan::KisToolPan(KoCanvasBase *canvas)
    : KisTool(canvas, KisCursor::openHandCursor())
{
}

KisToolPan::~KisToolPan()
{
}

void KisToolPan::activate(const QSet<KoShape*> &shapes)
{
    Q_UNUSED(shapes);

    m_actionConnections.addConnection(action("movetool-move-up"),
        &QAction::triggered, this, [this] () {
            canvas()->canvasController()->panUp();
        });

    m_actionConnections.addConnection(action("movetool-move-down"),
        &QAction::triggered, this, [this] () {
            canvas()->canvasController()->panDown();
        });

    m_actionConnections.addConnection(action("movetool-move-left"),
        &QAction::triggered, this, [this] () {
            canvas()->canvasController()->panLeft();
        });

    m_actionConnections.addConnection(action("movetool-move-right"),
        &QAction::triggered, this, [this] () {
            canvas()->canvasController()->panRight();
        });
}

void KisToolPan::deactivate()
{
    m_actionConnections.clear();
    KisTool::deactivate();
}

void KisToolPan::beginPrimaryAction(KoPointerEvent *event)
{
    m_lastPosition = event->pos();
    useCursor(KisCursor::closedHandCursor());
}

void KisToolPan::continuePrimaryAction(KoPointerEvent *event)
{
    QPoint pos = event->pos();
    QPoint delta = m_lastPosition - pos;
    canvas()->canvasController()->pan(delta);
    m_lastPosition = pos;
}

void KisToolPan::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
    useCursor(KisCursor::openHandCursor());
}

void KisToolPan::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

bool KisToolPan::wantsAutoScroll() const
{
    return false;
}

KisToolPanFactory::KisToolPanFactory()
    : KoToolFactoryBase("PanTool")
{
    setToolTip(i18n("Pan Tool"));
    setSection(ToolBoxSection::Navigation);
    setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    setPriority(2);
    setIconName(koIconNameCStr("tool_pan"));
}

KisToolPanFactory::~KisToolPanFactory()
{
}

QList<QAction *> KisToolPanFactory::createActionsImpl()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    QList<QAction *> actions = KoToolFactoryBase::createActionsImpl();

    actions << actionRegistry->makeQAction("movetool-move-up", this);
    actions << actionRegistry->makeQAction("movetool-move-down", this);
    actions << actionRegistry->makeQAction("movetool-move-left", this);
    actions << actionRegistry->makeQAction("movetool-move-right", this);

    return actions;
}

KoToolBase* KisToolPanFactory::createTool(KoCanvasBase *canvas)
{
    return new KisToolPan(canvas);
}
