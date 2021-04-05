#include "TwoPointAssistant.h"
#include "kis_debug.h"
#include <klocalizedstring.h>

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include <kis_algebra_2d.h>
#include <kis_dom_utils.h>
#include <math.h>
#include <QtCore/qmath.h>

TwoPointAssistant::TwoPointAssistant()
    : KisPaintingAssistant("two point", i18n("Two point assistant"))
    , m_followBrushPosition(false)
    , m_adjustedPositionValid(false)
{
}

TwoPointAssistant::TwoPointAssistant(const TwoPointAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisPaintingAssistant(rhs, handleMap)
    , m_canvas(rhs.m_canvas)
    , m_snapLine(rhs.m_snapLine)
    , m_gridDensity(rhs.m_gridDensity)
    , m_followBrushPosition(rhs.m_followBrushPosition)
    , m_adjustedPositionValid(rhs.m_adjustedPositionValid)
    , m_adjustedBrushPosition(rhs.m_adjustedBrushPosition)
{
}

inline qreal distsqr(const QPointF& pt, const QLineF& line)
{
    const qreal cross = (line.dx() * (line.y1() - pt.y()) - line.dy() * (line.x1() - pt.x()));
    return cross * cross / (line.dx() * line.dx() + line.dy() * line.dy());
}

// returns how far angle is from 180 or 0, depending on quadrant
inline qreal acuteAngle(qreal angle) {
    if (angle > 90 && angle < 270) {
        return abs(angle - 180);
    } else if (angle < 360 && angle > 270) {
        return 360 - angle;
    } else {
        return angle;
    }
}

KisPaintingAssistantSP TwoPointAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new TwoPointAssistant(*this, handleMap));
}

QPointF TwoPointAssistant::project(const QPointF& pt, const QPointF& strokeBegin)
{
    Q_ASSERT(isAssistantComplete());

    // nicked wholesale from PerspectiveAssistant.cc
    if (m_snapLine.isNull()) {
        const qreal dx = pt.x() - strokeBegin.x();
        const qreal dy = pt.y() - strokeBegin.y();

        if (dx * dx + dy * dy < 4.0) {
            return strokeBegin; // allow some movement before snapping
        }

        // figure out which direction to go
        const QLineF vp_snap_a = QLineF(strokeBegin, QLineF(strokeBegin, *handles()[0]).unitVector().p2());
        const QLineF vp_snap_b = QLineF(strokeBegin, QLineF(strokeBegin, *handles()[1]).unitVector().p2());

        m_snapLine = distsqr(pt, vp_snap_a) < distsqr(pt, vp_snap_b) ? vp_snap_a : vp_snap_b;
    }

    // snap to line
    const qreal
        dx = m_snapLine.dx(),
        dy = m_snapLine.dy(),
        dx2 = dx * dx,
        dy2 = dy * dy,
        invsqrlen = 1.0 / (dx2 + dy2);

    QPointF r(dx2 * pt.x() + dy2 * m_snapLine.x1() + dx * dy * (pt.y() - m_snapLine.y1()),
              dx2 * m_snapLine.y1() + dy2 * pt.y() + dx * dy * (pt.x() - m_snapLine.x1()));

    r *= invsqrlen;
    return r;
}

void TwoPointAssistant::setAdjustedBrushPosition(const QPointF position)
{
    m_adjustedBrushPosition = position;
    m_adjustedPositionValid = true;
}

void TwoPointAssistant::setFollowBrushPosition(bool follow)
{
    m_followBrushPosition = follow;
}

void TwoPointAssistant::endStroke()
{
    // Brush stroke ended, guides should follow the brush position again.
    m_followBrushPosition = false;
    m_adjustedPositionValid = false;
    m_snapLine = QLineF();
}

QPointF TwoPointAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin)
{
    return project(pt, strokeBegin);
}

void TwoPointAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    Q_UNUSED(updateRect);
    Q_UNUSED(cached);
    gc.save();
    gc.resetTransform();
    QPointF mousePos(0,0);

    const QTransform initialTransform = converter->documentToWidgetTransform();
    bool isEditing = canvas->paintingAssistantsDecoration()->isEditingAssistants();

    if (canvas){
        //simplest, cheapest way to get the mouse-position//
        mousePos= canvas->canvasWidget()->mapFromGlobal(QCursor::pos());
        m_canvas = canvas;
    }
    else {
        mousePos = QCursor::pos();//this'll give an offset//
        dbgFile<<"canvas does not exist in ruler, you may have passed arguments incorrectly:"<<canvas;
    }

    if (m_followBrushPosition && m_adjustedPositionValid) {
        mousePos = initialTransform.map(m_adjustedBrushPosition);
    }

    if (isEditing){
        Q_FOREACH (const QPointF* handle, handles()) {
            QPointF h = initialTransform.map(*handle);
            QRectF ellipse = QRectF(QPointF(h.x() -15, h.y() -15), QSizeF(30, 30));

            QPainterPath pathCenter;
            pathCenter.addEllipse(ellipse);
            drawPath(gc, pathCenter, isSnappingActive());
        }}

    if (handles().size() >= 2) {
        const QPointF p1 = *handles()[0];
        const QPointF p2 = *handles()[1];
        const QRect viewport= gc.viewport();
        QPainterPath path;

        // draw the horizon
        if (assistantVisible == true || isEditing == true) {
            QLineF horizonLine = initialTransform.map(QLineF(p1,p2));
            KisAlgebra2D::intersectLineRect(horizonLine, viewport);
            path.moveTo(horizonLine.p1());
            path.lineTo(horizonLine.p2());
            drawPath(gc, path, isSnappingActive());
        }

        // draw the VP-->mousePos lines
        if (isEditing == false && previewVisible == true && isSnappingActive() == true) {
            QLineF snapMouse1 = QLineF(initialTransform.map(p1), mousePos);
            QLineF snapMouse2 = QLineF(initialTransform.map(p2), mousePos);
            KisAlgebra2D::intersectLineRect(snapMouse1, viewport);
            KisAlgebra2D::intersectLineRect(snapMouse2, viewport);
            path.moveTo(initialTransform.map(p1));
            path.lineTo(snapMouse1.p1());
            path.moveTo(initialTransform.map(p2));
            path.lineTo(snapMouse2.p1());
        }

        // draw the side handle bars
        if (isEditing == true && !sideHandles().isEmpty()) {
            path.moveTo(initialTransform.map(p1));
            path.lineTo(initialTransform.map(*sideHandles()[0]));
            path.lineTo(initialTransform.map(*sideHandles()[1]));
            path.moveTo(initialTransform.map(p2));
            path.lineTo(initialTransform.map(*sideHandles()[2]));
            path.lineTo(initialTransform.map(*sideHandles()[3]));
            path.moveTo(initialTransform.map(p1));
            path.lineTo(initialTransform.map(*sideHandles()[4]));
            path.lineTo(initialTransform.map(*sideHandles()[5]));
            path.moveTo(initialTransform.map(p2));
            path.lineTo(initialTransform.map(*sideHandles()[6]));
            path.lineTo(initialTransform.map(*sideHandles()[7]));
        }

        drawPreview(gc,path);

        if (handles().size() >= 3 && isSnappingActive()) {
            path = QPainterPath(); // clear

            // Create transform for conversion into easier local coordinate system
            // - Rotated so horizon is perfectly horizonal
            // - Translated so 3rd handle is the origin
            QTransform t = QTransform();
            t.rotate(QLineF(*handles()[0],*handles()[1]).angle());
            t.translate(-handles()[2]->x(),-handles()[2]->y());

            const QTransform inv = t.inverted();

            const QPointF vp_a = t.map(*handles()[0]);
            const QPointF vp_b = t.map(*handles()[1]);
            const QPointF center = t.map(*handles()[2]);

            const QLineF horizon = QLineF(vp_a,vp_b);
            QLineF vertical = horizon.normalVector();
            vertical.translate(center - vertical.p1());
            QPointF cov = horizon.center();
            horizon.intersect(vertical,&cov);

            if ((vp_a.x() < center.x() && vp_b.x() > center.x()) ||
                (vp_a.x() > center.x() && vp_b.x() < center.x())) {
                if (isEditing) {
                    // Draw vertical line, but only if the center is between both VPs
                    QLineF drawn_vertical = initialTransform.map(inv.map(vertical));
                    KisAlgebra2D::intersectLineRect(drawn_vertical,viewport);
                    path.moveTo(drawn_vertical.p1());
                    path.lineTo(drawn_vertical.p2());
                }
                if (assistantVisible) {
                    // Display a notch to represent the center of vision
                    path.moveTo(initialTransform.map(inv.map(cov+QPointF(0,-10))));
                    path.lineTo(initialTransform.map(inv.map(cov+QPointF(0,10))));
                }
                drawPreview(gc,path);
                path = QPainterPath(); // clear
            }

            const qreal gap = QLineF(horizon.center(),cov).length();
            // size is the radius of the perspective's 90 degree cone of vision
            // Also represents unit size of grid square
            const qreal size = sqrt(pow(horizon.length()/2.0,2) - pow(gap,2));
            const QPointF upper = cov+QPointF(0,size);
            const QPointF lower = cov-QPointF(0,size);

            // Set up the fading effect for the grid lines
            // Needed so the grid density doesn't look distracting
            QColor color = effectiveAssistantColor();
            QGradient fade = QLinearGradient(initialTransform.map(inv.map(upper)),
                                             initialTransform.map(inv.map(lower)));
            color.setAlphaF(0);
            fade.setColorAt(0.2, effectiveAssistantColor());
            fade.setColorAt(0.5, color);
            fade.setColorAt(0.8, effectiveAssistantColor());
            const QPen pen = gc.pen();
            const QBrush new_brush = QBrush(fade);
            const QPen new_pen = QPen(new_brush, pen.width(), pen.style());
            gc.setPen(new_pen);

            const QList<QPointF> station_points = {upper, lower};
            const QList<QPointF> vanishing_points = {vp_a, vp_b};

            // Draw grid lines above and below the horizon
            Q_FOREACH (const QPointF sp, station_points) {

                // Draw grid lines towards each vanishing point
                Q_FOREACH (const QPointF vp, vanishing_points) {

                    // Interval between each grid line, uses grid density specified by user
                    const qreal initial_angle = QLineF(sp, vp).angle();
                    const qreal interval = size*m_gridDensity / cos((initial_angle - 90) * M_PI/180);
                    const QPointF translation = QPointF(interval, 0);

                    // Draw grid lines originating from both the left and right of the central vertical line
                    Q_FOREACH (const int dir, QList<int>({-1, 1})) {

                        // Limit at 300 grid lines per direction, reasonable even for m_gridDensity=0.1;
                        for (int i = 0; i <= 300; i++) {
                            const QLineF gridline = QLineF(sp + translation * i * dir, vp);

                            // Don't bother drawing lines that are nearly parallel to horizon
                            const qreal angle = gridline.angle();
                            if (angle < 0.25 || angle > 359.75 || (angle < 180.25 && angle > 179.75)) {
                                break;
                            }

                            QLineF drawn_gridline = initialTransform.map(inv.map(gridline));
                            KisAlgebra2D::intersectLineRect(drawn_gridline, viewport);
                            path.moveTo(initialTransform.map(inv.map(vp)));
                            path.lineTo(drawn_gridline.p2());
                        }
                    }
                }
            }

            gc.drawPath(path);
        }
    }

    gc.restore();
    //KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);
}

void TwoPointAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    Q_UNUSED(gc);
    Q_UNUSED(converter);
    Q_UNUSED(assistantVisible);
    if (!m_canvas || !isAssistantComplete()) {
        return;
    }

    if (assistantVisible == false ||   m_canvas->paintingAssistantsDecoration()->isEditingAssistants()) {
        return;
    }
}

QPointF TwoPointAssistant::getEditorPosition() const
{
    return *handles()[2];
}

void TwoPointAssistant::setGridDensity(double density)
{
    m_gridDensity = density;
}

double TwoPointAssistant::gridDensity()
{
    return m_gridDensity;
}

bool TwoPointAssistant::isAssistantComplete() const
{
  return handles().size() == 3;
}

void TwoPointAssistant::saveCustomXml(QXmlStreamWriter* xml)
{
    xml->writeStartElement("gridDensity");
    xml->writeAttribute("value", KisDomUtils::toString( this->gridDensity()));
    xml->writeEndElement();
}

bool TwoPointAssistant::loadCustomXml(QXmlStreamReader* xml)
{
    if (xml && xml->name() == "gridDensity") {
        this->setGridDensity((float)KisDomUtils::toDouble(xml->attributes().value("value").toString()));
    }
    return true;
}

TwoPointAssistantFactory::TwoPointAssistantFactory()
{
}

TwoPointAssistantFactory::~TwoPointAssistantFactory()
{
}

QString TwoPointAssistantFactory::id() const
{
    return "two point";
}

QString TwoPointAssistantFactory::name() const
{
    return i18n("2 Point Perspective");
}

KisPaintingAssistant* TwoPointAssistantFactory::createPaintingAssistant() const
{
    return new TwoPointAssistant;
}
