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
    , m_spacing(rhs.m_spacing)
    , m_spacingTilt(rhs.m_spacingTilt)
    , m_fade(rhs.m_fade)
    , m_count(rhs.m_count)
    , m_useSpacing(rhs.m_useSpacing)
    , sp(rhs.sp)
    , hl(rhs.hl)
    , cov(rhs.cov)
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
    mousePos= canvas->canvasWidget()->mapFromGlobal(QCursor::pos());
    m_canvas = canvas;
  }
  else {
    mousePos = QCursor::pos();//this'll give an offset//
    dbgFile<<"canvas does not exist in ruler, you may have passed arguments incorrectly:"<<canvas;
  }

  const QTransform initialTransform = converter->documentToWidgetTransform();

  if (handles().size() >= 2) {

    const QPointF p1 = initialTransform.map(*handles()[0]); 
    const QPointF p2 = initialTransform.map(*handles()[1]); 
    const QRect viewport= gc.viewport();

    QPainterPath path;
    QLineF horizonLine = QLineF(p1, p2);
    KisAlgebra2D::intersectLineRect(horizonLine, viewport);

    if (assistantVisible == true) {
      path.moveTo(horizonLine.p1());
      path.lineTo(horizonLine.p2());
    }
    
    if (isAssistantComplete()) {
      
      // persp lines, only draw when not editing eg when on brush tool
      if (canvas->paintingAssistantsDecoration()->isEditingAssistants() == false
      	  && previewVisible == true
      	  && isSnappingActive()) {
        QLineF snapMouse1 = QLineF(p1, mousePos);
        QLineF snapMouse2 = QLineF(p2, mousePos);
        KisAlgebra2D::intersectLineRect(snapMouse1, viewport);
        KisAlgebra2D::intersectLineRect(snapMouse2, viewport);
        path.moveTo(p1);
        path.lineTo(snapMouse1.p1());
        path.moveTo(p2);
        path.lineTo(snapMouse2.p1());
      }

      if (canvas->paintingAssistantsDecoration()->isEditingAssistants() == true && assistantVisible == true){
    	// normal
    	QPointF p3 = initialTransform.map(*handles()[2]);
    	float rise = horizonLine.dy();
    	float run = horizonLine.dx();
    	QPointF p4;
    	if (rise == 0) {
    	  p4 = QPointF(p3.x(),p3.y()+100);
    	} else {
    	  p4 = QPointF(p3.x()+100,p3.y()-(100/(rise/run)));
    	}
    	QLineF normal = QLineF(p3,p4);
    	KisAlgebra2D::intersectLineRect(normal, viewport);
    	QPointF* inter = new QPointF();
    	horizonLine.intersect(normal,inter);
	
    	// station point
    	QPointF h1 = *handles()[0];
    	QPointF h2 = *handles()[1];
    	QPointF h3 = *handles()[2];
	
    	hl = QLineF(h1,h2);
    	QLineF norm = hl.normalVector();
	
    	norm.translate(h3 - h1);
	
    	norm.intersect(hl,&cov);
	
    	qreal h1dst = QLineF(h1,cov).length();
    	qreal h2dst = QLineF(h2,cov).length();
	
    	qreal radius = (h1dst + h2dst) / 2;
    	qreal spdst = qSqrt(qPow(radius,2) - qPow(radius-qMin(h1dst,h2dst),2));
	
    	norm.setP1(cov);
    	norm.setLength(spdst);
	norm.setP1(norm.p2());
	norm.setP2(cov);
	norm.setLength(spdst*2);
	
    	if (sideHandles().isEmpty()) {
    	  addHandle(new KisPaintingAssistantHandle(norm.p1()), HandleType::SIDE);
    	  addHandle(new KisPaintingAssistantHandle(norm.p2()), HandleType::SIDE);
	}

        sideHandles()[0]->setX(norm.p1().x());
	sideHandles()[0]->setY(norm.p1().y());
	sideHandles()[1]->setX(norm.p2().x());
	sideHandles()[1]->setY(norm.p2().y());

    	sp = norm.p1();
	
    	QPointF snapPoint = *sideHandles()[0];
	
    	// finally draw Station Point and Normal
    	if (// inter must be between both p1 and p2
    	    (p2.x() < inter->x() && inter->x() < p1.x()) ||
    	    (p2.y() < inter->y() && inter->y() < p1.y()) ||
    	    (p2.x() > inter->x() && inter->x() > p1.x()) ||
    	    (p2.y() > inter->y() && inter->y() > p1.y())
    	    ) {
    	  // normal
    	  path.moveTo(normal.p1());
    	  path.lineTo(normal.p2());
    	  // conjugate ray, only draw when in assistant tool
    	  if (canvas->paintingAssistantsDecoration()->isEditingAssistants()) {
    	    path.moveTo(initialTransform.map(h1));
    	    path.lineTo(initialTransform.map(snapPoint));
    	    path.moveTo(initialTransform.map(h2));
    	    path.lineTo(initialTransform.map(snapPoint));
    	  }
    	} else {
    	  QPointF mid = QPointF((h2.x()-h1.x())/2+h1.x(),(h2.y()-h1.y())/2+h1.y()); // ?????????
    	  norm = QLineF(mid,h1).normalVector();
    	  norm.setLength(QLineF(mid,h1).length());
	  
    	  if (sideHandles().isEmpty()) {
    	    addHandle(new KisPaintingAssistantHandle(norm.p2()), HandleType::SIDE);
    	    sideHandles()[0]->setX(norm.p2().x());
    	    sideHandles()[0]->setY(norm.p2().y());
    	  } else {
    	    sideHandles()[0]->setX(norm.p2().x());
    	    sideHandles()[0]->setY(norm.p2().y());
    	  }
    	  sp = norm.p2();
    	}
      }

      if (assistantVisible == true && isSnappingActive()) {

    	// transform 
      QPointF tSP = initialTransform.map(sp);
      QPointF tCOV = initialTransform.map(cov);

      // these are already transformed
      QList<QPointF> vanishingPoints = QList<QPointF>({p1,p2});

      Q_FOREACH (QPointF p0, vanishingPoints) {
	
    	qreal horizonAngle = initialTransform.map(hl).angleTo(QLineF(0,0,100,0));

    	QLineF unit = QLineF(p0,tCOV).normalVector();
    	unit.setLength(1);

    	qreal square = 1.0;
	
    	QList<qreal> coefficients = QList<qreal>({1.0,-1.0});
    	QColor paintingColor = effectiveAssistantColor();
	
    	Q_FOREACH (qreal coefficient, coefficients)
    	  {
	    
    	    int count = 0;
    	    qreal opacity = 1.00;
    	    qreal tilt = coefficient*horizonAngle;

    	    QLinearGradient grad(QPointF(p0.x()+unit.dx()*700, p0.y()+unit.dy()*700),
    				 QPointF(p0.x()-unit.dx()*700, p0.y()-unit.dy()*700));
    	    grad.setSpread(QGradient::PadSpread);
    	    QColor colorStep3 = QColor(paintingColor.rgb());
    	    colorStep3.setAlphaF(0.0);
    	    grad.setColorAt(0.0, paintingColor);
    	    grad.setColorAt(0.50, colorStep3);
    	    grad.setColorAt(1.0, paintingColor);
	    
    	    QBrush brush_a(grad);
    	    brush_a.setStyle(Qt::LinearGradientPattern);
    	    QPen pen_a(brush_a, 1);
    	    pen_a.setStyle(Qt::SolidLine);
    	    pen_a.setCosmetic(true);
    	    gc.setPen(pen_a);

    	    gc.setOpacity(opacity);
    	    QPainterPath path;

    	    // QLineF groundLine = QLineF(tSP, p0);
    	    // KisAlgebra2D::intersectLineRect(groundLine, viewport);
    	    // path.moveTo(groundLine.p1());
    	    // path.lineTo(groundLine.p2());
	    
    	    gc.drawPath(path);

    	    qreal deviation = QLineF(tSP,p0).angleTo(QLineF(tSP,tCOV));
    	    if (deviation > 90) {
    	      deviation = 360 - deviation;
            }
    	    deviation = deviation * (M_PI/180);
            qreal interval = square/qCos(deviation);

    	    // normals
    	    QLineF normalLine = QLineF(tCOV,tSP);
    	    normalLine.translate(p0-tCOV);
    	    KisAlgebra2D::intersectLineRect(normalLine, viewport);
    	    path.moveTo(normalLine.p1());
    	    path.lineTo(normalLine.p2());

    	    for (qreal angle = atan(interval)*(180/M_PI)	+ 90.0+tilt;
    		 angle < 89.99					+ 90.0+tilt;
    		 angle = atan(interval*count)*(180/M_PI)	+ 90.0+tilt)
    	      {

    		QPointF unitAngle;
    		unitAngle.setX(p0.x() + coefficient * cos(angle * M_PI / 180));
    		unitAngle.setY(p0.y() +		      sin(angle * M_PI / 180));
		
    		gc.setOpacity(opacity);
		
    		QLineF snapLine = QLineF(p0, unitAngle);
		
    		KisAlgebra2D::intersectLineRect(snapLine, viewport);
		
    		path.moveTo(snapLine.p1());
    		path.lineTo(snapLine.p2());
    		//drawPreview(gc, path);//and we draw the preview.
    		KisPaintingAssistantHandleSP handle = handles()[0];
		
    		gc.drawPath(path);
		
    		count++;
    		if (opacity > 0) {opacity=opacity-m_fade;}
    		if (count > m_count) {break;}
    	      }
    	  }
      }
      }

    }

    // drawPreview(gc, path);//and we draw the preview.
    drawPath(gc, path, isSnappingActive());
    // gc.restore();
  }

  gc.restore();
  KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);
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
    QList<QPointF> points = QList<QPointF>({initialTransform.map(*handles()[0]),initialTransform.map(*handles()[1])});

    Q_FOREACH (QPointF p0, points) {

      // draws an "X"
      path.moveTo(QPointF(p0.x() - 10.0, p0.y() - 10.0));
      path.lineTo(QPointF(p0.x() + 10.0, p0.y() + 10.0));

      path.moveTo(QPointF(p0.x() - 10.0, p0.y() + 10.0));
      path.lineTo(QPointF(p0.x() + 10.0, p0.y() - 10.0));
    }

    

    drawPath(gc, path, isSnappingActive());
}

QPointF ConjugateAssistant::getEditorPosition() const
{
  if (sideHandles().isEmpty()) {
      return (*handles()[0] + *handles()[1]) * 0.5;
  } else {
    return *sideHandles()[0];
  }
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

QPointF ConjugateAssistant::stationPoint() {
  return sp;
}

void ConjugateAssistant::setStationPoint(QPointF p) {
  sp = p;
}

QPointF ConjugateAssistant::centerOfVision() {
  return cov;
}

void ConjugateAssistant::setCenterOfVision(QPointF c) {
  cov = c;
}

QLineF ConjugateAssistant::horizonLine() {
  return hl;
}

void ConjugateAssistant::setHorizonLine(QLineF l) {
  hl = l;
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

qreal ConjugateAssistant::spacing()
{
    return m_spacing;
}

void ConjugateAssistant::setSpacing(qreal value)
{
    if (value < 0.0) {
        value = 1.0;
    }

    m_spacing = value;
}

qreal ConjugateAssistant::tilt()
{
    return m_spacingTilt;
}

void ConjugateAssistant::setTilt(qreal value)
{
    m_spacingTilt = value;
}

qreal ConjugateAssistant::fade()
{
    return m_fade;
}

void ConjugateAssistant::setFade(qreal value)
{
    m_fade = value;
}

qreal ConjugateAssistant::count()
{
    return m_count;
}

void ConjugateAssistant::setCount(int value)
{
    m_count = value;
}

