/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2020 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#include "multigridpatterngenerator.h"

#include <QPoint>
#include <QPolygonF>
#include <QVector>
#include <QMap>
#include <QtMath>

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

    QVariant v;
    KoColor c;
    c.fromQColor(QColor(Qt::green));
    v.setValue(c);
    config->setProperty("color1", v);
    c.fromQColor(QColor(Qt::blue));
    v.setValue(c);
    config->setProperty("color2", v);
    config->setProperty("divisions", 1);
    config->setProperty("dimensions", 5);
    config->setProperty("offset", .2);
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

    KoColor c1;
    KoColor c2;
    int divisions = 0;
    if (config) {
        c1 = config->getColor("color1");
        c1.setOpacity(1.0);
        c2 = config->getColor("color2");
        c2.setOpacity(1.0);
        divisions = config->getInt("divisions", 1);
        int dimensions = config->getInt("dimensions", 5);
        qreal offset = config->getFloat("offset", .2);
        QRectF bounds(QPoint(), size);

        //QLineF l(bounds.topLeft(), bounds.bottomRight());

        int lineWidth = bounds.width()/2/divisions;
        QList<KisMultiGridRhomb> rhombs = generateRhombs(dimensions, lineWidth, offset, bounds);

        KisProgressUpdateHelper progress(progressUpdater, 100, rhombs.size());


        KisPainter gc(dst);
        gc.setChannelFlags(config->channelFlags());
        gc.setOpacity(255);
        gc.setFillStyle(KisPainter::FillStyleBackgroundColor);
        gc.setStrokeStyle(KisPainter::StrokeStyleBrush);
        gc.setSelection(dstInfo.selection());

        KoStopGradient grad;
        auto gradientStops = grad.stops();
        gradientStops.append(KoGradientStop(0, c1));
        gradientStops.append(KoGradientStop(1, c2));
        grad.setStops(gradientStops);

        QTransform tf;
        tf.translate(bounds.center().x(), bounds.center().y());
        tf.scale(lineWidth/2, lineWidth/2);
        KoColor c = c1;
        for (int i= 0; i < rhombs.size(); i++){
            KisMultiGridRhomb rhomb = rhombs.at(i);
            QPolygonF shape = tf.map(rhomb.shape);
            if (shape.boundingRect().intersects(bounds)) {
                QPainterPath p;
                p.addPolygon(shape);

                qreal gradientPos = 1;

                qreal w1 = QLineF (shape.at(0), shape.at(2)).length();
                qreal w2 = QLineF (shape.at(1), shape.at(3)).length();
                qreal shapeRatio = qMin(w1, w2)/qMax(w1, w2);

                qreal intersectRatio = qreal(rhomb.line1)/qreal(dimensions);
                intersectRatio += qreal(rhomb.line2)/qreal(dimensions);
                intersectRatio *= 0.5;

                qreal divisionRatio = 1-abs(qreal(rhomb.parallel1)/qreal(divisions));
                divisionRatio *= 1-abs(qreal(rhomb.parallel2)/qreal(divisions));

                gradientPos *= shapeRatio;
                gradientPos *= intersectRatio;
                gradientPos *= divisionRatio;

                grad.colorAt(c, gradientPos);
                gc.setBackgroundColor(c);

                gc.fillPainterPath(p, p.boundingRect().adjusted(-2, -2, 2, 2).toRect());
                progress.step();
            }
        }

        gc.end();
    }
}

QList<KisMultiGridRhomb> KisMultigridPatternGenerator::generateRhombs(int lines, int lineWidth, qreal offset, QRectF area) const
{
    QList<KisMultiGridRhomb> rhombs;
    QList<QLineF> parallelLines;
    QList<qreal> angles;


    qreal radius = QLineF(area.topLeft(), area.topRight()).length();
    int halfLines = radius/lineWidth;
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
                    int intersection = l1.intersects(l2, &intersect);
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
