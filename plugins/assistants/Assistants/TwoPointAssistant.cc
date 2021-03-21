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
    , m_horizon(rhs.m_horizon)
    , m_cov(rhs.m_cov)
    , m_sp(rhs.m_sp)
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
        QLineF vertical_snap = QLineF(m_cov, m_sp); // nickde from ParallelRulerAssistant.cc
        QPointF translation = (m_cov-strokeBegin)*-1.0;
        vertical_snap = vertical_snap.translated(translation);

        // determine whether the horizontal or vertical line is closer to the point
        m_snapLine = distsqr(pt, vp_snap_a) < distsqr(pt, vp_snap_b) ? vp_snap_a : vp_snap_b;
        m_snapLine = distsqr(pt, m_snapLine) < distsqr(pt, vertical_snap) ? m_snapLine : vertical_snap;
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

            // draw the vertical normal line
            if (previewVisible == true) {
                QLineF norm = m_horizon.normalVector();
                QLineF normalLine = initialTransform.map(norm);
                normalLine.translate(mousePos - normalLine.p1());
                KisAlgebra2D::intersectLineRect(normalLine, viewport);
                path.moveTo(normalLine.p1());
                path.lineTo(normalLine.p2());
                drawPreview(gc, path);
            }

            // Now we will draw the grid lines.
            if (assistantVisible == true) {
                path = QPainterPath(); // clear

                // First set up fading gradient for grid lines
                const QPointF translation = m_cov - m_sp;
                QGradient fade = QLinearGradient(initialTransform.map(m_cov + translation), initialTransform.map(m_sp));
                QColor c = effectiveAssistantColor();
                c.setAlphaF(0);
                fade.setColorAt(0.2, effectiveAssistantColor());
                fade.setColorAt(0.5, c);
                fade.setColorAt(0.8, effectiveAssistantColor());
                QPen p = gc.pen();
                QBrush new_b = QBrush(fade);
                QPen new_p = QPen(new_b, p.width(), p.style());
                gc.setPen(new_p);

                // to draw the grid, we're gonna do the exact same calculations for both VPs
                const QList<QPointF> v_points = QList<QPointF>({p1, p2});

                // the furthest grid lines (most parallel to horizon) shall be at least as far as this point
                const QPointF farthest_point = m_cov + ((m_sp - m_cov) / 40.0);

                // Radius of the cone of vision
                const qreal radius = sqrt(pow(m_cov.x()-m_sp.x(),2) + pow(m_cov.y()-m_sp.y(),2));

                QLineF grid_line;
                QLineF mirror_grid_line;
                QLineF ray;	// this is what actually gets drawn
                QLineF mirror_ray;

                for (QPointF vp : v_points) {

                    // calculate interval between each grid line
                    qreal acute_angle = acuteAngle(QLineF(m_sp, vp).angleTo(m_horizon));
                    const qreal interval = (radius / sin(acute_angle*M_PI/180)) * m_gridDensity;

                    if (qIsNaN(interval)) {
                        // happens the instant user places 3rd handle (no gap between 2nd and 3rd handle)
                        break;
                    }

                    // How the farthest grid line gets drawn depends on how far away the relevant VP is
                    const qreal cov_to_vp = sqrt(pow(m_cov.x()-vp.x(),2) + pow(m_cov.y()-vp.y(),2));
                    if (cov_to_vp > radius) {
                        grid_line = QLineF(vp, farthest_point);
                    } else {
                        // The point where VP is on the cone. Neither inside not outside cone of vision
                        const QPointF threshold_point = m_cov + ( vp - m_cov ) * (radius / cov_to_vp);
                        const QPointF translation = vp - threshold_point;
                        grid_line = QLineF(threshold_point + translation, farthest_point + translation);
                    }

                    // the base point is where the first grid_line passes through the "foot" of the viewer
                    QPointF base_point;
                    const QPointF translation = m_sp - m_cov;
                    QPointF far_point;
                    QLineF(*handles()[0] + translation, *handles()[1] + translation).intersect(grid_line, &far_point);
                    const qreal base_distance = QLineF(m_sp, far_point).length();
                    QLineF base_gap = QLineF(m_sp, far_point);
                    base_gap.setLength(base_distance - remainder(base_distance, interval));
                    base_point = base_gap.p2();

                    // we will apply a translation to the base point to draw each of the following grid lines
                    QLineF interval_vector = QLineF(base_point, m_sp);

                    // sanity check
                    if (qIsNaN(interval_vector.length())) {
                      break;
                    }

                    interval_vector.setLength(interval);
                    const QPointF interval_translation = QPointF(interval_vector.dx(), interval_vector.dy());

                    // final sanity check
                    if (qIsNull(m_sp.x()) && qIsNull(m_sp.y())) {
                        break;
                    }

                    // initialize variables to control the grid drawing loop
                    const qreal threshold_length = QLineF(far_point, vp + translation).length();
                    qreal current_length = 0;
                    qreal acute_grid_angle = acuteAngle(grid_line.angleTo(m_horizon));
                    qreal curr_grid_angle = acute_grid_angle;
                    int i = 0;

                    // here be dragons, this code runs for *every* subsequent grid line
                    while (i < 300) {
                        grid_line = QLineF(vp, base_point + i*interval_translation);
                        mirror_grid_line = grid_line;
                        mirror_grid_line.setAngle(grid_line.angle() + 2*grid_line.angleTo(m_horizon));

                        mirror_ray = initialTransform.map(mirror_grid_line);
                        ray = initialTransform.map(grid_line);

                        KisAlgebra2D::intersectLineRect(ray, viewport);
                        KisAlgebra2D::intersectLineRect(mirror_ray, viewport);
                        path.moveTo(ray.p1());
                        path.lineTo(initialTransform.map(vp));
                        path.lineTo(mirror_ray.p1());

                        current_length = QLineF(base_point, grid_line.p2()).length();
                        curr_grid_angle = acuteAngle(grid_line.angleTo(m_horizon));

                        if (cov_to_vp > radius) {
                            if (!(current_length < threshold_length && cov_to_vp > radius)) {
                                break;
                            }
                        } else {
                            if (!((curr_grid_angle >= acute_grid_angle ||
                                 current_length < threshold_length) &&
                                cov_to_vp < radius)) {
                                break;
                            }
                        }
                        i++;
                    }
                    gc.drawPath(path);
                }
            }

        }
    }

    gc.restore();
    //KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);
}

void TwoPointAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    if (!m_canvas || !isAssistantComplete()) {
        return;
    }

    if (assistantVisible == false ||   m_canvas->paintingAssistantsDecoration()->isEditingAssistants()) {
        return;
    }

    QTransform initialTransform = converter->documentToWidgetTransform();
    QPainterPath path;
    QPointF centerOfVision = initialTransform.map(m_cov);

    path.moveTo(QPointF(centerOfVision.x() - 10.0, centerOfVision.y() - 10.0));
    path.lineTo(QPointF(centerOfVision.x() + 10.0, centerOfVision.y() + 10.0));

    path.moveTo(QPointF(centerOfVision.x() - 10.0, centerOfVision.y() + 10.0));
    path.lineTo(QPointF(centerOfVision.x() + 10.0, centerOfVision.y() - 10.0));

    drawPath(gc, path, isSnappingActive());
}

QPointF TwoPointAssistant::getEditorPosition() const
{
    QLineF arm_0 = QLineF(m_sp, *handles()[0]).unitVector();
    QLineF arm_1 = QLineF(m_sp, *handles()[1]).unitVector();
    QPointF bisect_point = QLineF(arm_1.p2(), arm_0.p2()).center();
    QLineF bisect_line = QLineF(m_sp, bisect_point);

    // the "diagonal" vanishing point
    QPointF diagonal_point;
    m_horizon.intersect(bisect_line, &diagonal_point);

    return diagonal_point;
}

void TwoPointAssistant::setHorizon(const QPointF a, const QPointF b)
{
    m_horizon = QLineF(a,b);
}

QLineF TwoPointAssistant::horizon()
{
    return m_horizon;
}

void TwoPointAssistant::setCov(const QPointF a, const QPointF b, const QPointF c)
{
    if (qFuzzyCompare(a.x(), b.x()) && qFuzzyCompare(a.y(), b.y())) {
        m_sp = a;
    } else {
        float px = 0;
        float py = 0;

        if (qFuzzyCompare(b.y(),a.y())) {
            px = c.x();
            py = a.y();
        } else if (qFuzzyCompare(b.x(),a.x())) {
            px = a.x();
            py = c.y();
        } else {
            float m_num = (b.y() - a.y());
            float m_denom = (b.x() - a.x());
            float m = m_num / m_denom;
            px = (m * m * a.x() + m * c.y() - m * a.y() + c.x()) / (m * m + 1);
            py = m * px + a.y() - m * a.x();
        }

        m_cov = QPointF(px, py);
        QLineF gap = QLineF(m_cov,m_horizon.center());
        if (gap.length() > m_horizon.length() / 2.0) {
            m_cov = m_horizon.center();
        }
    }
    *handles()[2] = m_cov;
}

QPointF TwoPointAssistant::cov()
{
    return m_cov;
}

void TwoPointAssistant::setSp(const QPointF a, const QPointF b, const QPointF c)
{
    if (qFuzzyCompare(a.x(), b.x()) && qFuzzyCompare(a.y(), b.y())) {
        m_sp = a;
    } else {
        float px = 0;
        float py = 0;
        QLineF gap = QLineF(m_cov,m_horizon.center());
        float h = pow(m_horizon.length() / 2.0,2) - pow(gap.length(),2);

        QPointF _c = c;

        if (h < 0) {
            h = pow(m_horizon.length() / 2.0,2);
            QLineF alt = QLineF(m_horizon.center(), b).normalVector();
            _c = alt.center();
        }

        if (qFuzzyCompare(b.y(),a.y())) {
            px = _c.x();
            py = _c.y() - sqrt(h);
        } else if (qFuzzyCompare(b.x(),a.x())) {
            px = _c.x() - sqrt(h);
            py = _c.y();
        } else {
            float m_num = (b.y() - a.y());
            float m_denom = (b.x() - a.x());
            float m = m_num / m_denom;
            float dx = sqrt(h) * sin(m_horizon.angle()*M_PI/180);
            px = _c.x() + dx;
            py = _c.y() + (_c.x() / m) - (px / m);
        }

        m_sp = QPointF(px,py);
    }

}

QPointF TwoPointAssistant::sp()
{
    return m_sp;
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

    xml->writeStartElement("horizon");
    xml->writeAttribute("x1", KisDomUtils::toString(this->horizon().p1().x()));
    xml->writeAttribute("y1", KisDomUtils::toString(this->horizon().p1().y()));
    xml->writeAttribute("x2", KisDomUtils::toString(this->horizon().p2().x()));
    xml->writeAttribute("y2", KisDomUtils::toString(this->horizon().p2().y()));
    xml->writeEndElement();

    xml->writeStartElement("cov");
    xml->writeAttribute("x", KisDomUtils::toString(this->cov().x()));
    xml->writeAttribute("y", KisDomUtils::toString(this->cov().y()));
    xml->writeEndElement();

    xml->writeStartElement("sp");
    xml->writeAttribute("x", KisDomUtils::toString(this->sp().x()));
    xml->writeAttribute("y", KisDomUtils::toString(this->sp().y()));
    xml->writeEndElement();
}

bool TwoPointAssistant::loadCustomXml(QXmlStreamReader* xml)
{
    if (xml && xml->name() == "gridDensity") {
        this->setGridDensity((float)KisDomUtils::toDouble(xml->attributes().value("value").toString()));
    }

    if (xml && xml->name() == "horizon") {
        QPointF p1 = QPointF((float)KisDomUtils::toDouble(xml->attributes().value("x1").toString()),
                             (float)KisDomUtils::toDouble(xml->attributes().value("y1").toString()));
        QPointF p2 = QPointF((float)KisDomUtils::toDouble(xml->attributes().value("x2").toString()),
                             (float)KisDomUtils::toDouble(xml->attributes().value("y2").toString()));

        this->m_horizon = QLineF(p1,p2);
    }

    if (xml && xml->name() == "cov") {
        QPointF cov = QPointF((float)KisDomUtils::toDouble(xml->attributes().value("x").toString()),
                              (float)KisDomUtils::toDouble(xml->attributes().value("y").toString()));
        this->m_cov = cov;
    }

    if (xml && xml->name() == "sp") {
        QPointF sp = QPointF((float)KisDomUtils::toDouble(xml->attributes().value("x").toString()),
                             (float)KisDomUtils::toDouble(xml->attributes().value("y").toString()));
        this->m_sp = sp;
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
