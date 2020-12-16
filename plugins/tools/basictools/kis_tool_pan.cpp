/*
 *  SPDX-FileCopyrightText: 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_pan.h"
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

void KisToolPan::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Up:
            canvas()->canvasController()->panUp();
            break;
        case Qt::Key_Down:
            canvas()->canvasController()->panDown();
            break;
        case Qt::Key_Left:
            canvas()->canvasController()->panLeft();
            break;
        case Qt::Key_Right:
            canvas()->canvasController()->panRight();
            break;
    }
    event->accept();
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
    setSection(navigationToolType());
    setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    setPriority(2);
    setIconName(koIconNameCStr("tool_pan"));
}

KisToolPanFactory::~KisToolPanFactory()
{
}

KoToolBase* KisToolPanFactory::createTool(KoCanvasBase *canvas)
{
    return new KisToolPan(canvas);
}
