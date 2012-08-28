/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_tool_select_magnetic.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QVector2D>
#include <QLinkedList>
#include <KIntNumInput>
#include <cmath>
#include <cstdlib>

#include <KoPathShape.h>
#include <KoCanvasBase.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>

#include "kis_cursor.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_selection_options.h"
#include "kis_selection_tool_helper.h"
#include "kis_random_accessor_ng.h"
#include "kis_pixel_selection.h"

#include "kis_tool_select_magnetic_option_widget.h"
#include "ui_kis_tool_select_magnetic_option_widget.h"

#include <iostream>
#include <QDebug>

const qreal pi = 3.1415926535897932384626433832795;

inline qreal dist(const QPointF &p1, const QPointF &p2)
{
    QPointF a = p2-p1;
    qreal r=sqrt(a.x()*a.x() + a.y()*a.y());
    return r;
}

inline QVector2D tangentAt(const QPolygonF &polyline, int i)
{
    if(i<0 || i>=polyline.count()) {
        //must not happen
        Q_ASSERT(false);
        return QVector2D(0,0);
    }
    int a=i-1;
    int b=i+1;
    if(a<0) a=0;
    if(b>=polyline.count()) b=i;
    if(a==b) {
        //must not happen as well
        Q_ASSERT(false);
        return QVector2D(1,0);
    }

    return QVector2D(polyline.at(b)-polyline.at(a)).normalized();
}

inline QVector2D rotateClockWise(const QVector2D &v)
{
    return QVector2D(v.y(), -v.x());
}

inline QVector2D rotateAntiClockWise(const QVector2D &v)
{
    return QVector2D(-v.y(), v.x());
}

KisToolSelectMagnetic::KisToolSelectMagnetic(KoCanvasBase * canvas)
    : KisToolSelectBase(canvas, KisCursor::load("tool_magneticoutline_selection_cursor.png", 6, 6), i18n("Magnetic Selection")),
        m_magneticOptions(0),
        m_localTool(canvas, this)
{
}

KisToolSelectMagnetic::~KisToolSelectMagnetic()
{
}

int KisToolSelectMagnetic::radius() const
{
    return m_magneticOptions->m_radius->value();
}

int KisToolSelectMagnetic::threshold() const
{
    return m_magneticOptions->m_threshold->value();
}

int KisToolSelectMagnetic::searchStartPoint() const
{
    if(m_magneticOptions->m_searchFromLeft->isChecked())
        return KisToolSelectMagneticOptionWidget::SearchFromLeft;
    else
        return KisToolSelectMagneticOptionWidget::SearchFromRight;
}

int KisToolSelectMagnetic::colorLimitation() const
{
    return m_magneticOptions->m_colorLimitation->currentIndex();
}

bool KisToolSelectMagnetic::limitToCurrentLayer() const
{
    return m_magneticOptions->m_limitToCurrentLayer->isChecked();
}


QWidget* KisToolSelectMagnetic::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    KisSelectionOptions *selectionWidget = selectionOptionWidget();

    selectionWidget->setWindowTitle();
    selectionWidget->disableAntiAliasSelectionOption();
    selectionWidget->disableSelectionModeOption();

    KisToolSelectMagneticOptionWidget *magneticOptionsWidget;
    magneticOptionsWidget = new KisToolSelectMagneticOptionWidget(selectionWidget);
    m_magneticOptions = magneticOptionsWidget->ui;

    QVBoxLayout* l = dynamic_cast<QVBoxLayout*>(selectionWidget->layout());
    Q_ASSERT(l);
    l->addWidget(magneticOptionsWidget);

    return selectionWidget;
}



KisToolSelectMagnetic::LocalTool::LocalTool(KoCanvasBase * canvas, KisToolSelectMagnetic* selectingTool)
        : KoCreatePathTool(canvas), m_selectingTool(selectingTool), m_accuracy(5) {}

KisToolSelectMagnetic::LocalTool::~LocalTool()
{
}

void KisToolSelectMagnetic::LocalTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KoCreatePathTool::activate(toolActivation, shapes);
    KisCanvas2* kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);
    KisImageWSP img = kisCanvas->image();

    m_colorSpace = img->colorSpace();
    Q_ASSERT(m_colorSpace);
    m_colorTransformation = m_colorSpace->createDesaturateAdjustment();
    Q_ASSERT(m_colorTransformation);
}

void KisToolSelectMagnetic::LocalTool::deactivate()
{
    KoCreatePathTool::deactivate();

    delete m_colorTransformation;
}

void KisToolSelectMagnetic::LocalTool::paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    QTransform matrix;
    matrix.scale(kisCanvas->image()->xRes(), kisCanvas->image()->yRes());
    matrix.translate(pathShape.position().x(), pathShape.position().y());


    qreal zoomX, zoomY;
    kisCanvas->viewConverter()->zoom(&zoomX, &zoomY);

    //if the following fails, just comment it out. this tool won't be scaled correctly.
    Q_ASSERT(qFuzzyCompare(zoomX, zoomY));

    qreal width = m_selectingTool->radius()*2;
    width *= zoomX/m_selectingTool->image()->xRes();


    paintOutline(&painter, m_selectingTool->pixelToView(matrix.map(pathShape.outline())), width);

    computeOutline(matrix.map(pathShape.outline()));
    painter.drawPolyline(m_selectingTool->pixelToView(m_debugPolyline));
    painter.setPen(QColor(255,255,255));
    painter.drawPoints(m_selectingTool->pixelToView(m_debugPolyline));

    painter.setPen(QPen(QColor(255,0,0), 1));
    painter.drawPoints(m_selectingTool->pixelToView(QPolygonF(m_debugScannedPoints)));

    painter.setPen(QPen(QColor(255,0,0), 5));
    cleanDetectedBorder();
    painter.drawPolyline(m_selectingTool->pixelToView(QPolygonF(m_detectedBorder)));
//    painter.drawPolyline(m_selectingTool->pixelToView(QPolygonF(m_tmpDetectedBorder)));
}

void KisToolSelectMagnetic::LocalTool::paintOutline(QPainter *painter, const QPainterPath &path, qreal width)
{
    painter->save();
    painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter->setPen(QPen(QColor(128, 255, 128), width));
    painter->drawPath(path);
    m_selectingTool->updateCanvasViewRect(path.controlPointRect().adjusted(-width, -width, width, width));
    painter->restore();
}

void KisToolSelectMagnetic::LocalTool::mouseReleaseEvent(KoPointerEvent *event)
{
    KoCreatePathTool::mouseReleaseEvent(event);
//    m_detectedBorder+=m_tmpDetectedBorder;
//    m_tmpDetectedBorder.clear();
//    if(m_detectedBorder.size()>1) {
//        m_tmpDetectedBorder.append(m_detectedBorder.last());
//        m_detectedBorder.remove(m_detectedBorder.size()-1);
//    }
//    qDebug()<<m_detectedBorder.size();
}

void KisToolSelectMagnetic::LocalTool::computeOutline(const QPainterPath &pixelPath)
{
    //the algorithm works as follows:
    // travers along the given path and from one side of the path to the other. and call the subfunction to compute the edge

    KisCanvas2* kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);
    KisImageWSP img = kisCanvas->image();

    KisPaintDeviceSP dev;
    if(m_selectingTool->limitToCurrentLayer()) {
        dev = m_selectingTool->currentNode()->paintDevice();
    }
    else {
        dev = img->projection();
    }
    KisRandomConstAccessorSP randomAccessorSP = dev->createRandomAccessorNG(0,0);


    QPolygonF polyline = pixelPath.toFillPolygon();
    if(polyline.count()>1) polyline.remove(polyline.count()-1);
    if(polyline.count()<2) return;

    //TODO: port from QPolygonF (==QVector) to QList
    QPolygonF points;
    points.append(polyline.at(0));

    //create points with the distance of accuracy
    for(int i=1; i<polyline.count(); i++) {
        qreal d = dist(points.last(), polyline.at(i));
        if(d<m_accuracy) {
            continue;
        }
        else {
            QVector2D fragment(polyline.at(i)-points.last());
            fragment.normalize();
            fragment*=m_accuracy;
            for(int j=0; j<((int) d)/m_accuracy; j++) {
                points.append(fragment.toPointF() + points.last());
            }
        }
    }


    m_debugScannedPoints = QPolygon();
//    m_tmpDetectedBorder.clear();
    m_detectedBorder.clear();
    if(m_selectingTool->searchStartPoint() == KisToolSelectMagneticOptionWidget::SearchFromLeft) {
        for(int i=1; i<points.count(); i++) {
            QVector2D tangent = tangentAt(points, i);
            QVector2D startPos = QVector2D(points.at(i)) + rotateClockWise(tangent) * (m_selectingTool->radius());
            computeEdge(startPos, rotateAntiClockWise(tangent), randomAccessor);
        }
    }
    else { //SearchFromRight
        for(int i=1; i<points.count(); i++) {
            QVector2D tangent = tangentAt(points, i);
            QVector2D startPos = QVector2D(points.at(i)) + rotateAntiClockWise(tangent) * (m_selectingTool->radius());
            computeEdge(startPos, rotateClockWise(tangent), randomAccessor);
        }
    }

    m_debugPolyline = points;

}

//void KisToolSelectMagnetic::LocalTool::computeEdge(const QVector2D &startPoint, const QVector2D &direction, KisRandomConstAccessorSP pixelAccessor)
//{
//// take the colour value of the starting side and match it against the colour on the current position
//// if the difference is beyond the threshold, there is a edge.
//    QVector2D currentPoint = startPoint;
//    KoColor transformedColor(m_colorSpace);
//
//    pixelAccessor->moveTo(currentPoint.x(), currentPoint.y());
//    m_colorTransformation->transform(pixelAccessor->rawData(), transformedColor.data(), 1);
//
//    int value = transformedColor.toQColor().value();
//
//    while((currentPoint - startPoint).length() < m_selectingTool->radius()*2) {
//        m_debugScannedPoints.append(currentPoint.toPoint());
//
//        pixelAccessor->moveTo(currentPoint.x(), currentPoint.y());
//        m_colorTransformation->transform(pixelAccessor->rawData(), transformedColor.data(), 1);
//
//        int currentValue = transformedColor.toQColor().value();
//        if(std::abs(value-currentValue)>m_selectingTool->threshold()) {
//            m_tmpDetectedBorder.append(currentPoint.toPoint());
//            return;
//        }
//        else {
//        }
//        //move to next pixel
//        currentPoint+=direction*1;
//    }
////    m_tmpDetectedBorder.append(currentPoint.toPoint());
//}


inline bool pointCandidateLessThan(const QPair<QVector2D, QHash<int, qreal> >& v1, const QPair<QVector2D, QHash<int, qreal> >& v2)
{
//    if((v1.second[0]/v1.second[1])+(0.1*v1.second[0]/v1.second[2]) > (v2.second[0]/v2.second[1])+(0.1*v2.second[0]/v2.second[2])) return true;
    if((v1.second[0]/v1.second[1]) + (0.1*v1.second[0]/(2+v1.second[2])) >
       (v2.second[0]/v2.second[1]) + (0.1*v2.second[0]/(2+v2.second[2])) )
        return true;
    else return false;
}

void KisToolSelectMagnetic::LocalTool::computeEdge(const QVector2D &startPoint, const QVector2D &direction, KisRandomConstAccessorSP pixelAccessor)
{
    QVector2D currentPoint = startPoint;
//    QVector2D lastPoint = m_tmpDetectedBorder.size()>0?QVector2D(m_tmpDetectedBorder.last()):QVector2D(-1, -1);
    QVector2D lastPoint = m_detectedBorder.size()>0?QVector2D(m_detectedBorder.last()):QVector2D(-1, -1);
    QVector2D centerPoint = startPoint+direction*m_selectingTool->radius();
    FilterMatrix horizontalFMatrix = getHorizontalFilterMatrix();
    FilterMatrix verticalFMatrix = getVerticalFilterMatrix();

    QList<QPair<QVector2D, QHash<int, qreal> > > pointCandidates; //pair<point, qreal[0]=edge strength(value derivate), qreal[1]=distance to last point, qreal[2]=distance to center of path>

    while((currentPoint - startPoint).length() < m_selectingTool->radius()*2) {
//        m_debugScannedPoints.append(currentPoint.toPoint());

        FilterMatrix pointValues = getMatrixForPoint(currentPoint, pixelAccessor);
//        std::cout<<pointValues;
        FilterMatrix horizontalPoints = pointValues.cwise() * horizontalFMatrix;
        FilterMatrix verticalPoints = pointValues.cwise() * verticalFMatrix;

        QHash<int, qreal> parameters;
        parameters[0] = fabs(horizontalPoints.sum()+verticalPoints.sum()); // edge strength
        parameters[1] = (currentPoint-lastPoint).length(); // dist to last point
        parameters[2] = (centerPoint-currentPoint).length(); //dist to center

        QPair<QVector2D, QHash<int, qreal> > pair;
        pair.first = currentPoint;
        pair.second = parameters;
        pointCandidates.append(pair);

//        qDebug()<<parameters[0]<<"/"<<parameters[1]<<"/"<<parameters[2];
        
        //move to next pixel
        currentPoint+=direction*1;
    }
    
    qSort(pointCandidates.begin(), pointCandidates.end(), pointCandidateLessThan);

//    if(qFuzzyCompare(sum, 0)) bestPoint = startPoint+direction*m_selectingTool->radius();

//    m_tmpDetectedBorder.append(pointCandidates.first().first.toPoint());
    m_detectedBorder.append(pointCandidates.first().first.toPoint());
}


FilterMatrix KisToolSelectMagnetic::LocalTool::getMatrixForPoint(const QVector2D &point, KisRandomConstAccessorSP pixelAccessor) const
{
    QVector2D currentPoint(point.x()-1, point.y()-1);
    KoColor transformedColor(m_colorSpace);

    double coeff[3][3];
    for(int i=0; i<3; i++) {
        for(int j=0; j<3; j++) {
            pixelAccessor->moveTo(currentPoint.x()+i, currentPoint.y()+j);
            m_colorTransformation->transform(pixelAccessor->rawData(), transformedColor.data(), 1);
            coeff[i][j]=transformedColor.toQColor().valueF();
        }
    }

    return (FilterMatrix() << coeff[0][0], coeff[0][1], coeff[0][2],
                              coeff[1][0], coeff[1][1], coeff[1][2],
                              coeff[2][0], coeff[2][1], coeff[2][2]).finished();
}

FilterMatrix KisToolSelectMagnetic::LocalTool::getHorizontalFilterMatrix() const
{
    return (Matrix3d() <<-1, 0, 1,
                         -2, 0, 2,
                         -1, 0, 1).finished();
}

FilterMatrix KisToolSelectMagnetic::LocalTool::getVerticalFilterMatrix() const
{
    return (Matrix3d() <<-1,-2,-1,
                          0, 0, 0,
                          1, 2, 1).finished();
}

void KisToolSelectMagnetic::LocalTool::cleanDetectedBorder()
{
    //load the points into a linked list, because removing on a vector is expensive
    QLinkedList<QPoint> detectedBorder;
    foreach(QPoint point, m_detectedBorder) {
        detectedBorder << point;
    }

    QLinkedList<QPoint>::iterator iter = detectedBorder.begin();

    while ((iter+2) != detectedBorder.end() && (iter+1) != detectedBorder.end() && iter != detectedBorder.end()) {
        QPoint current = *iter;
        QPoint next = *(iter+1);
        QPoint afterNext = *(iter+2);

        QVector2D relativeVectorToNext = QVector2D(next)-QVector2D(current);
        QVector2D relativeVectorToAfterNext = QVector2D(afterNext)-QVector2D(current);

        qreal distToNext = relativeVectorToNext.length();
        // FIXME: seems to be unused!
        //qreal distToAfterNext = relativeVectorToAfterNext.length();

        qreal angleToNext = std::atan2((qreal)relativeVectorToNext.y(), (qreal) relativeVectorToNext.x());
        qreal angleToAfterNext = std::atan2((qreal)relativeVectorToAfterNext.y(), (qreal) relativeVectorToAfterNext.x());

        if(fabs((angleToAfterNext-angleToNext)*distToNext)<1) {
            qDebug()<<"deleting: "<<(angleToAfterNext-angleToNext)*distToNext;
            iter = detectedBorder.erase(iter);
        }
        else {
            iter++;
        }
    }

    m_detectedBorder.clear();
    foreach(QPoint point, detectedBorder) {
        m_detectedBorder << point;
    }
}

void KisToolSelectMagnetic::LocalTool::addPathShape(KoPathShape* pathShape)
{
    KisNodeSP currentNode =
        canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
    if (!currentNode)
        return;

    KisImageWSP image = qobject_cast<KisLayer*>(currentNode->parent().data())->image();

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    KisSelectionToolHelper helper(kisCanvas, currentNode, i18n("Path Selection"));


    KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

    KisPainter painter(tmpSel);
    painter.setBounds(m_selectingTool->currentImage()->bounds());
    painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);
    painter.setOpacity(OPACITY_OPAQUE_U8);
    painter.setCompositeOp(tmpSel->colorSpace()->compositeOp(COMPOSITE_OVER));

    painter.paintPolygon(QPolygonF(m_detectedBorder));

    helper.selectPixelSelection(tmpSel, m_selectingTool->m_selectAction);

    m_detectedBorder.clear();
    m_tmpDetectedBorder.clear();
    delete pathShape;
}

#include "kis_tool_select_magnetic.moc"
