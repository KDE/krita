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
#include <KIntNumInput>
#include <cmath>

#include <KoPathShape.h>
#include <KoCanvasBase.h>

#include "kis_cursor.h"
#include "kis_canvas2.h"
#include "kis_image.h"
#include "kis_selection_options.h"

inline qreal dist(const QPointF &p1, const QPointF &p2)
{
    QPointF a = p2-p1;
    qreal r=sqrt(a.x()*a.x() + a.y()*a.y());
//    qDebug()<<"dist from "<<p1<<"to"<<p2<<" is "<<r;
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
        : KisToolSelectBase(canvas, KisCursor::load("tool_magneticoutline_selection_cursor.png", 6, 6)), m_distance(25), m_localTool(canvas, this)
{
}

KisToolSelectMagnetic::~KisToolSelectMagnetic()
{
}




void KisToolSelectMagnetic::slotSetDistance(int distance)
{
    m_distance= distance;
}


QWidget* KisToolSelectMagnetic::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    m_optWidget->setWindowTitle(i18n("Magnetic Selection"));
    m_optWidget->disableAntiAliasSelectionOption();
    m_optWidget->disableSelectionModeOption();

    QHBoxLayout* fl = new QHBoxLayout();
    QLabel * lbl = new QLabel(i18n("Distance: "), m_optWidget);
    fl->addWidget(lbl);

    KIntNumInput * input = new KIntNumInput(m_optWidget);
    input->setRange(15, 55, 5);
    input->setValue(m_distance);
    fl->addWidget(input);
    connect(input, SIGNAL(valueChanged(int)), this, SLOT(slotSetDistance(int)));

    QVBoxLayout* l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    Q_ASSERT(l);
    l->insertLayout(1, fl);

    return m_optWidget;
}

KisToolSelectMagnetic::LocalTool::LocalTool(KoCanvasBase * canvas, KisToolSelectMagnetic* selectingTool)
        : KoCreatePathTool(canvas), m_selectingTool(selectingTool) {}

void KisToolSelectMagnetic::LocalTool::paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    QMatrix matrix;
    matrix.scale(kisCanvas->image()->xRes(), kisCanvas->image()->yRes());
    matrix.translate(pathShape.position().x(), pathShape.position().y());


    qreal zoomX, zoomY;
    kisCanvas->viewConverter()->zoom(&zoomX, &zoomY);

    //if the following fails, just comment it out. this tool won't be scaled correctly.
    Q_ASSERT(qFuzzyCompare(zoomX, zoomY));

    qreal width = m_selectingTool->m_distance;
    width *= zoomX/m_selectingTool->image()->xRes();


    paintOutline(&painter, m_selectingTool->pixelToView(matrix.map(pathShape.outline())), width);

    computeOutline(matrix.map(pathShape.outline()));
    painter.drawPolyline(m_selectingTool->pixelToView(m_debugPolyline));
    painter.setPen(QColor(255,255,255));
    painter.drawPoints(m_selectingTool->pixelToView(m_debugPolyline));

    painter.setPen(QColor(255,0,0));
    painter.drawPoints(m_selectingTool->pixelToView(m_debugScannedPoints));
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

void KisToolSelectMagnetic::LocalTool::computeOutline(const QPainterPath &pixelPath)
{
    const int accuracy = 2;

    QPolygonF polyline = pixelPath.toFillPolygon();
    if(polyline.count()>1) polyline.remove(polyline.count()-1);
    if(polyline.count()<2) return;

    //port from QPolygonF (==QVector) to QList
    QPolygonF points;
    points.append(polyline.at(0));

    //create points with the distance of accuracy
    for(int i=1; i<polyline.count(); i++) {
        qreal d = dist(points.last(), polyline.at(i));
        if(d<accuracy) {
            continue;
        }
        else {
            QVector2D fragment(polyline.at(i)-points.last());
            fragment.normalize();
            fragment*=accuracy;
            for(int j=0; j<((int) d)/accuracy; j++) {
                points.append(fragment.toPointF() + points.last());
            }
        }
    }

    m_debugScannedPoints = QPolygonF();
    for(int i=0; i<points.count(); i++) {
        QVector2D tangent = tangentAt(points, i);

        QVector2D startPos = QVector2D(points.at(i)) + rotateClockWise(tangent) * (m_selectingTool->m_distance/2);

        QVector2D currentPos = startPos;
        while((currentPos - startPos).length()<m_selectingTool->m_distance) {
            m_debugScannedPoints.append(currentPos.toPointF());
            //move one pixel
            currentPos+=rotateAntiClockWise(tangent)*1;
        }
    }

    m_debugPolyline = points;

}

void KisToolSelectMagnetic::LocalTool::addPathShape(KoPathShape* pathShape)
{
    delete pathShape;
//    KisNodeSP currentNode =
//        canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
//    if (!currentNode)
//        return;
//
//    KisImageWSP image = qobject_cast<KisLayer*>(currentNode->parent().data())->image();
//
//    m_shape->normalize();
//
//    KoPathShape *shape = m_shape;
//    shape->close();
//    m_shape = 0;
//
//    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
//    if (!kisCanvas)
//        return;
//
//    KisSelectionToolHelper helper(kisCanvas, currentNode, i18n("Path Selection"));
//
//    if (m_selectingTool->m_selectionMode == PIXEL_SELECTION) {
//
//        KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());
//
//        KisPainter painter(tmpSel);
//        painter.setBounds(m_selectingTool->currentImage()->bounds());
//        painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
//        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
//        painter.setStrokeStyle(KisPainter::StrokeStyleNone);
//        painter.setOpacity(OPACITY_OPAQUE);
//        painter.setCompositeOp(tmpSel->colorSpace()->compositeOp(COMPOSITE_OVER));
//
//        QMatrix matrix;
//        matrix.scale(image->xRes(), image->yRes());
//        matrix.translate(shape->position().x(), shape->position().y());
//        painter.fillPainterPath(matrix.map(shape->outline()));
//
//        QUndoCommand* cmd = helper.selectPixelSelection(tmpSel, m_selectingTool->m_selectAction);
//        canvas()->addCommand(cmd);
//
//        delete shape;
//    } else {
//        helper.addSelectionShape(shape);
//    }
}

#include "kis_tool_select_magnetic.moc"
