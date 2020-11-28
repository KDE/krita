/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2020 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "multigridpatterngenerator.h"

#include <QPoint>
#include <QPolygonF>
#include <QVector>
#include <QMap>
#include <QtMath>
#include <QDomDocument>

#include <kis_debug.h>

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <kis_fill_painter.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <generator/kis_generator_registry.h>
#include <kis_global.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_progress_update_helper.h>
#include <KoStopGradient.h>

#include "kis_wdg_multigrid_pattern.h"
#include "ui_wdgmultigridpatternoptions.h"


K_PLUGIN_FACTORY_WITH_JSON(KritaMultigridPatternGeneratorFactory, "kritamultigridpatterngenerator.json", registerPlugin<KritaMultigridPatternGenerator>();)

KritaMultigridPatternGenerator::KritaMultigridPatternGenerator(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisGeneratorRegistry::instance()->add(new KisMultigridPatternGenerator());
}

KritaMultigridPatternGenerator::~KritaMultigridPatternGenerator()
{
}

KisMultigridPatternGenerator::KisMultigridPatternGenerator() : KisGenerator(id(), KoID("basic"), i18n("&Multigrid Pattern..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
}

KisFilterConfigurationSP KisMultigridPatternGenerator::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);

    QLinearGradient gradient;
    gradient.setColorAt(0, Qt::green);
    gradient.setColorAt(1.0, Qt::blue);
    KoStopGradientSP grad = KoStopGradient::fromQGradient(&gradient);
    if (grad) {
        QDomDocument doc;
        QDomElement elt = doc.createElement("gradient");
        grad->toXML(doc, elt);
        doc.appendChild(elt);
        config->setProperty("gradientXML", doc.toString());
    }

    QVariant v;
    KoColor c;
    v.setValue(c);
    config->setProperty("lineColor", v);
    config->setProperty("divisions", 5);
    config->setProperty("lineWidth", 1);
    config->setProperty("dimensions", 5);
    config->setProperty("offset", .2);

    config->setProperty("colorRatio", 1.0);
    config->setProperty("colorIndex", 0.0);
    config->setProperty("colorIntersect", 0.0);

    config->setProperty("connectorColor", v);
    config->setProperty("connectorType", Connector::None);
    config->setProperty("connectorWidth", 1);
    return config;
}

KisConfigWidget * KisMultigridPatternGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisWdgMultigridPattern(parent);
}

void KisMultigridPatternGenerator::generate(KisProcessingInformation dstInfo,
                                 const QSize& size,
                                 const KisFilterConfigurationSP config,
                                 KoUpdater* progressUpdater) const
{
    KisPaintDeviceSP dst = dstInfo.paintDevice();

    Q_ASSERT(!dst.isNull());
    Q_ASSERT(config);

    if (config) {

        QLinearGradient gradient;
        gradient.setColorAt(0, Qt::green);
        gradient.setColorAt(1.0, Qt::blue);
        KoStopGradientSP grad = KoStopGradient::fromQGradient(&gradient);
        if (config->hasProperty("gradientXML")) {
            QDomDocument doc;
            doc.setContent(config->getString("gradientXML", ""));
            KoStopGradient gradient = KoStopGradient::fromXML(doc.firstChildElement());
            if (gradient.stops().size() > 0) {
                grad->setStops(gradient.stops());
            }
        }

        int divisions = config->getInt("divisions", 1);
        int dimensions = config->getInt("dimensions", 5);
        qreal offset = config->getFloat("offset", .2);
        QRectF bounds(QPoint(), size);

        KoColor lineColor = config->getColor("lineColor");
        lineColor.setOpacity(1.0);
        int lineWidth = config->getInt("lineWidth", 1);

        qreal colorRatio = config->getFloat("colorRatio", 1.0);
        qreal colorIndex = config->getFloat("colorIndex", 0.0);
        qreal colorIntersect = config->getFloat("colorIntersect", 0.0);

        qreal diameter = QLineF(bounds.topLeft(), bounds.bottomRight()).length();
        qreal scale = diameter/2/divisions;

        QList<KisMultiGridRhomb> rhombs = generateRhombs(dimensions, divisions, offset);

        KisProgressUpdateHelper progress(progressUpdater, 100, rhombs.size());


        KisPainter gc(dst);
        gc.setChannelFlags(config->channelFlags());
        gc.setOpacity(255);
        gc.setFillStyle(KisPainter::FillStyleBackgroundColor);
        gc.setStrokeStyle(KisPainter::StrokeStyleBrush);
        gc.setSelection(dstInfo.selection());

        gc.fill(bounds.left(), bounds.top(), bounds.right(), bounds.bottom(), lineColor);

        QTransform tf;
        tf.translate(bounds.center().x(), bounds.center().y());
        tf.scale(scale, scale);
        if (dimensions%2>0) {
            tf.rotateRadians((M_PI/dimensions)*0.5);
        }
        KoColor c = grad->stops()[0].color;
        for (int i= 0; i < rhombs.size(); i++){
            KisMultiGridRhomb rhomb = rhombs.at(i);
            QPolygonF shape = tf.map(rhomb.shape);

            QPointF center = shape.at(0)+shape.at(1)+shape.at(2)+shape.at(3);
            center.setX(center.x()/4.0);
            center.setY(center.y()/4.0);

            QTransform lineWidthTransform;

            qreal scaleForLineWidth = qMax(1-(qreal(lineWidth)/scale), 0.0);
            lineWidthTransform.scale(scaleForLineWidth, scaleForLineWidth);
            QPointF scaledCenter = lineWidthTransform.map(center);
            lineWidthTransform.reset();

            lineWidthTransform.translate(center.x()-scaledCenter.x(), center.y()-scaledCenter.y());
            lineWidthTransform.scale(scaleForLineWidth, scaleForLineWidth);

            shape = lineWidthTransform.map(shape);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
            if (shape.intersects(bounds) && shape.boundingRect().width()>0) {
#else
            if (!shape.intersected(bounds).isEmpty() && shape.boundingRect().width()>0) {
#endif
                QPainterPath p;
                p.addPolygon(lineWidthTransform.map(shape));

                qreal gradientPos = 1;

                qreal w1 = QLineF (shape.at(0), shape.at(2)).length();
                qreal w2 = QLineF (shape.at(1), shape.at(3)).length();
                qreal shapeRatio = qMin(w1, w2)/qMax(w1, w2);

                qreal intersectRatio = qreal(rhomb.line1)/qreal(dimensions);
                intersectRatio += qreal(rhomb.line2)/qreal(dimensions);
                intersectRatio *= 0.5;

                qreal indexRatio = 1-abs(qreal(rhomb.parallel1)/qreal(divisions/2.0));
                indexRatio *= 1-abs(qreal(rhomb.parallel2)/qreal(divisions/2.0));

                if (colorRatio>=0) {
                    gradientPos *= 1-(shapeRatio*colorRatio);
                } else {
                    gradientPos *= 1-((1-shapeRatio)*abs(colorRatio));
                }
                if (colorIntersect>=0) {
                    gradientPos *= 1-(intersectRatio*colorIntersect);
                } else {
                    gradientPos *= 1-((1-intersectRatio)*abs(colorIntersect));
                }
                if (colorIndex>=0) {
                    gradientPos *= 1-(indexRatio*colorIndex);
                } else {
                    gradientPos *= 1-((1-indexRatio)*abs(colorIndex));
                }

                grad->colorAt(c, gradientPos);
                gc.setBackgroundColor(c);

                gc.fillPainterPath(p, p.boundingRect().adjusted(-2, -2, 2, 2).toRect());

                int connectorType = config->getInt("connectorType", Connector::None);

                if (connectorType != Connector::None) {
                    gc.setBackgroundColor(config->getColor("connectorColor"));
                    qreal connectorWidth = qreal(config->getInt("connectorWidth", 1))*.5;
                    QPainterPath pConnect;
                    qreal lower = connectorWidth/scale;

                    if (connectorType == Connector::Cross) {
                        QPointF cl = QLineF(shape.at(0), shape.at(1)).pointAt(0.5-lower);
                        pConnect.moveTo(cl);
                        cl = QLineF(shape.at(0), shape.at(1)).pointAt(0.5+lower);
                        pConnect.lineTo(cl);
                        cl = QLineF(shape.at(2), shape.at(3)).pointAt(0.5-lower);
                        pConnect.lineTo(cl);
                        cl = QLineF(shape.at(2), shape.at(3)).pointAt(0.5+lower);
                        pConnect.lineTo(cl);
                        pConnect.closeSubpath();

                        cl = QLineF(shape.at(1), shape.at(2)).pointAt(0.5-lower);
                        pConnect.moveTo(cl);
                        cl = QLineF(shape.at(1), shape.at(2)).pointAt(0.5+lower);
                        pConnect.lineTo(cl);
                        cl = QLineF(shape.at(3), shape.at(0)).pointAt(0.5-lower);
                        pConnect.lineTo(cl);
                        cl = QLineF(shape.at(3), shape.at(0)).pointAt(0.5+lower);
                        pConnect.lineTo(cl);
                        pConnect.closeSubpath();

                    } else if (connectorType == Connector::CornerDot) {
                        QPointF cW(connectorWidth, connectorWidth);
                        
                        QRectF dot (shape.at(0)-cW, shape.at(0)+cW);
                        pConnect.addEllipse(dot);
                        dot = QRectF(shape.at(1)-cW, shape.at(1)+cW);
                        pConnect.addEllipse(dot);
                        dot = QRectF(shape.at(2)-cW, shape.at(2)+cW);
                        pConnect.addEllipse(dot);
                        dot = QRectF(shape.at(3)-cW, shape.at(3)+cW);
                        pConnect.addEllipse(dot);
                        pConnect = pConnect.intersected(p);
                        
                    } else if (connectorType == Connector::CenterDot) {
                        
                        QRectF dot (center-QPointF(connectorWidth, connectorWidth), center+QPointF(connectorWidth, connectorWidth));
                        pConnect.addEllipse(dot);
                        
                    } else {
                        for (int i=1; i<shape.size(); i++) {
                            QPainterPath pAngle;
                            QPointF curPoint = shape.at(i);
                            QLineF l1(curPoint, shape.at(i-1));
                            QPointF np;
                            if (i==4) {
                                np = shape.at(1);
                            } else {
                                np = shape.at(i+1);
                            }
                            QLineF l2(curPoint, np);
                            qreal angleDiff = abs(fmod(abs(l1.angle()-l2.angle())+180, 360)-180);

                            if (round(angleDiff) == 90) {
                                continue;
                            }
                            if (angleDiff > 90 && connectorType == Connector::Acute) {
                                continue;
                            }
                            if (angleDiff < 90 && connectorType == Connector::Obtuse) {
                                continue;
                            }

                            qreal length = (l1.length()*0.5)-connectorWidth;
                            QRectF sweep(curPoint-QPointF(length, length), curPoint+QPointF(length, length));
                            length = (l1.length()*0.5)+connectorWidth;
                            QRectF sweep2(curPoint-QPointF(length, length), curPoint+QPointF(length, length));

                            pAngle.moveTo(shape.at(i));
                            pAngle.addEllipse(sweep2);
                            pAngle.addEllipse(sweep);
                            pAngle = pAngle.intersected(p);
                            pAngle.closeSubpath();
                            pConnect.addPath(pAngle);
                        }
                        pConnect.closeSubpath();

                    }
                    pConnect.setFillRule(Qt::WindingFill);
                    gc.fillPainterPath(pConnect);

                }

                progress.step();
            }
        }

        gc.end();
    }
}

QList<KisMultiGridRhomb> KisMultigridPatternGenerator::generateRhombs(int lines, int divisions, qreal offset) const
{
    QList<KisMultiGridRhomb> rhombs;
    QList<QLineF> parallelLines;
    QList<qreal> angles;

    int halfLines = divisions;
    int totalLines = (halfLines*2) +1;

    //setup our imaginary lines...
    int dimensions = lines;
    for (int i = 0; i< dimensions; i++) {
        qreal angle = 2*(M_PI/lines)*i;
        angles.append(angle);
    }


    for (int i = 0; i <angles.size(); i++ ) {
        qreal angle1 = angles.at(i);
        QPointF p1(totalLines*cos(angle1), -totalLines*sin(angle1));
        QPointF p2 = -p1;


        for (int parallel1 = 0; parallel1 < totalLines; parallel1++) {
            int index1 = halfLines-parallel1;

            QPointF offset1((index1+offset)*sin(angle1), (index1+offset)*cos(angle1));
            QLineF l1(p1, p2);
            l1.translate(offset1);

            for (int k = i+1; k <angles.size(); k++ ) {
                qreal angle2 = angles.at(k);
                QPointF p3(totalLines*cos(angle2), -totalLines*sin(angle2));
                QPointF p4 = -p3;


                for (int parallel2 = 0; parallel2 < totalLines; parallel2++) {
                    int index2 = halfLines-parallel2;

                    QPointF offset2((index2+offset)*sin(angle2), (index2+offset)*cos(angle2));
                    QLineF l2(p3, p4);
                    l2.translate(offset2);


                    QPointF intersect;
#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
                    int intersection = l1.intersect(l2, &intersect);
#else
                    int intersection = l1.intersects(l2, &intersect);
#endif
                    if (intersection==QLineF::BoundedIntersection) {

                        QList<int> indices = getIndicesFromPoint(intersect, angles, offset );

                        QPolygonF shape;
                        indices[i] = index1+1;
                        indices[k] = index2+1;
                        shape << getVertice(indices, angles);
                        indices[i] = index1;
                        indices[k] = index2+1;
                        shape << getVertice(indices, angles);
                        indices[i] = index1;
                        indices[k] = index2;
                        shape << getVertice(indices, angles);
                        indices[i] = index1+1;
                        indices[k] = index2;
                        shape << getVertice(indices, angles);
                        indices[i] = index1+1;
                        indices[k] = index2+1;
                        shape << getVertice(indices, angles);

                        KisMultiGridRhomb rhomb;
                        rhomb.shape = shape;
                        rhomb.parallel1 = index1;
                        rhomb.parallel2 = index2;
                        rhomb.line1 = i;
                        rhomb.line2 = k;

                        rhombs.append(rhomb);
                    }

                }
            }
        }
    }


    return rhombs;
}

QList<int> KisMultigridPatternGenerator::getIndicesFromPoint(QPointF point, QList<qreal> angles, qreal offset) const
{
    QList<int> indices;

    for (int a=0; a< angles.size(); a++) {

        QPointF p = point;

        qreal index = p.x() * sin(angles.at(a)) + (p.y()) * cos(angles.at(a));
        indices.append(floor(index-offset+1));
    }
    return indices;
}

QPointF KisMultigridPatternGenerator::getVertice(QList<int> indices, QList<qreal> angles) const
{
    if (indices.isEmpty() || angles.isEmpty()) {
        qDebug() << "error";
        return QPointF();
    }
    qreal x = 0;
    qreal y = 0;

    for (int i=0; i<indices.size(); i++) {
        x += indices.at(i)*cos(angles.at(i));
        y += indices.at(i)*sin(angles.at(i));
    }

    return QPointF(x, y);
}

#include "multigridpatterngenerator.moc"
