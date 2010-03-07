/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "enhancedpath/EnhancedPathShapeFactory.h"
#include "enhancedpath/EnhancedPathShape.h"

#include <KoLineBorder.h>
#include <KoProperties.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoColorBackground.h>

#include <klocale.h>

#include <QString>

#include <math.h>

EnhancedPathShapeFactory::EnhancedPathShapeFactory(QObject *parent)
    : KoShapeFactoryBase(parent, EnhancedPathShapeId, i18n("An enhanced path shape"))
{
    setToolTip(i18n("An enhanced path"));
    setIcon("enhancedpath");
    setOdfElementNames(KoXmlNS::draw, QStringList("custom-shape"));
    setLoadingPriority(1);

    addCross();
    addArrow();
    addCallout();
    addSmiley();
    addCircularArrow();
    addGearhead();
}

KoShape *EnhancedPathShapeFactory::createDefaultShape(KoResourceManager *) const
{
    EnhancedPathShape *shape = new EnhancedPathShape(QRectF(0, 0, 100, 100));
    shape->setBorder(new KoLineBorder(1.0));
    shape->setShapeId(KoPathShapeId);

    shape->addModifiers("35");

    shape->addFormula("Right", "width - $0");
    shape->addFormula("Bottom", "height - $0");
    shape->addFormula("Half", "min(0.5 * height, 0.5 * width)");

    shape->addCommand("M $0 0");
    shape->addCommand("L ?Right 0 ?Right $0 width $0 width ?Bottom ?Right ?Bottom");
    shape->addCommand("L ?Right height $0 height $0 ?Bottom 0 ?Bottom 0 $0 $0 $0");
    shape->addCommand("Z");

    ComplexType handle;
    handle["draw:handle-position"] = "$0 0";
    handle["draw:handle-range-x-minimum"] = "0";
    handle["draw:handle-range-x-maximum"] = "?Half";
    shape->addHandle(handle);
    shape->setSize(QSize(100, 100));

    return shape;
}

KoShape *EnhancedPathShapeFactory::createShape(const KoProperties *params, KoResourceManager *) const
{
    QRectF viewBox(0, 0, 100, 100);
    QVariant viewboxData;
    if (params->property("viewBox", viewboxData))
        viewBox = viewboxData.toRectF();

    EnhancedPathShape *shape = new EnhancedPathShape(viewBox);
    if (! shape)
        return 0;

    shape->setShapeId(KoPathShapeId);
    shape->setBorder(new KoLineBorder(1.0));
    shape->addModifiers(params->stringProperty("modifiers"));

    ListType handles = params->property("handles").toList();
    foreach (const QVariant &v, handles)
        shape->addHandle(v.toMap());

    ComplexType formulae = params->property("formulae").toMap();
    ComplexType::const_iterator formula = formulae.constBegin();
    ComplexType::const_iterator lastFormula = formulae.constEnd();
    for (; formula != lastFormula; ++formula)
        shape->addFormula(formula.key(), formula.value().toString());

    QStringList commands = params->property("commands").toStringList();
    foreach (const QString &cmd, commands)
        shape->addCommand(cmd);

    QVariant color;
    if (params->property("background", color))
        shape->setBackground(new KoColorBackground(color.value<QColor>()));
    QSizeF size = shape->size();
    if (size.width() > size.height())
        shape->setSize(QSizeF(100, 100 * size.height() / size.width()));
    else
        shape->setSize(QSizeF(100 * size.width() / size.height(), 100));

    return shape;
}

KoProperties* EnhancedPathShapeFactory::dataToProperties(
    const QString &modifiers, const QStringList &commands,
    const ListType &handles, const ComplexType & formulae) const
{
    KoProperties *props = new KoProperties();
    props->setProperty("modifiers", modifiers);
    props->setProperty("commands", commands);
    props->setProperty("handles", handles);
    props->setProperty("formulae", formulae);
    props->setProperty("background", QVariant::fromValue<QColor>(QColor(Qt::red)));

    return props;
}

void EnhancedPathShapeFactory::addCross()
{
    QString modifiers("35");

    QStringList commands;
    commands.append("M $0 0");
    commands.append("L ?Right 0 ?Right $0 width $0 width ?Bottom ?Right ?Bottom");
    commands.append("L ?Right height $0 height $0 ?Bottom 0 ?Bottom 0 $0 $0 $0");
    commands.append("Z");

    ListType handles;
    ComplexType handle;
    handle["draw:handle-position"] = "$0 0";
    handle["draw:handle-range-x-minimum"] = "0";
    handle["draw:handle-range-x-maximum"] = "?Half";
    handles.append(QVariant(handle));

    ComplexType formulae;
    formulae["Right"] = "width - $0";
    formulae["Bottom"] = "height - $0";
    formulae["Half"] = "min(0.5 * height, 0.5 * width)";

    KoShapeTemplate t;
    t.id = KoPathShapeId;
    t.templateId = "cross";
    t.name = i18n("Cross");
    t.family = "funny";
    t.toolTip = i18n("A cross");
    t.icon = "cross-shape";
    t.properties = dataToProperties(modifiers, commands, handles, formulae);

    addTemplate(t);
}

void EnhancedPathShapeFactory::addArrow()
{
    { // arrow right
        QString modifiers("60 35");

        QStringList commands;
        commands.append("M $0 $1");
        commands.append("L $0 0 width ?HalfHeight $0 height $0 ?LowerCorner 0 ?LowerCorner 0 $1");
        commands.append("Z");

        ListType handles;
        ComplexType handle;
        handle["draw:handle-position"] = "$0 $1";
        handle["draw:handle-range-x-minimum"] = "0";
        handle["draw:handle-range-x-maximum"] = "width";
        handle["draw:handle-range-y-minimum"] = "0";
        handle["draw:handle-range-y-maximum"] = "?HalfHeight";
        handles.append(QVariant(handle));

        ComplexType formulae;
        formulae["HalfHeight"] = "0.5 * height";
        formulae["LowerCorner"] = "height - $1";

        KoShapeTemplate t;
        t.id = KoPathShapeId;
        t.templateId = "arrow_right";
        t.name = i18n("Arrow");
        t.family = "arrow";
        t.toolTip = i18n("An arrow");
        t.icon = "arrow-right-koffice";
        t.properties = dataToProperties(modifiers, commands, handles, formulae);

        addTemplate(t);
    }

    { // arrow left
        QString modifiers("40 35");

        QStringList commands;
        commands.append("M $0 $1");
        commands.append("L $0 0 0 ?HalfHeight $0 height $0 ?LowerCorner width ?LowerCorner width $1");
        commands.append("Z");

        ListType handles;
        ComplexType handle;
        handle["draw:handle-position"] = "$0 $1";
        handle["draw:handle-range-x-minimum"] = "0";
        handle["draw:handle-range-x-maximum"] = "width";
        handle["draw:handle-range-y-minimum"] = "0";
        handle["draw:handle-range-y-maximum"] = "?HalfHeight";
        handles.append(QVariant(handle));

        ComplexType formulae;
        formulae["HalfHeight"] = "0.5 * height";
        formulae["LowerCorner"] = "height - $1";

        KoShapeTemplate t;
        t.id = KoPathShapeId;
        t.templateId = "arrow_left";
        t.name = i18n("Arrow");
        t.family = "arrow";
        t.toolTip = i18n("An arrow");
        t.icon = "arrow-left-koffice";
        t.properties = dataToProperties(modifiers, commands, handles, formulae);

        addTemplate(t);
    }

    { // arrow top
        QString modifiers("35 40");

        QStringList commands;
        commands.append("M $0 $1");
        commands.append("L 0 $1 ?HalfWidth 0 width $1 ?RightCorner $1 ?RightCorner height $0 height");
        commands.append("Z");

        ListType handles;
        ComplexType handle;
        handle["draw:handle-position"] = "$0 $1";
        handle["draw:handle-range-x-minimum"] = "0";
        handle["draw:handle-range-x-maximum"] = "?HalfWidth";
        handle["draw:handle-range-y-minimum"] = "0";
        handle["draw:handle-range-y-maximum"] = "height";
        handles.append(QVariant(handle));

        ComplexType formulae;
        formulae["HalfWidth"] = "0.5 * width";
        formulae["RightCorner"] = "width - $0";

        KoShapeTemplate t;
        t.id = KoPathShapeId;
        t.templateId = "arrow_top";
        t.name = i18n("Arrow");
        t.family = "arrow";
        t.toolTip = i18n("An arrow");
        t.icon = "arrow-up-koffice";
        t.properties = dataToProperties(modifiers, commands, handles, formulae);

        addTemplate(t);
    }

    { // arrow bottom
        QString modifiers("35 60");

        QStringList commands;
        commands.append("M $0 $1");
        commands.append("L 0 $1 ?HalfWidth height width $1 ?RightCorner $1 ?RightCorner 0 $0 0");
        commands.append("Z");

        ListType handles;
        ComplexType handle;
        handle["draw:handle-position"] = "$0 $1";
        handle["draw:handle-range-x-minimum"] = "0";
        handle["draw:handle-range-x-maximum"] = "?HalfWidth";
        handle["draw:handle-range-y-minimum"] = "0";
        handle["draw:handle-range-y-maximum"] = "height";
        handles.append(QVariant(handle));

        ComplexType formulae;
        formulae["HalfWidth"] = "0.5 * width";
        formulae["RightCorner"] = "width - $0";

        KoShapeTemplate t;
        t.id = KoPathShapeId;
        t.templateId = "arrow_bottom";
        t.name = i18n("Arrow");
        t.family = "arrow";
        t.toolTip = i18n("An arrow");
        t.icon = "arrow-down-koffice";
        t.properties = dataToProperties(modifiers, commands, handles, formulae);

        addTemplate(t);
    }
}

void EnhancedPathShapeFactory::addCallout()
{
    QString modifiers("4250 45000");

    QStringList commands;
    commands.append("M 3590 0");
    commands.append("X 0 3590");
    commands.append("L ?f2 ?f3 0 8970 0 12630 ?f4 ?f5 0 18010");
    commands.append("Y 3590 21600");
    commands.append("L ?f6 ?f7 8970 21600 12630 21600 ?f8 ?f9 18010 21600");
    commands.append("X 21600 18010");
    commands.append("L ?f10 ?f11 21600 12630 21600 8970 ?f12 ?f13 21600 3590");
    commands.append("Y 18010 0");
    commands.append("L ?f14 ?f15 12630 0 8970 0 ?f16 ?f17");
    commands.append("Z");
    commands.append("N");

    ComplexType formulae;
    formulae["f0"] = "$0 -10800";
    formulae["f1"] = "$1 -10800";
    formulae["f2"] = "if(?f18 ,$0 ,0)";
    formulae["f3"] = "if(?f18 ,$1 ,6280)";
    formulae["f4"] = "if(?f23 ,$0 ,0)";
    formulae["f5"] = "if(?f23 ,$1 ,15320)";
    formulae["f6"] = "if(?f26 ,$0 ,6280)";
    formulae["f7"] = "if(?f26 ,$1 ,21600)";
    formulae["f8"] = "if(?f29 ,$0 ,15320)";
    formulae["f9"] = "if(?f29 ,$1 ,21600)";
    formulae["f10"] = "if(?f32 ,$0 ,21600)";
    formulae["f11"] = "if(?f32 ,$1 ,15320)";
    formulae["f12"] = "if(?f34 ,$0 ,21600)";
    formulae["f13"] = "if(?f34 ,$1 ,6280)";
    formulae["f14"] = "if(?f36 ,$0 ,15320)";
    formulae["f15"] = "if(?f36 ,$1 ,0)";
    formulae["f16"] = "if(?f38 ,$0 ,6280)";
    formulae["f17"] = "if(?f38 ,$1 ,0)";
    formulae["f18"] = "if($0 ,-1,?f19)";
    formulae["f19"] = "if(?f1 ,-1,?f22)";
    formulae["f20"] = "abs(?f0)";
    formulae["f21"] = "abs(?f1)";
    formulae["f22"] = "?f20 -?f21";
    formulae["f23"] = "if($0 ,-1,?f24)";
    formulae["f24"] = "if(?f1 ,?f22 ,-1)";
    formulae["f25"] = "$1 -21600";
    formulae["f26"] = "if(?f25 ,?f27 ,-1)";
    formulae["f27"] = "if(?f0 ,-1,?f28)";
    formulae["f28"] = "?f21 -?f20";
    formulae["f29"] = "if(?f25 ,?f30 ,-1)";
    formulae["f30"] = "if(?f0 ,?f28 ,-1)";
    formulae["f31"] = "$0 -21600";
    formulae["f32"] = "if(?f31 ,?f33 ,-1)";
    formulae["f33"] = "if(?f1 ,?f22 ,-1)";
    formulae["f34"] = "if(?f31 ,?f35 ,-1)";
    formulae["f35"] = "if(?f1 ,-1,?f22)";
    formulae["f36"] = "if($1 ,-1,?f37)";
    formulae["f37"] = "if(?f0 ,?f28 ,-1)";
    formulae["f38"] = "if($1 ,-1,?f39)";
    formulae["f39"] = "if(?f0 ,-1,?f28)";
    formulae["f40"] = "$0";
    formulae["f41"] = "$1";

    ListType handles;
    ComplexType handle;
    handle["draw:handle-position"] = "$0 $1";
    handles.append(QVariant(handle));

    KoShapeTemplate t;
    t.id = KoPathShapeId;
    t.templateId = "callout";
    t.name = i18n("Callout");
    t.family = "funny";
    t.toolTip = i18n("A callout");
    t.icon = "callout-shape";
    t.properties = dataToProperties(modifiers, commands, handles, formulae);
    t.properties->setProperty("viewBox", QRectF(0, 0, 21600, 21600));

    addTemplate(t);
}

void EnhancedPathShapeFactory::addSmiley()
{
    QString modifiers("17520");

    QStringList commands;
    commands.append("U 10800 10800 10800 10800 0 23592960");
    commands.append("Z");
    commands.append("N");
    commands.append("U 7305 7515 1165 1165 0 23592960");
    commands.append("Z");
    commands.append("N");
    commands.append("U 14295 7515 1165 1165 0 23592960");
    commands.append("Z");
    commands.append("N");
    commands.append("M 4870 ?f1");
    commands.append("C 8680 ?f2 12920 ?f2 16730 ?f1");
    commands.append("Z");
    commands.append("F");
    commands.append("N");

    ComplexType formulae;
    formulae["f0"] = "$0 -15510";
    formulae["f1"] = "17520-?f0";
    formulae["f2"] = "15510+?f0";

    ListType handles;
    ComplexType handle;
    handle["draw:handle-position"] = "10800 $0";
    handle["draw:handle-range-y-minimum"] = "15510";
    handle["draw:handle-range-y-maximum"] = "17520";
    handles.append(QVariant(handle));

    KoShapeTemplate t;
    t.id = KoPathShapeId;
    t.templateId = "smiley";
    t.name = i18n("Smiley");
    t.family = "funny";
    t.toolTip = i18n("Smiley");
    t.icon = "smiley-shape";
    t.properties = dataToProperties(modifiers, commands, handles, formulae);
    t.properties->setProperty("viewBox", QRectF(0, 0, 21600, 21600));

    addTemplate(t);
}

void EnhancedPathShapeFactory::addCircularArrow()
{
    QString modifiers("180 0 5500");

    QStringList commands;
    commands.append("B ?f3 ?f3 ?f20 ?f20 ?f19 ?f18 ?f17 ?f16");
    commands.append("W 0 0 21600 21600 ?f9 ?f8 ?f11 ?f10");
    commands.append("L ?f24 ?f23 ?f36 ?f35 ?f29 ?f28");
    commands.append("Z");
    commands.append("N");

    ComplexType formulae;

    formulae["f0"] = "$0";
    formulae["f1"] = "$1";
    formulae["f2"] = "$2";
    formulae["f3"] = "10800+$2";
    formulae["f4"] = "10800*sin($0 *(pi/180))";
    formulae["f5"] = "10800*cos($0 *(pi/180))";
    formulae["f6"] = "10800*sin($1 *(pi/180))";
    formulae["f7"] = "10800*cos($1 *(pi/180))";
    formulae["f8"] = "?f4 +10800";
    formulae["f9"] = "?f5 +10800";
    formulae["f10"] = "?f6 +10800";
    formulae["f11"] = "?f7 +10800";
    formulae["f12"] = "?f3 *sin($0 *(pi/180))";
    formulae["f13"] = "?f3 *cos($0 *(pi/180))";
    formulae["f14"] = "?f3 *sin($1 *(pi/180))";
    formulae["f15"] = "?f3 *cos($1 *(pi/180))";
    formulae["f16"] = "?f12 +10800";
    formulae["f17"] = "?f13 +10800";
    formulae["f18"] = "?f14 +10800";
    formulae["f19"] = "?f15 +10800";
    formulae["f20"] = "21600-?f3";
    formulae["f21"] = "13500*sin($1 *(pi/180))";
    formulae["f22"] = "13500*cos($1 *(pi/180))";
    formulae["f23"] = "?f21 +10800";
    formulae["f24"] = "?f22 +10800";
    formulae["f25"] = "$2 -2700";
    formulae["f26"] = "?f25 *sin($1 *(pi/180))";
    formulae["f27"] = "?f25 *cos($1 *(pi/180))";
    formulae["f28"] = "?f26 +10800";
    formulae["f29"] = "?f27 +10800";
    formulae["f30"] = "($1+45)*pi/180";
    formulae["f31"] = "sqrt(((?f29-?f24)*(?f29-?f24))+((?f28-?f23)*(?f28-?f23)))";
    formulae["f32"] = "sqrt(2)/2*?f31";
    formulae["f33"] = "?f32*sin(?f30)";
    formulae["f34"] = "?f32*cos(?f30)";
    formulae["f35"] = "?f28+?f33";
    formulae["f36"] = "?f29+?f34";

    ListType handles;
    ComplexType handle;
    handle["draw:handle-position"] = "$0 10800";
    handle["draw:handle-polar"] = "10800 10800";
    handle["draw:handle-radius-range-minimum"] = "10800";
    handle["draw:handle-radius-range-maximum"] = "10800";
    handles.append(QVariant(handle));

    handle.clear();
    handle["draw:handle-position"] = "$1 $2";
    handle["draw:handle-polar"] = "10800 10800";
    handle["draw:handle-radius-range-minimum"] = "0";
    handle["draw:handle-radius-range-maximum"] = "10800";
    handles.append(QVariant(handle));

    KoShapeTemplate t;
    t.id = KoPathShapeId;
    t.templateId = "circulararrow";
    t.name = i18n("Circular Arrow");
    t.family = "arrow";
    t.toolTip = i18n("A circular-arrow");
    t.icon = "circular-arrow-shape";
    t.properties = dataToProperties(modifiers, commands, handles, formulae);
    t.properties->setProperty("viewBox", QRectF(0, 0, 21600, 21600));
    addTemplate(t);
}

void EnhancedPathShapeFactory::addGearhead()
{
    QStringList commands;
    commands.append("M 20 70");
    commands.append("L 20 100 30 100 30 50 30 70 40 70 40 40 0 40 0 70 10 70 10 50 10 100 20 100");
    commands.append("Z");
    commands.append("N");

    uint toothCount = 10;
    qreal toothAngle = 360.0 / qreal(toothCount);
    //kDebug() <<"toothAngle =" << toothAngle;
    qreal outerRadius = 0.5 * 25.0;
    qreal innerRadius = 0.5 * 17.0;
    QPointF center(20, 25);
    qreal radian = (270.0 - 0.35 * toothAngle) * M_PI / 180.0;
    commands.append(QString("M %1 %2").arg(center.x() + innerRadius*cos(radian)).arg(center.y() + innerRadius*sin(radian)));
    QString cmd("L");
    for (uint i = 0; i < toothCount; ++i) {
        radian += 0.15 * toothAngle * M_PI / 180.0;
        cmd += QString(" %1 %2").arg(center.x() + outerRadius*cos(radian)).arg(center.y() + outerRadius*sin(radian));
        radian += 0.35 * toothAngle * M_PI / 180.0;
        cmd += QString(" %1 %2").arg(center.x() + outerRadius*cos(radian)).arg(center.y() + outerRadius*sin(radian));
        radian += 0.15 * toothAngle * M_PI / 180.0;
        cmd += QString(" %1 %2").arg(center.x() + innerRadius*cos(radian)).arg(center.y() + innerRadius*sin(radian));
        radian += 0.35 * toothAngle * M_PI / 180.0;
        cmd += QString(" %1 %2").arg(center.x() + innerRadius*cos(radian)).arg(center.y() + innerRadius*sin(radian));
    }
    //kDebug() <<"gear command =" << cmd;
    commands.append(cmd);
    commands.append("Z");
    commands.append("N");

    KoShapeTemplate t;
    t.id = KoPathShapeId;
    t.templateId = "gearhead";
    t.name = i18n("Gearhead");
    t.family = "funny";
    t.toolTip = i18n("A gearhead");
    t.icon = "gearhead-shape";
    t.properties = dataToProperties(QString(), commands, ListType(), ComplexType());
    t.properties->setProperty("background", QVariant::fromValue<QColor>(QColor(Qt::blue)));
    t.properties->setProperty("viewBox", QRectF(0, 0, 40, 90));
    addTemplate(t);
}

bool EnhancedPathShapeFactory::supports(const KoXmlElement & e) const
{
    return (e.localName() == "custom-shape" && e.namespaceURI() == KoXmlNS::draw);
}

#include <EnhancedPathShapeFactory.moc>
