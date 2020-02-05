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
{
}

KisPaintingAssistantSP ConjugateAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new ConjugateAssistant(*this, handleMap));
}

QPointF ConjugateAssistant::project(const QPointF& pt, const QPointF& strokeBegin)
{
    //Q_ASSERT(handles().size() == 1 || handles().size() == 5);
    //code nicked from the perspective ruler.
    qreal dx = pt.x() - strokeBegin.x();
    qreal dy = pt.y() - strokeBegin.y();

    if (dx * dx + dy * dy < 4.0) {
        // allow some movement before snapping
        return strokeBegin;
    }

    QLineF first = QLineF(strokeBegin, *handles()[0]);
    QLineF second = QLineF(strokeBegin, *handles()[1]);

    QList<qreal> angles = QList<qreal>({first.angleTo(QLineF(strokeBegin,pt)),
					second.angleTo(QLineF(strokeBegin,pt)),
      });

    // for (int i = 0; i < 2; i++) {
    //   ENTER_FUNCTION() << "angle: " << angles[i];
    //   if (angles[i] > 180) {
    // 	angles[i] = 360 - angles[i] ;
    //   }
    // }

    for (int i = 0; i < angles.length(); i++) {
      ENTER_FUNCTION() << i << angles[i];

      if (90 < angles[i] && angles[i] < 270) {
	angles[i] = qAbs(angles[i] - 180);
      }

      if (angles[i] > 270) {
	angles[i] = 360 - angles[i];
      }
      ENTER_FUNCTION() << i << angles[i];
    }

    //dbgKrita<<strokeBegin<< ", " <<*handles()[0];
    QLineF snapLine;
    if (angles[0]< angles[1]) {
      snapLine = QLineF(*handles()[0], strokeBegin);
    } else {
      snapLine = QLineF(*handles()[1], strokeBegin);
    }

    //= QLineF(*handles()[0], strokeBegin);

    dx = snapLine.dx();
    dy = snapLine.dy();

    const qreal dx2 = dx * dx;
    const qreal dy2 = dy * dy;
    const qreal invsqrlen = 1.0 / (dx2 + dy2);

    QPointF r(dx2 * pt.x() + dy2 * snapLine.x1() + dx * dy * (pt.y() - snapLine.y1()),
              dx2 * snapLine.y1() + dy2 * pt.y() + dx * dy * (pt.x() - snapLine.x1()));

    r *= invsqrlen;
    return r;
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

	    QLineF normalLine = initialTransform.map(norm); // subjective normal line for drawing

	    // draw the vertical normal line
	    if (assistantVisible == true || isEditing == true) {
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
