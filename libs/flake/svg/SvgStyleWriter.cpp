/* This file is part of the KDE project
   Copyright (C) 2002 Lars Siebold <khandha5@gmx.net>
   Copyright (C) 2002-2003,2005 Rob Buis <buis@kde.org>
   Copyright (C) 2002,2005-2006 David Faure <faure@kde.org>
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2004 Nicolas Goutte <nicolasg@snafu.de>
   Copyright (C) 2005 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2005 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2005,2007-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006 Gábor Lehel <illissius@gmail.com>
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2006 Christian Mueller <cmueller@gmx.de>
   Copyright (C) 2006 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "SvgStyleWriter.h"
#include "SvgSavingContext.h"
#include "SvgUtil.h"

#include <KoShape.h>
#include <KoPathShape.h>
#include <KoFilterEffect.h>
#include <KoFilterEffectStack.h>
#include <KoColorBackground.h>
#include <KoGradientBackground.h>
#include <KoPatternBackground.h>
#include <KoVectorPatternBackground.h>
#include <KoShapeStroke.h>
#include <KoClipPath.h>
#include <KoClipMask.h>
#include <KoMarker.h>
#include <KoXmlWriter.h>

#include <QBuffer>
#include <QGradient>
#include <QLinearGradient>
#include <QRadialGradient>
#include <KisMimeDatabase.h>
#include "kis_dom_utils.h"
#include "kis_algebra_2d.h"
#include <SvgWriter.h>
#include <KoFlakeCoordinateSystem.h>


void SvgStyleWriter::saveSvgStyle(KoShape *shape, SvgSavingContext &context)
{
    saveSvgBasicStyle(shape, context);

    saveSvgFill(shape, context);
    saveSvgStroke(shape, context);

    saveSvgEffects(shape, context);
    saveSvgClipping(shape, context);
    saveSvgMasking(shape, context);
    saveSvgMarkers(shape, context);
}

void SvgStyleWriter::saveSvgBasicStyle(KoShape *shape, SvgSavingContext &context)
{
    if (!shape->isVisible(false)) {
        context.shapeWriter().addAttribute("display", "none");
    } else if (shape->transparency() > 0.0) {
        context.shapeWriter().addAttribute("opacity", 1.0 - shape->transparency());
    }
}

void SvgStyleWriter::saveSvgFill(KoShape *shape, SvgSavingContext &context)
{
    if (! shape->background()) {
        context.shapeWriter().addAttribute("fill", "none");
    }

    QBrush fill(Qt::NoBrush);
    QSharedPointer<KoColorBackground>  cbg = qSharedPointerDynamicCast<KoColorBackground>(shape->background());
    if (cbg) {
        context.shapeWriter().addAttribute("fill", cbg->color().name());
        if (cbg->color().alphaF() < 1.0)
            context.shapeWriter().addAttribute("fill-opacity", cbg->color().alphaF());
    }
    QSharedPointer<KoGradientBackground>  gbg = qSharedPointerDynamicCast<KoGradientBackground>(shape->background());
    if (gbg) {
        QString gradientId = saveSvgGradient(gbg->gradient(), gbg->transform(), context);
        context.shapeWriter().addAttribute("fill", "url(#" + gradientId + ")");
    }
    QSharedPointer<KoPatternBackground>  pbg = qSharedPointerDynamicCast<KoPatternBackground>(shape->background());
    if (pbg) {
        const QString patternId = saveSvgPattern(pbg, shape, context);
        context.shapeWriter().addAttribute("fill", "url(#" + patternId + ")");
    }
    QSharedPointer<KoVectorPatternBackground>  vpbg = qSharedPointerDynamicCast<KoVectorPatternBackground>(shape->background());
    if (vpbg) {
        const QString patternId = saveSvgVectorPattern(vpbg, shape, context);
        context.shapeWriter().addAttribute("fill", "url(#" + patternId + ")");
    }

    KoPathShape * path = dynamic_cast<KoPathShape*>(shape);
    if (path && shape->background()) {
        // non-zero is default, so only write fillrule if evenodd is set
        if (path->fillRule() == Qt::OddEvenFill)
            context.shapeWriter().addAttribute("fill-rule", "evenodd");
    }
}

void SvgStyleWriter::saveSvgStroke(KoShape *shape, SvgSavingContext &context)
{
    const QSharedPointer<KoShapeStroke> lineBorder = qSharedPointerDynamicCast<KoShapeStroke>(shape->stroke());

    if (! lineBorder)
        return;

    QString strokeStr("none");
    if (lineBorder->lineBrush().gradient()) {
        QString gradientId = saveSvgGradient(lineBorder->lineBrush().gradient(), lineBorder->lineBrush().transform(), context);
        strokeStr = "url(#" + gradientId + ")";
    } else {
        strokeStr = lineBorder->color().name();
    }
    if (!strokeStr.isEmpty())
        context.shapeWriter().addAttribute("stroke", strokeStr);

    if (lineBorder->color().alphaF() < 1.0)
        context.shapeWriter().addAttribute("stroke-opacity", lineBorder->color().alphaF());
    context.shapeWriter().addAttribute("stroke-width", SvgUtil::toUserSpace(lineBorder->lineWidth()));

    if (lineBorder->capStyle() == Qt::FlatCap)
        context.shapeWriter().addAttribute("stroke-linecap", "butt");
    else if (lineBorder->capStyle() == Qt::RoundCap)
        context.shapeWriter().addAttribute("stroke-linecap", "round");
    else if (lineBorder->capStyle() == Qt::SquareCap)
        context.shapeWriter().addAttribute("stroke-linecap", "square");

    if (lineBorder->joinStyle() == Qt::MiterJoin) {
        context.shapeWriter().addAttribute("stroke-linejoin", "miter");
        context.shapeWriter().addAttribute("stroke-miterlimit", lineBorder->miterLimit());
    } else if (lineBorder->joinStyle() == Qt::RoundJoin)
        context.shapeWriter().addAttribute("stroke-linejoin", "round");
    else if (lineBorder->joinStyle() == Qt::BevelJoin)
        context.shapeWriter().addAttribute("stroke-linejoin", "bevel");

    // dash
    if (lineBorder->lineStyle() > Qt::SolidLine) {
        qreal dashFactor = lineBorder->lineWidth();

        if (lineBorder->dashOffset() != 0)
            context.shapeWriter().addAttribute("stroke-dashoffset", dashFactor * lineBorder->dashOffset());

        QString dashStr;
        const QVector<qreal> dashes = lineBorder->lineDashes();
        int dashCount = dashes.size();
        for (int i = 0; i < dashCount; ++i) {
            if (i > 0)
                dashStr += ",";
            dashStr += QString("%1").arg(KisDomUtils::toString(dashes[i] * dashFactor));
        }
        context.shapeWriter().addAttribute("stroke-dasharray", dashStr);
    }
}

void SvgStyleWriter::saveSvgEffects(KoShape *shape, SvgSavingContext &context)
{
    KoFilterEffectStack * filterStack = shape->filterEffectStack();
    if (!filterStack)
        return;

    QList<KoFilterEffect*> filterEffects = filterStack->filterEffects();
    if (!filterEffects.count())
        return;

    const QString uid = context.createUID("filter");

    filterStack->save(context.styleWriter(), uid);

    context.shapeWriter().addAttribute("filter", "url(#" + uid + ")");
}

void embedShapes(const QList<KoShape*> &shapes, KoXmlWriter &outWriter)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    {
        SvgWriter shapesWriter(shapes);
        shapesWriter.saveDetached(buffer);
    }
    buffer.close();
    outWriter.addCompleteElement(&buffer);
}


void SvgStyleWriter::saveSvgClipping(KoShape *shape, SvgSavingContext &context)
{
    KoClipPath *clipPath = shape->clipPath();
    if (!clipPath)
        return;

    const QString uid = context.createUID("clippath");

    context.styleWriter().startElement("clipPath");
    context.styleWriter().addAttribute("id", uid);
    context.styleWriter().addAttribute("clipPathUnits", KoFlake::coordinateToString(clipPath->coordinates()));

    embedShapes(clipPath->clipShapes(), context.styleWriter());

    context.styleWriter().endElement(); // clipPath

    context.shapeWriter().addAttribute("clip-path", "url(#" + uid + ")");
    if (clipPath->clipRule() != Qt::WindingFill)
        context.shapeWriter().addAttribute("clip-rule", "evenodd");
}

void SvgStyleWriter::saveSvgMasking(KoShape *shape, SvgSavingContext &context)
{
    KoClipMask*clipMask = shape->clipMask();
    if (!clipMask)
        return;

    const QString uid = context.createUID("clipmask");

    context.styleWriter().startElement("mask");
    context.styleWriter().addAttribute("id", uid);
    context.styleWriter().addAttribute("maskUnits", KoFlake::coordinateToString(clipMask->coordinates()));
    context.styleWriter().addAttribute("maskContentUnits", KoFlake::coordinateToString(clipMask->contentCoordinates()));

    const QRectF rect = clipMask->maskRect();

    context.styleWriter().addAttribute("x", rect.x());
    context.styleWriter().addAttribute("y", rect.y());
    context.styleWriter().addAttribute("width", rect.width());
    context.styleWriter().addAttribute("height", rect.height());

    embedShapes(clipMask->shapes(), context.styleWriter());

    context.styleWriter().endElement(); // clipMask

    context.shapeWriter().addAttribute("mask", "url(#" + uid + ")");
}

namespace {
void writeMarkerStyle(KoXmlWriter &styleWriter, const KoMarker *marker, const QString &assignedId) {

    styleWriter.startElement("marker");
    styleWriter.addAttribute("id", assignedId);
    styleWriter.addAttribute("markerUnits", KoMarker::coordinateSystemToString(marker->coordinateSystem()));

    const QPointF refPoint = marker->referencePoint();
    styleWriter.addAttribute("refX", refPoint.x());
    styleWriter.addAttribute("refY", refPoint.y());

    const QSizeF refSize = marker->referenceSize();
    styleWriter.addAttribute("markerWidth", refSize.width());
    styleWriter.addAttribute("markerHeight", refSize.height());


    if (marker->hasAutoOtientation()) {
        styleWriter.addAttribute("orient", "auto");
    } else {
        // no suffix means 'degrees'
        styleWriter.addAttribute("orient", kisRadiansToDegrees(marker->explicitOrientation()));
    }

    embedShapes(marker->shapes(), styleWriter);

    styleWriter.endElement(); // marker
}

void tryEmbedMarker(const KoPathShape *pathShape,
                    const QString &markerTag,
                    KoFlake::MarkerPosition markerPosition,
                    SvgSavingContext &context)
{
    KoMarker *marker = pathShape->marker(markerPosition);

    if (marker) {
        const QString uid = context.createUID("lineMarker");
        writeMarkerStyle(context.styleWriter(), marker, uid);
        context.shapeWriter().addAttribute(markerTag.toLatin1().data(), "url(#" + uid + ")");
    }
}

}

void SvgStyleWriter::saveSvgMarkers(KoShape *shape, SvgSavingContext &context)
{
    KoPathShape *pathShape = dynamic_cast<KoPathShape*>(shape);
    if (!pathShape || !pathShape->hasMarkers()) return;


    tryEmbedMarker(pathShape, "marker-start", KoFlake::StartMarker, context);
    tryEmbedMarker(pathShape, "marker-mid", KoFlake::MidMarker, context);
    tryEmbedMarker(pathShape, "marker-end", KoFlake::EndMarker, context);

    if (pathShape->autoFillMarkers()) {
        context.shapeWriter().addAttribute("krita:marker-fill-method", "auto");
    }
}

void SvgStyleWriter::saveSvgColorStops(const QGradientStops &colorStops, SvgSavingContext &context)
{
    Q_FOREACH (const QGradientStop &stop, colorStops) {
        context.styleWriter().startElement("stop");
        context.styleWriter().addAttribute("stop-color", stop.second.name());
        context.styleWriter().addAttribute("offset", stop.first);
        context.styleWriter().addAttribute("stop-opacity", stop.second.alphaF());
        context.styleWriter().endElement();
    }
}

inline QString convertGradientMode(QGradient::CoordinateMode mode) {
    KIS_ASSERT_RECOVER_NOOP(mode != QGradient::StretchToDeviceMode);

    return
        mode == QGradient::ObjectBoundingMode ?
        "objectBoundingBox" :
        "userSpaceOnUse";

}

QString SvgStyleWriter::saveSvgGradient(const QGradient *gradient, const QTransform &gradientTransform, SvgSavingContext &context)
{
    if (! gradient)
        return QString();

    const QString spreadMethod[3] = {
        QString("pad"),
        QString("reflect"),
        QString("repeat")
    };

    const QString uid = context.createUID("gradient");

    if (gradient->type() == QGradient::LinearGradient) {
        const QLinearGradient * g = static_cast<const QLinearGradient*>(gradient);
        context.styleWriter().startElement("linearGradient");
        context.styleWriter().addAttribute("id", uid);
        SvgUtil::writeTransformAttributeLazy("gradientTransform", gradientTransform, context.styleWriter());
        context.styleWriter().addAttribute("gradientUnits", convertGradientMode(g->coordinateMode()));
        context.styleWriter().addAttribute("x1", g->start().x());
        context.styleWriter().addAttribute("y1", g->start().y());
        context.styleWriter().addAttribute("x2", g->finalStop().x());
        context.styleWriter().addAttribute("y2", g->finalStop().y());
        context.styleWriter().addAttribute("spreadMethod", spreadMethod[g->spread()]);
        // color stops
        saveSvgColorStops(gradient->stops(), context);
        context.styleWriter().endElement();
    } else if (gradient->type() == QGradient::RadialGradient) {
        const QRadialGradient * g = static_cast<const QRadialGradient*>(gradient);
        context.styleWriter().startElement("radialGradient");
        context.styleWriter().addAttribute("id", uid);
        SvgUtil::writeTransformAttributeLazy("gradientTransform", gradientTransform, context.styleWriter());
        context.styleWriter().addAttribute("gradientUnits", convertGradientMode(g->coordinateMode()));
        context.styleWriter().addAttribute("cx", g->center().x());
        context.styleWriter().addAttribute("cy", g->center().y());
        context.styleWriter().addAttribute("fx", g->focalPoint().x());
        context.styleWriter().addAttribute("fy", g->focalPoint().y());
        context.styleWriter().addAttribute("r", g->radius());
        context.styleWriter().addAttribute("spreadMethod", spreadMethod[g->spread()]);
        // color stops
        saveSvgColorStops(gradient->stops(), context);
        context.styleWriter().endElement();
    } else if (gradient->type() == QGradient::ConicalGradient) {
        //const QConicalGradient * g = static_cast<const QConicalGradient*>( gradient );
        // fake conical grad as radial.
        // fugly but better than data loss.
        /*
        printIndentation( m_defs, m_indent2 );
        *m_defs << "<radialGradient id=\"" << uid << "\" ";
        *m_defs << "gradientUnits=\"userSpaceOnUse\" ";
        *m_defs << "cx=\"" << g->center().x() << "\" ";
        *m_defs << "cy=\"" << g->center().y() << "\" ";
        *m_defs << "fx=\"" << grad.focalPoint().x() << "\" ";
        *m_defs << "fy=\"" << grad.focalPoint().y() << "\" ";
        double r = sqrt( pow( grad.vector().x() - grad.origin().x(), 2 ) + pow( grad.vector().y() - grad.origin().y(), 2 ) );
        *m_defs << "r=\"" << QString().setNum( r ) << "\" ";
        *m_defs << spreadMethod[g->spread()];
        *m_defs << ">" << endl;

        // color stops
        getColorStops( gradient->stops() );

        printIndentation( m_defs, m_indent2 );
        *m_defs << "</radialGradient>" << endl;
        *m_body << "url(#" << uid << ")";
        */
    }

    return uid;
}

QString SvgStyleWriter::saveSvgPattern(QSharedPointer<KoPatternBackground> pattern, KoShape *shape, SvgSavingContext &context)
{
    const QString uid = context.createUID("pattern");

    const QSizeF shapeSize = shape->size();
    const QSizeF patternSize = pattern->patternDisplaySize();
    const QSize imageSize = pattern->pattern().size();

    // calculate offset in point
    QPointF offset = pattern->referencePointOffset();
    offset.rx() = 0.01 * offset.x() * patternSize.width();
    offset.ry() = 0.01 * offset.y() * patternSize.height();

    // now take the reference point into account
    switch (pattern->referencePoint()) {
    case KoPatternBackground::TopLeft:
        break;
    case KoPatternBackground::Top:
        offset += QPointF(0.5 * shapeSize.width(), 0.0);
        break;
    case KoPatternBackground::TopRight:
        offset += QPointF(shapeSize.width(), 0.0);
        break;
    case KoPatternBackground::Left:
        offset += QPointF(0.0, 0.5 * shapeSize.height());
        break;
    case KoPatternBackground::Center:
        offset += QPointF(0.5 * shapeSize.width(), 0.5 * shapeSize.height());
        break;
    case KoPatternBackground::Right:
        offset += QPointF(shapeSize.width(), 0.5 * shapeSize.height());
        break;
    case KoPatternBackground::BottomLeft:
        offset += QPointF(0.0, shapeSize.height());
        break;
    case KoPatternBackground::Bottom:
        offset += QPointF(0.5 * shapeSize.width(), shapeSize.height());
        break;
    case KoPatternBackground::BottomRight:
        offset += QPointF(shapeSize.width(), shapeSize.height());
        break;
    }

    offset = shape->absoluteTransformation().map(offset);

    context.styleWriter().startElement("pattern");
    context.styleWriter().addAttribute("id", uid);
    context.styleWriter().addAttribute("x", SvgUtil::toUserSpace(offset.x()));
    context.styleWriter().addAttribute("y", SvgUtil::toUserSpace(offset.y()));

    if (pattern->repeat() == KoPatternBackground::Stretched) {
        context.styleWriter().addAttribute("width", "100%");
        context.styleWriter().addAttribute("height", "100%");
        context.styleWriter().addAttribute("patternUnits", "objectBoundingBox");
    } else {
        context.styleWriter().addAttribute("width", SvgUtil::toUserSpace(patternSize.width()));
        context.styleWriter().addAttribute("height", SvgUtil::toUserSpace(patternSize.height()));
        context.styleWriter().addAttribute("patternUnits", "userSpaceOnUse");
    }

    context.styleWriter().addAttribute("viewBox", QString("0 0 %1 %2").arg(KisDomUtils::toString(imageSize.width())).arg(KisDomUtils::toString(imageSize.height())));
    //*m_defs << " patternContentUnits=\"userSpaceOnUse\"";

    context.styleWriter().startElement("image");
    context.styleWriter().addAttribute("x", "0");
    context.styleWriter().addAttribute("y", "0");
    context.styleWriter().addAttribute("width", QString("%1px").arg(KisDomUtils::toString(imageSize.width())));
    context.styleWriter().addAttribute("height", QString("%1px").arg(KisDomUtils::toString(imageSize.height())));

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    if (pattern->pattern().save(&buffer, "PNG")) {
        const QString mimeType = KisMimeDatabase::mimeTypeForSuffix("*.png");
        context.styleWriter().addAttribute("xlink:href", "data:"+ mimeType + ";base64," + ba.toBase64());
    }

    context.styleWriter().endElement(); // image
    context.styleWriter().endElement(); // pattern

    return uid;
}

QString SvgStyleWriter::saveSvgVectorPattern(QSharedPointer<KoVectorPatternBackground> pattern, KoShape *parentShape, SvgSavingContext &context)
{
    const QString uid = context.createUID("pattern");

    context.styleWriter().startElement("pattern");
    context.styleWriter().addAttribute("id", uid);

    context.styleWriter().addAttribute("patternUnits", KoFlake::coordinateToString(pattern->referenceCoordinates()));
    context.styleWriter().addAttribute("patternContentUnits", KoFlake::coordinateToString(pattern->contentCoordinates()));

    const QRectF rect = pattern->referenceRect();

    context.styleWriter().addAttribute("x", rect.x());
    context.styleWriter().addAttribute("y", rect.y());
    context.styleWriter().addAttribute("width", rect.width());
    context.styleWriter().addAttribute("height", rect.height());

    SvgUtil::writeTransformAttributeLazy("patternTransform", pattern->patternTransform(), context.styleWriter());

    if (pattern->contentCoordinates() == KoFlake::ObjectBoundingBox) {
        // TODO: move this normalization into the KoVectorPatternBackground itself

        QList<KoShape*> shapes = pattern->shapes();
        QList<KoShape*> clonedShapes;

        const QRectF dstShapeBoundingRect = parentShape->outlineRect();
        const QTransform relativeToShape = KisAlgebra2D::mapToRect(dstShapeBoundingRect);
        const QTransform shapeToRelative = relativeToShape.inverted();

        Q_FOREACH (KoShape *shape, shapes) {
            KoShape *clone = shape->cloneShape();
            clone->applyAbsoluteTransformation(shapeToRelative);
            clonedShapes.append(clone);
        }

        embedShapes(clonedShapes, context.styleWriter());
        qDeleteAll(clonedShapes);

    } else {
        QList<KoShape*> shapes = pattern->shapes();
        embedShapes(shapes, context.styleWriter());
    }

    context.styleWriter().endElement(); // pattern

    return uid;
}
