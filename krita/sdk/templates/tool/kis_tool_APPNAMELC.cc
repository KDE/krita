#include <kis_tool_%{APPNAMELC}.h>

#include <qpainter.h>

#include <kis_debug.h>
#include <klocale.h>

#include <KoCanvasController.h>
#include <KoPointerEvent.h>

#include <kis_canvas2.h>
#include <kis_cursor.h>
#include <kis_view2.h>


KisTool % {APPNAME}::KisTool % {APPNAME}(KoCanvasBase * canvas)
        : KisTool(canvas, KisCursor::arrowCursor()), m_canvas(dynamic_cast<KisCanvas2*>(canvas))
{
    Q_ASSERT(m_canvas);
    setObjectName("tool_%{APPNAMELC}");

}

KisTool % {APPNAME}::~KisTool % {APPNAME}()
{
}

void KisTool % {APPNAME}::activate(bool)
{
    // Add code here to initialize your tool when it got activated
    KisTool::activate();
}

void KisTool % {APPNAME}::deactivate()
{
    // Add code here to initialize your tool when it got deactivated
    KisTool::deactivate();
}

void KisTool % {APPNAME}::mousePressEvent(KoPointerEvent *event)
{
    event->ignore();
}


void KisTool % {APPNAME}::mouseMoveEvent(KoPointerEvent *event)
{
    event->ignore();
}

void KisTool % {APPNAME}::mouseReleaseEvent(KoPointerEvent *event)
{
    event->ignore();
}

void KisTool % {APPNAME}::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(gc);
    Q_UNUSED(converter);
}

// Uncomment if you have a configuration widget
// QWidget* KisTool%{APPNAME}::createOptionWidget()
// {
//     return 0;
// }
//
// QWidget* KisTool%{APPNAME}::optionWidget()
// {
//         return 0;
// }


#include "kis_tool_%{APPNAMELC}.moc"
