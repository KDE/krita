#include "ConjugateAssistant.h"
#include "kis_debug.h"
#include <klocalizedstring.h>

#include <QPainter>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include <kis_algebra_2d.h>
#include <kis_dom_utils.h>
#include <math.h>
#include <QtCore/qmath.h>

ConjugateAssistant::ConjugateAssistant()
    : KisPaintingAssistant("conjugate", i18n("Conjugate assistant"))
{
}

ConjugateAssistant::ConjugateAssistant(const ConjugateAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisPaintingAssistant(rhs, handleMap)
    , m_canvas(rhs.m_canvas)
    , m_referenceLineDensity(rhs.m_referenceLineDensity)
    , m_snapLine(rhs.m_snapLine)
    , m_horizon(rhs.m_horizon)
    , m_cov(rhs.m_cov)
    , m_sp(rhs.m_sp)
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

KisPaintingAssistantSP ConjugateAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new ConjugateAssistant(*this, handleMap));
}

QPointF ConjugateAssistant::project(const QPointF& pt, const QPointF& strokeBegin)
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

void ConjugateAssistant::endStroke()
{
    m_snapLine = QLineF();
}

QPointF ConjugateAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin)
{
    return project(pt, strokeBegin);
}

void ConjugateAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    gc.save();
    gc.resetTransform();
    QPointF mousePos(0,0);

    if (canvas){
	//simplest, cheapest way to get the mouse-position//
	mousePos= canvas->canvasWidget()->mapFromGlobal(QCursor::pos());
	m_canvas = canvas;
    }
    else {
	mousePos = QCursor::pos();//this'll give an offset//
	dbgFile<<"canvas does not exist in ruler, you may have passed arguments incorrectly:"<<canvas;
    }

    const QTransform initialTransform = converter->documentToWidgetTransform();
    bool isEditing = canvas->paintingAssistantsDecoration()->isEditingAssistants();

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
	    drawPreview(gc, path);
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
	    drawPreview(gc,path);
	}

	if (handles().size() >= 3 && isSnappingActive()) {
	    const QPointF p3 = *handles()[2];

	    // draw the vertical normal line
	    if (isEditing == true) {
		QLineF norm = m_horizon.normalVector();
		norm.translate(-norm.p1()+p3);
		QLineF normalLine = initialTransform.map(norm);
		KisAlgebra2D::intersectLineRect(normalLine, viewport);
		path.moveTo(normalLine.p1());
		path.lineTo(normalLine.p2());
		drawPath(gc, path, isSnappingActive());
	    }

	    // Now we will draw the grid lines.
	    // to draw the grid, we're gonna do the exact same calculations for both VPs
            const QList<QPointF> v_points = QList<QPointF>({p1, p2});

	    // We will start by drawing the "furthest" gridline, ie nearly parallel to the horizon
	    // the furthest grid line shall be at least as far as this point
	    const QPointF farthest_point = m_cov + ((m_sp - m_cov) / 40.0);

	    // Radius of the cone of vision
	    const qreal radius = sqrt(pow(m_cov.x()-m_sp.x(),2) + pow(m_cov.y()-m_sp.y(),2));

	    QLineF grid_line;
	    QLineF ray;	// this is what actually gets drawn

            for (QPointF vp : v_points) {

		// How the farthest grid line gets drawn depends on how far away the relevant VP is
		const qreal cov_to_vp = sqrt(pow(m_cov.x()-vp.x(),2) + pow(m_cov.y()-vp.y(),2));
                if (cov_to_vp > radius) {
		    grid_line = QLineF(vp, farthest_point);
		    ray = initialTransform.map(grid_line);
                } else {
		    // The point where VP is on the cone. Neither inside not outside cone of vision
		    const QPointF threshold_point = m_cov + ( vp - m_cov ) * (radius / cov_to_vp);
		    const QPointF translation = vp - threshold_point;
		    grid_line = QLineF(threshold_point + translation, farthest_point + translation);
		    ray = initialTransform.map(grid_line);
                }

		KisAlgebra2D::intersectLineRect(ray, viewport);
		path.moveTo(initialTransform.map(vp));
		path.lineTo(ray.p1());

		// calculate interval between each grid line
		qreal acute_angle = acuteAngle(QLineF(m_sp, vp).angleTo(m_horizon));
		const qreal interval = radius / sin(acute_angle*M_PI/180);

		// the base point is where the grid line we drew earlier passes through
		const QPointF translation = m_sp - m_cov;
		const QLineF base_line = QLineF(*handles()[0] + translation, *handles()[1] + translation);
		QPointF base_point;
		base_line.intersect(grid_line, &base_point);

		// we will apply a translation to the base point to draw each of the following grid lines
		QLineF interval_vector = QLineF(base_point, m_sp);
		interval_vector.setLength(interval);
		const QPointF interval_translation = QPointF(interval_vector.dx(), interval_vector.dy());

		// initialize variables to control the grid drawing loop
		const qreal threshold_length = QLineF(base_point, vp + translation).length();
		qreal current_length = 0;
		qreal acute_grid_angle = acuteAngle(grid_line.angleTo(m_horizon));
		qreal curr_grid_angle = acute_grid_angle;
		bool draw_next = true;
		int i = 0;

		// here be dragons, this code runs for *every* subsequent grid line
                while (draw_next == true) {
		    grid_line = QLineF(vp, base_point + i*interval_translation);

		    ray = initialTransform.map(grid_line);
		    KisAlgebra2D::intersectLineRect(ray, viewport);
		    path.moveTo(initialTransform.map(vp));
		    path.lineTo(ray.p1());

                    current_length = QLineF(base_point, grid_line.p2()).length();
		    curr_grid_angle = acuteAngle(grid_line.angleTo(m_horizon));
		    i++;

		    cov_to_vp > radius ?
			draw_next = current_length < threshold_length && cov_to_vp > radius :
			draw_next = (curr_grid_angle >= acute_grid_angle || current_length < threshold_length ) && cov_to_vp < radius ;
                }
            }
	    drawPreview(gc, path);
	}
    }

    gc.restore();
    //KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);
}

void ConjugateAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    if (!m_canvas || !isAssistantComplete()) {
        return;
    }

    if (assistantVisible == false ||   m_canvas->paintingAssistantsDecoration()->isEditingAssistants()) {
        return;
    }

    QTransform initialTransform = converter->documentToWidgetTransform();
    QPainterPath path;

    if (isAssistantComplete()){
      QPointF cov;
      QLineF hl = QLineF(*handles()[0],*handles()[1]);
      QLineF vertical = hl.normalVector();
      vertical.translate(*handles()[2] - vertical.p1());
      vertical.intersect(hl, &cov);
      QPointF centerOfVision = initialTransform.map(cov);

      path.moveTo(QPointF(centerOfVision.x() - 10.0, centerOfVision.y() - 10.0));
      path.lineTo(QPointF(centerOfVision.x() + 10.0, centerOfVision.y() + 10.0));

      path.moveTo(QPointF(centerOfVision.x() - 10.0, centerOfVision.y() + 10.0));
      path.lineTo(QPointF(centerOfVision.x() + 10.0, centerOfVision.y() - 10.0));

      drawPath(gc, path, isSnappingActive());
    }
}

QPointF ConjugateAssistant::getEditorPosition() const
{
    return *handles()[2];
}

void ConjugateAssistant::setReferenceLineDensity(float value)
{
}

float ConjugateAssistant::referenceLineDensity()
{
  float f = 0.0;
  return f;
}

void ConjugateAssistant::setHorizon(const QPointF a, const QPointF b)
{
    m_horizon = QLineF(a,b);
}

QLineF ConjugateAssistant::horizon()
{
    return m_horizon;
}

void ConjugateAssistant::setCov(const QPointF a, const QPointF b, const QPointF c)
{
    float px = 0;
    float py = 0;

    if (qFuzzyCompare(b.y(),a.y())) {
	px = c.x();
	py = a.y();
    } else if (qFuzzyCompare(b.x(),a.x())) {
	py = a.x();
	px = c.y();
    } else {
	float m_num = (b.y() - a.y());
	float m_denom = (b.x() - a.x());
	float m = m_num / m_denom;
	px = (m * m * a.x() + m * c.y() - m * a.y() + c.x()) / (m * m + 1);
	py = m * px + a.y() - m * a.x();
    }

    m_cov = QPointF(px, py);
    *handles()[2] = m_cov;
}

QPointF ConjugateAssistant::cov()
{
    return m_cov;
}

void ConjugateAssistant::setSp(const QPointF a, const QPointF b, const QPointF c)
{
    float px = 0;
    float py = 0;
    QLineF gap = QLineF(m_cov,m_horizon.center());

    if (qFuzzyCompare(b.y(),a.y())) {
	px = c.x();
	py = c.y() - (sqrt(pow(m_horizon.length() / 2.0,2) - pow(gap.length(),2)));
    } else if (qFuzzyCompare(b.x(),a.x())) {
	py = c.x() - (sqrt(pow(m_horizon.length() / 2.0,2) - pow(gap.length(),2)));
	px = c.y();
    } else {
	float m_num = (b.y() - a.y());
	float m_denom = (b.x() - a.x());
	float m = m_num / m_denom;
	float dx = sqrt(pow(m_horizon.length() / 2.0,2) - pow(gap.length(),2)) * sin(m_horizon.angle()*M_PI/180);
	px = c.x() + dx;
	py = c.y() + (c.x() / m) - (px / m);
    }

    m_sp = QPointF(px,py);
}

QPointF ConjugateAssistant::sp()
{
    return m_sp;
}

bool ConjugateAssistant::isAssistantComplete() const
{
  return handles().size() >= 3;
}

void ConjugateAssistant::saveCustomXml(QXmlStreamWriter* xml)
{
}

bool ConjugateAssistant::loadCustomXml(QXmlStreamReader* xml)
{
  bool b = true;
  return b;
}

ConjugateAssistantFactory::ConjugateAssistantFactory()
{
}

ConjugateAssistantFactory::~ConjugateAssistantFactory()
{
}

QString ConjugateAssistantFactory::id() const
{
    return "conjugate";
}

QString ConjugateAssistantFactory::name() const
{
    return i18n("Conjugate");
}

KisPaintingAssistant* ConjugateAssistantFactory::createPaintingAssistant() const
{
    return new ConjugateAssistant;
}
