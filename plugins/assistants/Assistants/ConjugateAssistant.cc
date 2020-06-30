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
    QLineF hl; // objective horizon line

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

	hl = QLineF(p1,p2);
	QLineF horizonLine = initialTransform.map(hl); // The apparent, SUBJECTIVE horizon line to draw
	KisAlgebra2D::intersectLineRect(horizonLine, viewport);

	QPainterPath path;

	// draw the horizon
	if (assistantVisible == true || isEditing == true) {
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

	QPointF p3;
	QPointF cov;
	if (handles().size() >= 3) {
	    p3 = *handles()[2];
	    QLineF norm = hl.normalVector();
	    norm.translate(-norm.p1()+p3);
	    hl.intersect(norm,&cov);

	    // p3 is invalid if cov doesnt lie somewhere between p1 and p2, so set a valid cov
	    if(!((p1.y() <= p2.y() && cov.y() >= p1.y() && cov.y() <= p2.y()) ||
		 (p1.y() >= p2.y() && cov.y() <= p1.y() && cov.y() >= p2.y()) ||
		 (p1.x() <= p2.x() && cov.x() >= p1.x() && cov.x() <= p2.x()) ||
		 (p1.x() >= p2.x() && cov.x() <= p1.x() && cov.x() >= p2.x()))) {
		cov = QLineF(p1,p2).center();
		p3 = QLineF(p1,p2).center();
		norm.translate(-norm.p1()+p3);
		*handles()[2] = norm.p2();
	    }

	    // draw the vertical normal line
	    if (isEditing == true) {
		QLineF normalLine = initialTransform.map(norm);
		KisAlgebra2D::intersectLineRect(normalLine, viewport);
		path.moveTo(normalLine.p1());
		path.lineTo(normalLine.p2());
		drawPath(gc, path, isSnappingActive());
	    }
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

	if (handles().size() >= 3) {
	    const QLineF hl = QLineF(*handles()[0], *handles()[1]);
	    const qreal radius = hl.length() / 2.0;
	    const qreal gap = QLineF(hl.center(),cov).length();
	    QLineF vertical;
	    vertical = hl.normalVector();
	    vertical.translate(cov - vertical.p1());
	    vertical.setLength(sqrt((radius*radius) - (gap*gap)));
	    const QPointF sp = vertical.p2();

	    qreal hl_angle = hl.angle();
	    qreal interval = QLineF(sp,cov).length();
	    hl_angle > 180 ? hl_angle = hl_angle - 180: hl_angle = hl_angle;

	    QList<QPointF> points = QList<QPointF>({p1,p2});

	    Q_FOREACH (QPointF pt, points) {

		QPointF point = initialTransform.map(pt);
		qreal arm = QLineF(pt,cov).length();
		QLineF gridline = QLineF(pt,sp);
		const QLineF sp_line = QLineF(pt,sp);
		QLineF ray = initialTransform.map(gridline);
		KisAlgebra2D::intersectLineRect(ray, viewport);

		QLineF other_gridline;
		pt == p1 ? other_gridline = QLineF(p2,sp) : other_gridline = QLineF(p1,sp);

		qreal actual_interval = abs( interval / cos((other_gridline.angle()-hl_angle) * M_PI / 180) );

		qreal new_angle;
		qreal actual_angle;
		qreal mirroring_adjustment;

		for (int i = -50; i!=50; i++) {
		    new_angle = atan2(interval,arm + actual_interval*i) * 180 / M_PI;

		    if (sp_line.angle() - hl_angle <  90) {
			actual_angle = new_angle ;
			mirroring_adjustment = 360 - new_angle ;}

		    if (sp_line.angle() - hl_angle >  90)  {
			actual_angle = 180  - new_angle;
			mirroring_adjustment = new_angle + 180 ;}

		    if (sp_line.angle() - hl_angle >  180) {
			actual_angle = new_angle + 180 ;
			mirroring_adjustment = 180  - new_angle;}

		    if (sp_line.angle() - hl_angle >  270) {
			actual_angle = 360 - new_angle;
			mirroring_adjustment = new_angle ;}

		    gridline.setAngle(actual_angle + hl_angle);
		    ray = initialTransform.map(gridline);
		    KisAlgebra2D::intersectLineRect(ray, viewport);
		    path.moveTo(point);
		    path.lineTo(ray.p2());

		    gridline.setAngle(mirroring_adjustment + hl_angle);
		    ray = initialTransform.map(gridline);
		    KisAlgebra2D::intersectLineRect(ray, viewport);
		    path.moveTo(point);
		    path.lineTo(ray.p2());
		}
	    }

	    drawPreview(gc,path);
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
