#include "kis_selection_assistants_decoration.h"

#include <limits>

#include <QList>
#include <QPointF>
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kis_algebra_2d.h>
#include "kis_debug.h"
#include "KisDocument.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_icon_utils.h"
#include "KisViewManager.h"
#include <KoCompositeOpRegistry.h>
#include "kis_tool_proxy.h"
#include <QTransform>

#include <QPainter>
#include <QPainterPath>
#include <QApplication>

struct KisSelectionAssistantsDecoration::Private {
    Private()
    {}
    KisCanvas2 * m_canvas = 0;
};



KisSelectionAssistantsDecoration::KisSelectionAssistantsDecoration() :
    d(new Private)
{
}

KisSelectionAssistantsDecoration::~KisSelectionAssistantsDecoration()
{
    delete d;
}

void KisSelectionAssistantsDecoration::drawDecoration(QPainter& gc, const KisCoordinatesConverter *converter)
{
    QPainterPath bgPath;
    bgPath.addRoundedRect(QRectF(200, 512, 128, 67), 6, 6); // placeholder position and size
    gc.fillPath(bgPath, Qt::darkGray);

    QPainterPath dragRect;
    int width = 240;
    int height = 67;
    dragRect.addRect(QRectF(300, 512, width, height));
    gc.fillPath(bgPath.intersected(dragRect),Qt::lightGray);

    QPainterPath dragRectDots;
    QColor dragDecorationDotsColor(50,50,50,255);
    int dotSize = 2;
    dragRectDots.addEllipse(3,2.5,dotSize,dotSize);
    dragRectDots.addEllipse(3,7.5,dotSize,dotSize);
    dragRectDots.addEllipse(3,-2.5,dotSize,dotSize);
    dragRectDots.addEllipse(3,-7.5,dotSize,dotSize);
    dragRectDots.addEllipse(-3,2.5,dotSize,dotSize);
    dragRectDots.addEllipse(-3,7.5,dotSize,dotSize);
    dragRectDots.addEllipse(-3,-2.5,dotSize,dotSize);
    dragRectDots.addEllipse(-3,-7.5,dotSize,dotSize);
    dragRectDots.translate(315, 542);

    gc.fillPath(dragRectDots,dragDecorationDotsColor);
}
