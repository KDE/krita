/* This file is part of the KDE project
 * Copyright (C) 2002-2005,2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2002-2004 Nicolas Goutte <nicolasg@snafu.de>
 * Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * Copyright (C) 2005-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2005,2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2007 Inge Wallin <inge@lysator.liu.se>
 * Copyright (C) 2007-2008,2010 Thorsten Zachmann <zachmann@kde.org>

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

#include "SvgParser.h"
#include "SvgUtil.h"
#include "SvgShape.h"

#include <KoShape.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactoryBase.h>
#include <KoShapeLayer.h>
#include <KoShapeContainer.h>
#include <KoShapeGroup.h>
#include <KoPathShape.h>
#include <KoDocumentResourceManager.h>
#include <KoPathShapeLoader.h>
#include <commands/KoShapeGroupCommand.h>
#include <commands/KoShapeUngroupCommand.h>
#include <KoImageData.h>
#include <KoImageCollection.h>
#include <KoColorBackground.h>
#include <KoGradientBackground.h>
#include <KoPatternBackground.h>
#include <KoFilterEffectRegistry.h>
#include <KoFilterEffect.h>
#include "KoFilterEffectStack.h"
#include "KoFilterEffectLoadingContext.h"
#include <KoClipPath.h>
#include <KoXmlNS.h>

#include <KDebug>

#include <QColor>


SvgParser::SvgParser(KoDocumentResourceManager *documentResourceManager)
    : m_context(documentResourceManager)
    , m_documentResourceManager(documentResourceManager)
{
}

SvgParser::~SvgParser()
{
}

void SvgParser::setXmlBaseDir(const QString &baseDir)
{
    m_context.setInitialXmlBaseDir(baseDir);
}

QList<KoShape*> SvgParser::shapes() const
{
    return m_shapes;
}

// Helper functions
// ---------------------------------------------------------------------------------------

SvgGradientHelper* SvgParser::findGradient(const QString &id, const QString &href)
{
    // check if gradient was already parsed, and return it
    if (m_gradients.contains(id))
        return &m_gradients[ id ];

    // check if gradient was stored for later parsing
    if (!m_context.hasDefinition(id))
        return 0;

    const KoXmlElement &e = m_context.definition(id);
    if (!e.tagName().contains("Gradient"))
        return 0;

    if (e.childNodesCount() == 0) {
        QString mhref = e.attribute("xlink:href").mid(1);

        if (m_context.hasDefinition(mhref))
            return findGradient(mhref, id);
        else
            return 0;
    } else {
        // ok parse gradient now
        if (! parseGradient(m_context.definition(id), m_context.definition(href)))
            return 0;
    }

    // return successfully parsed gradient or NULL
    QString n;
    if (href.isEmpty())
        n = id;
    else
        n = href;

    if (m_gradients.contains(n))
        return &m_gradients[ n ];
    else
        return 0;
}

SvgPatternHelper* SvgParser::findPattern(const QString &id)
{
    // check if pattern was already parsed, and return it
    if (m_patterns.contains(id))
        return &m_patterns[ id ];

    // check if pattern was stored for later parsing
    if (!m_context.hasDefinition(id))
        return 0;

    SvgPatternHelper pattern;

    const KoXmlElement &e = m_context.definition(id);
    if (e.tagName() != "pattern")
        return 0;

    // are we referencing another pattern ?
    if (e.hasAttribute("xlink:href")) {
        QString mhref = e.attribute("xlink:href").mid(1);
        SvgPatternHelper *refPattern = findPattern(mhref);
        // inherit attributes of referenced pattern
        if (refPattern)
            pattern = *refPattern;
    }

    // ok parse pattern now
    parsePattern(pattern, m_context.definition(id));
    // add to parsed pattern list
    m_patterns.insert(id, pattern);

    return &m_patterns[ id ];
}

SvgFilterHelper* SvgParser::findFilter(const QString &id, const QString &href)
{
    // check if filter was already parsed, and return it
    if (m_filters.contains(id))
        return &m_filters[ id ];

    // check if filter was stored for later parsing
    if (!m_context.hasDefinition(id))
        return 0;

    const KoXmlElement &e = m_context.definition(id);
    if (e.childNodesCount() == 0) {
        QString mhref = e.attribute("xlink:href").mid(1);

        if (m_context.hasDefinition(mhref))
            return findFilter(mhref, id);
        else
            return 0;
    } else {
        // ok parse filter now
        if (! parseFilter(m_context.definition(id), m_context.definition(href)))
            return 0;
    }

    // return successfully parsed filter or NULL
    QString n;
    if (href.isEmpty())
        n = id;
    else
        n = href;

    if (m_filters.contains(n))
        return &m_filters[ n ];
    else
        return 0;
}

SvgClipPathHelper* SvgParser::findClipPath(const QString &id, const QString &href)
{
    // check if clip path was already parsed, and return it
    if (m_clipPaths.contains(id))
        return &m_clipPaths[ id ];

    // check if clip path was stored for later parsing
    if (!m_context.hasDefinition(id))
        return 0;

    const KoXmlElement &e = m_context.definition(id);
    if (e.childNodesCount() == 0) {
        QString mhref = e.attribute("xlink:href").mid(1);

        if (m_context.hasDefinition(mhref))
            return findClipPath(mhref, id);
        else
            return 0;
    } else {
        // ok clip path filter now
        if (! parseClipPath(m_context.definition(id), m_context.definition(href)))
            return 0;
    }

    // return successfully parsed clip path or NULL
    const QString n = href.isEmpty() ? id : href;
    if (m_clipPaths.contains(n))
        return &m_clipPaths[ n ];
    else
        return 0;
}

// Parsing functions
// ---------------------------------------------------------------------------------------

qreal SvgParser::parseUnit(const QString &unit, bool horiz, bool vert, const QRectF &bbox)
{
    return SvgUtil::parseUnit(m_context.currentGC(), unit, horiz, vert, bbox);
}

qreal SvgParser::parseUnitX(const QString &unit)
{
    return SvgUtil::parseUnitX(m_context.currentGC(), unit);
}

qreal SvgParser::parseUnitY(const QString &unit)
{
    return SvgUtil::parseUnitY(m_context.currentGC(), unit);
}

qreal SvgParser::parseUnitXY(const QString &unit)
{
    return SvgUtil::parseUnitXY(m_context.currentGC(), unit);
}

bool SvgParser::parseGradient(const KoXmlElement &e, const KoXmlElement &referencedBy)
{
    // IMPROVEMENTS:
    // - Store the parsed colorstops in some sort of a cache so they don't need to be parsed again.
    // - A gradient inherits attributes it does not have from the referencing gradient.
    // - Gradients with no color stops have no fill or stroke.
    // - Gradients with one color stop have a solid color.

    SvgGraphicsContext *gc = m_context.currentGC();
    if (!gc)
        return false;

    SvgGradientHelper gradhelper;

    if (e.hasAttribute("xlink:href")) {
        QString href = e.attribute("xlink:href").mid(1);
        if (! href.isEmpty()) {
            // copy the referenced gradient if found
            SvgGradientHelper *pGrad = findGradient(href);
            if (pGrad)
                gradhelper = *pGrad;
        } else {
            //gc->fillType = SvgGraphicsContext::None; // <--- TODO Fill OR Stroke are none
            return false;
        }
    }

    // Use the gradient that is referencing, or if there isn't one, the original gradient.
    KoXmlElement b;
    if (!referencedBy.isNull())
        b = referencedBy;
    else
        b = e;

    QString gradientId = b.attribute("id");

    if (! gradientId.isEmpty()) {
        // check if we have this gradient already parsed
        // copy existing gradient if it exists
        if (m_gradients.find(gradientId) != m_gradients.end())
            gradhelper.copyGradient(m_gradients[ gradientId ].gradient());
    }

    if (b.attribute("gradientUnits") == "userSpaceOnUse")
        gradhelper.setGradientUnits(SvgGradientHelper::UserSpaceOnUse);

    // parse color prop
    QColor c = gc->currentColor;

    if (!b.attribute("color").isEmpty()) {
        m_context.styleParser().parseColor(c, b.attribute("color"));
    } else {
        // try style attr
        QString style = b.attribute("style").simplified();
        QStringList substyles = style.split(';', QString::SkipEmptyParts);
        for (QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it) {
            QStringList substyle = it->split(':');
            QString command = substyle[0].trimmed();
            QString params  = substyle[1].trimmed();
            if (command == "color")
                m_context.styleParser().parseColor(c, params);
        }
    }
    gc->currentColor = c;

    if (b.tagName() == "linearGradient") {
        QLinearGradient *g = new QLinearGradient();
        if (gradhelper.gradientUnits() == SvgGradientHelper::ObjectBoundingBox) {
            g->setCoordinateMode(QGradient::ObjectBoundingMode);
            g->setStart(QPointF(SvgUtil::fromPercentage(b.attribute("x1", "0%")),
                                SvgUtil::fromPercentage(b.attribute("y1", "0%"))));
            g->setFinalStop(QPointF(SvgUtil::fromPercentage(b.attribute("x2", "100%")),
                                    SvgUtil::fromPercentage(b.attribute("y2", "0%"))));
        } else {
            g->setStart(QPointF(SvgUtil::fromUserSpace(b.attribute("x1").toDouble()),
                                SvgUtil::fromUserSpace(b.attribute("y1").toDouble())));
            g->setFinalStop(QPointF(SvgUtil::fromUserSpace(b.attribute("x2").toDouble()),
                                    SvgUtil::fromUserSpace(b.attribute("y2").toDouble())));
        }
        // preserve color stops
        if (gradhelper.gradient())
            g->setStops(gradhelper.gradient()->stops());
        gradhelper.setGradient(g);
    } else if (b.tagName() == "radialGradient") {
        QRadialGradient *g = new QRadialGradient();
        if (gradhelper.gradientUnits() == SvgGradientHelper::ObjectBoundingBox) {
            g->setCoordinateMode(QGradient::ObjectBoundingMode);
            g->setCenter(QPointF(SvgUtil::fromPercentage(b.attribute("cx", "50%")),
                                 SvgUtil::fromPercentage(b.attribute("cy", "50%"))));
            g->setRadius(SvgUtil::fromPercentage(b.attribute("r", "50%")));
            g->setFocalPoint(QPointF(SvgUtil::fromPercentage(b.attribute("fx", "50%")),
                                     SvgUtil::fromPercentage(b.attribute("fy", "50%"))));
        } else {
            g->setCenter(QPointF(SvgUtil::fromUserSpace(b.attribute("cx").toDouble()),
                                 SvgUtil::fromUserSpace(b.attribute("cy").toDouble())));
            g->setFocalPoint(QPointF(SvgUtil::fromUserSpace(b.attribute("fx").toDouble()),
                                     SvgUtil::fromUserSpace(b.attribute("fy").toDouble())));
            g->setRadius(SvgUtil::fromUserSpace(b.attribute("r").toDouble()));
        }
        // preserve color stops
        if (gradhelper.gradient())
            g->setStops(gradhelper.gradient()->stops());
        gradhelper.setGradient(g);
    } else {
        return false;
    }

    // handle spread method
    QString spreadMethod = b.attribute("spreadMethod");
    if (!spreadMethod.isEmpty()) {
        if (spreadMethod == "reflect")
            gradhelper.gradient()->setSpread(QGradient::ReflectSpread);
        else if (spreadMethod == "repeat")
            gradhelper.gradient()->setSpread(QGradient::RepeatSpread);
        else
            gradhelper.gradient()->setSpread(QGradient::PadSpread);
    } else
        gradhelper.gradient()->setSpread(QGradient::PadSpread);

    // Parse the color stops. The referencing gradient does not have colorstops,
    // so use the stops from the gradient it references to (e in this case and not b)
    m_context.styleParser().parseColorStops(gradhelper.gradient(), e);
    gradhelper.setTransform(SvgUtil::parseTransform(b.attribute("gradientTransform")));
    m_gradients.insert(gradientId, gradhelper);

    return true;
}

void SvgParser::parsePattern(SvgPatternHelper &pattern, const KoXmlElement &e)
{
    if (e.attribute("patternUnits") == "userSpaceOnUse") {
        pattern.setPatternUnits(SvgPatternHelper::UserSpaceOnUse);
    }
    if (e.attribute("patternContentUnits") == "objectBoundingBox") {
        pattern.setPatternContentUnits(SvgPatternHelper::ObjectBoundingBox);
    }
    const QString viewBox = e.attribute("viewBox");
    if (!viewBox.isEmpty()) {
        pattern.setPatternContentViewbox(SvgUtil::parseViewBox(viewBox));
    }
    const QString transform = e.attribute("patternTransform");
    if (!transform.isEmpty()) {
        pattern.setTransform(SvgUtil::parseTransform(transform));
    }

    const QString x = e.attribute("x");
    const QString y = e.attribute("y");
    const QString w = e.attribute("width");
    const QString h = e.attribute("height");
    // parse tile reference rectangle
    if (pattern.patternUnits() == SvgPatternHelper::UserSpaceOnUse) {
        if (!x.isEmpty() && !y.isEmpty()) {
            pattern.setPosition(QPointF(parseUnitX(x), parseUnitY(y)));
        }
        if (!w.isEmpty() && !h.isEmpty()) {
            pattern.setSize(QSizeF(parseUnitX(w), parseUnitY(h)));
        }
    } else {
        // x, y, width, height are in percentages of the object referencing the pattern
        // so we just parse the percentages
        if (!x.isEmpty() && !y.isEmpty()) {
            pattern.setPosition(QPointF(SvgUtil::fromPercentage(x), SvgUtil::fromPercentage(y)));
        }
        if (!w.isEmpty() && !h.isEmpty()) {
            pattern.setSize(QSizeF(SvgUtil::fromPercentage(w), SvgUtil::fromPercentage(h)));
        }
    }

    if (e.hasChildNodes()) {
        pattern.setContent(e);
    }
}

bool SvgParser::parseFilter(const KoXmlElement &e, const KoXmlElement &referencedBy)
{
    SvgFilterHelper filter;

    // Use the filter that is referencing, or if there isn't one, the original filter
    KoXmlElement b;
    if (!referencedBy.isNull())
        b = referencedBy;
    else
        b = e;

    // check if we are referencing another filter
    if (e.hasAttribute("xlink:href")) {
        QString href = e.attribute("xlink:href").mid(1);
        if (! href.isEmpty()) {
            // copy the referenced filter if found
            SvgFilterHelper *refFilter = findFilter(href);
            if (refFilter)
                filter = *refFilter;
        }
    } else {
        filter.setContent(b);
    }

    if (b.attribute("filterUnits") == "userSpaceOnUse")
        filter.setFilterUnits(SvgFilterHelper::UserSpaceOnUse);
    if (b.attribute("primitiveUnits") == "objectBoundingBox")
        filter.setPrimitiveUnits(SvgFilterHelper::ObjectBoundingBox);

    // parse filter region rectangle
    if (filter.filterUnits() == SvgFilterHelper::UserSpaceOnUse) {
        filter.setPosition(QPointF(parseUnitX(b.attribute("x")),
                                   parseUnitY(b.attribute("y"))));
        filter.setSize(QSizeF(parseUnitX(b.attribute("width")),
                              parseUnitY(b.attribute("height"))));
    } else {
        // x, y, width, height are in percentages of the object referencing the filter
        // so we just parse the percentages
        filter.setPosition(QPointF(SvgUtil::fromPercentage(b.attribute("x", "-0.1")),
                                   SvgUtil::fromPercentage(b.attribute("y", "-0.1"))));
        filter.setSize(QSizeF(SvgUtil::fromPercentage(b.attribute("width", "1.2")),
                              SvgUtil::fromPercentage(b.attribute("height", "1.2"))));
    }

    m_filters.insert(b.attribute("id"), filter);

    return true;
}

bool SvgParser::parseClipPath(const KoXmlElement &e, const KoXmlElement &referencedBy)
{
    SvgClipPathHelper clipPath;

    // Use the filter that is referencing, or if there isn't one, the original filter
    KoXmlElement b;
    if (!referencedBy.isNull())
        b = referencedBy;
    else
        b = e;

    // check if we are referencing another clip path
    if (e.hasAttribute("xlink:href")) {
        QString href = e.attribute("xlink:href").mid(1);
        if (! href.isEmpty()) {
            // copy the referenced clip path if found
            SvgClipPathHelper *refClipPath = findClipPath(href);
            if (refClipPath)
                clipPath = *refClipPath;
        }
    } else {
        clipPath.setContent(b);
    }

    if (b.attribute("clipPathUnits") == "objectBoundingBox")
        clipPath.setClipPathUnits(SvgClipPathHelper::ObjectBoundingBox);


    m_clipPaths.insert(b.attribute("id"), clipPath);

    return true;
}

void SvgParser::applyStyle(KoShape *obj, const KoXmlElement &e)
{
    applyStyle(obj, m_context.styleParser().collectStyles(e));
}

void SvgParser::applyStyle(KoShape *obj, const SvgStyles &styles)
{
    SvgGraphicsContext *gc = m_context.currentGC();
    if (!gc)
        return;

    m_context.styleParser().parseStyle(styles);

    if (!obj)
        return;

    if (!dynamic_cast<KoShapeGroup*>(obj)) {
        applyFillStyle(obj);
        applyStrokeStyle(obj);
    }
    applyFilter(obj);
    applyClipping(obj);

    if (! gc->display)
        obj->setVisible(false);
    obj->setTransparency(1.0 - gc->opacity);
}

void SvgParser::applyFillStyle(KoShape *shape)
{
    SvgGraphicsContext *gc = m_context.currentGC();
    if (! gc)
        return;

    if (gc->fillType == SvgGraphicsContext::None) {
        shape->setBackground(0);
    } else if (gc->fillType == SvgGraphicsContext::Solid) {
        shape->setBackground(new KoColorBackground(gc->fillColor));
    } else if (gc->fillType == SvgGraphicsContext::Complex) {
        // try to find referenced gradient
        SvgGradientHelper *gradient = findGradient(gc->fillId);
        if (gradient) {
            // great, we have a gradient fill
            KoGradientBackground *bg = 0;
            if (gradient->gradientUnits() == SvgGradientHelper::ObjectBoundingBox) {
                bg = new KoGradientBackground(*gradient->gradient());
                bg->setTransform(gradient->transform());
            } else {
                QGradient *convertedGradient = SvgGradientHelper::convertGradient(gradient->gradient(), shape->size());
                bg = new KoGradientBackground(*convertedGradient);
                delete convertedGradient;
                QTransform invShapematrix = shape->transformation().inverted();
                bg->setTransform(gradient->transform() * gc->matrix * invShapematrix);
            }
            shape->setBackground(bg);
        } else {
            // try to find referenced pattern
            SvgPatternHelper *pattern = findPattern(gc->fillId);
            KoImageCollection *imageCollection = m_documentResourceManager->imageCollection();
            if (pattern && imageCollection) {
                // great we have a pattern fill
                QRectF objectBound = QRectF(QPoint(), shape->size());
                QRectF currentBoundbox = gc->currentBoundbox;

                // properties from the object are not inherited
                // so we are creating a new context without copying
                SvgGraphicsContext *gc = m_context.pushGraphicsContext(pattern->content(), false);

                // the pattern establishes a new coordinate system with its
                // origin at the patterns x and y attributes
                gc->matrix = QTransform();
                // object bounding box units are relative to the object the pattern is applied
                if (pattern->patternContentUnits() == SvgPatternHelper::ObjectBoundingBox) {
                    gc->currentBoundbox = objectBound;
                    gc->forcePercentage = true;
                } else {
                    // inherit the current bounding box
                    gc->currentBoundbox = currentBoundbox;
                }

                applyStyle(0, pattern->content());

                // parse the pattern content elements
                QList<KoShape*> patternContent = parseContainer(pattern->content());

                // generate the pattern image from the shapes and the object bounding rect
                QImage image = pattern->generateImage(objectBound, patternContent);

                m_context.popGraphicsContext();

                // delete the shapes created from the pattern content
                qDeleteAll(patternContent);

                if (!image.isNull()) {
                    KoPatternBackground *bg = new KoPatternBackground(imageCollection);
                    bg->setPattern(image);

                    QPointF refPoint = shape->documentToShape(pattern->position(objectBound));
                    QSizeF tileSize = pattern->size(objectBound);

                    bg->setPatternDisplaySize(tileSize);
                    if (pattern->patternUnits() == SvgPatternHelper::ObjectBoundingBox) {
                        if (tileSize == objectBound.size())
                            bg->setRepeat(KoPatternBackground::Stretched);
                    }

                    // calculate pattern reference point offset in percent of tileSize
                    // and relative to the topleft corner of the shape
                    qreal fx = refPoint.x() / tileSize.width();
                    qreal fy = refPoint.y() / tileSize.height();
                    if (fx < 0.0)
                        fx = floor(fx);
                    else if (fx > 1.0)
                        fx = ceil(fx);
                    else
                        fx = 0.0;
                    if (fy < 0.0)
                        fy = floor(fy);
                    else if (fx > 1.0)
                        fy = ceil(fy);
                    else
                        fy = 0.0;
                    qreal offsetX = 100.0 * (refPoint.x() - fx * tileSize.width()) / tileSize.width();
                    qreal offsetY = 100.0 * (refPoint.y() - fy * tileSize.height()) / tileSize.height();
                    bg->setReferencePointOffset(QPointF(offsetX, offsetY));

                    shape->setBackground(bg);
                }
            } else {
                // no referenced fill found, use fallback color
                shape->setBackground(new KoColorBackground(gc->fillColor));
            }
        }
    }

    KoPathShape *path = dynamic_cast<KoPathShape*>(shape);
    if (path)
        path->setFillRule(gc->fillRule);
}

void SvgParser::applyStrokeStyle(KoShape *shape)
{
    SvgGraphicsContext *gc = m_context.currentGC();
    if (! gc)
        return;

    if (gc->strokeType == SvgGraphicsContext::None) {
        shape->setStroke(0);
    } else if (gc->strokeType == SvgGraphicsContext::Solid) {
        double lineWidth = gc->stroke.lineWidth();
        QVector<qreal> dashes = gc->stroke.lineDashes();

        KoShapeStroke *stroke = new KoShapeStroke(gc->stroke);

        // apply line width to dashes and dash offset
        if (dashes.count() && lineWidth > 0.0) {
            QVector<qreal> dashes = stroke->lineDashes();
            for (int i = 0; i < dashes.count(); ++i)
                dashes[i] /= lineWidth;
            double dashOffset = stroke->dashOffset();
            stroke->setLineStyle(Qt::CustomDashLine, dashes);
            stroke->setDashOffset(dashOffset / lineWidth);
        } else {
            stroke->setLineStyle(Qt::SolidLine, QVector<qreal>());
        }
        shape->setStroke(stroke);
    } else if (gc->strokeType == SvgGraphicsContext::Complex) {
        // try to find referenced gradient
        SvgGradientHelper *gradient = findGradient(gc->strokeId);
        if (gradient) {
            // great, we have a gradient stroke
            QBrush brush;
            if (gradient->gradientUnits() == SvgGradientHelper::ObjectBoundingBox) {
                brush = *gradient->gradient();
                brush.setTransform(gradient->transform());
            } else {
                QGradient *convertedGradient(SvgGradientHelper::convertGradient(gradient->gradient(), shape->size()));
                brush = *convertedGradient;
                delete convertedGradient;
                brush.setTransform(gradient->transform() * gc->matrix * shape->transformation().inverted());
            }
            KoShapeStroke *stroke = new KoShapeStroke(gc->stroke);
            stroke->setLineBrush(brush);
            stroke->setLineStyle(Qt::SolidLine, QVector<qreal>());
            shape->setStroke(stroke);
        } else {
            // no referenced stroke found, use fallback color
            KoShapeStroke *stroke = new KoShapeStroke(gc->stroke);
            stroke->setLineStyle(Qt::SolidLine, QVector<qreal>());
            shape->setStroke(stroke);
        }
    }
}

void SvgParser::applyFilter(KoShape *shape)
{
    SvgGraphicsContext *gc = m_context.currentGC();
    if (! gc)
        return;

    if (gc->filterId.isEmpty())
        return;

    SvgFilterHelper *filter = findFilter(gc->filterId);
    if (! filter)
        return;

    KoXmlElement content = filter->content();

    // parse filter region
    QRectF bound(shape->position(), shape->size());
    // work on bounding box without viewbox tranformation applied
    // so user space coordinates of bounding box and filter region match up
    bound = gc->viewboxTransform.inverted().mapRect(bound);

    QRectF filterRegion(filter->position(bound), filter->size(bound));

    // convert filter region to boundingbox units
    QRectF objectFilterRegion;
    objectFilterRegion.setTopLeft(SvgUtil::userSpaceToObject(filterRegion.topLeft(), bound));
    objectFilterRegion.setSize(SvgUtil::userSpaceToObject(filterRegion.size(), bound));

    KoFilterEffectLoadingContext context(m_context.xmlBaseDir());
    context.setShapeBoundingBox(bound);
    // enable units conversion
    context.enableFilterUnitsConversion(filter->filterUnits() == SvgFilterHelper::UserSpaceOnUse);
    context.enableFilterPrimitiveUnitsConversion(filter->primitiveUnits() == SvgFilterHelper::UserSpaceOnUse);

    KoFilterEffectRegistry *registry = KoFilterEffectRegistry::instance();

    KoFilterEffectStack *filterStack = 0;

    QSet<QString> stdInputs;
    stdInputs << "SourceGraphic" << "SourceAlpha";
    stdInputs << "BackgroundImage" << "BackgroundAlpha";
    stdInputs << "FillPaint" << "StrokePaint";

    QMap<QString, KoFilterEffect*> inputs;

    // create the filter effects and add them to the shape
    for (KoXmlNode n = content.firstChild(); !n.isNull(); n = n.nextSibling()) {
        KoXmlElement primitive = n.toElement();
        KoFilterEffect *filterEffect = registry->createFilterEffectFromXml(primitive, context);
        if (!filterEffect) {
            kWarning(30514) << "filter effect" << primitive.tagName() << "is not implemented yet";
            continue;
        }

        const QString input = primitive.attribute("in");
        if (!input.isEmpty()) {
            filterEffect->setInput(0, input);
        }
        const QString output = primitive.attribute("result");
        if (!output.isEmpty()) {
            filterEffect->setOutput(output);
        }

        QRectF subRegion;
        // parse subregion
        if (filter->primitiveUnits() == SvgFilterHelper::UserSpaceOnUse) {
            const QString xa = primitive.attribute("x");
            const QString ya = primitive.attribute("y");
            const QString wa = primitive.attribute("width");
            const QString ha = primitive.attribute("height");

            if (xa.isEmpty() || ya.isEmpty() || wa.isEmpty() || ha.isEmpty()) {
                bool hasStdInput = false;
                bool isFirstEffect = filterStack == 0;
                // check if one of the inputs is a standard input
                foreach(const QString &input, filterEffect->inputs()) {
                    if ((isFirstEffect && input.isEmpty()) || stdInputs.contains(input)) {
                        hasStdInput = true;
                        break;
                    }
                }
                if (hasStdInput || primitive.tagName() == "feImage") {
                    // default to 0%, 0%, 100%, 100%
                    subRegion.setTopLeft(QPointF(0, 0));
                    subRegion.setSize(QSizeF(1, 1));
                } else {
                    // defaults to bounding rect of all referenced nodes
                    foreach(const QString &input, filterEffect->inputs()) {
                        if (!inputs.contains(input))
                            continue;

                        KoFilterEffect *inputFilter = inputs[input];
                        if (inputFilter)
                            subRegion |= inputFilter->filterRect();
                    }
                }
            } else {
                const qreal x = parseUnitX(xa);
                const qreal y = parseUnitY(ya);
                const qreal w = parseUnitX(wa);
                const qreal h = parseUnitY(ha);
                subRegion.setTopLeft(SvgUtil::userSpaceToObject(QPointF(x, y), bound));
                subRegion.setSize(SvgUtil::userSpaceToObject(QSizeF(w, h), bound));
            }
        } else {
            // x, y, width, height are in percentages of the object referencing the filter
            // so we just parse the percentages
            const qreal x = SvgUtil::fromPercentage(primitive.attribute("x", "0"));
            const qreal y = SvgUtil::fromPercentage(primitive.attribute("y", "0"));
            const qreal w = SvgUtil::fromPercentage(primitive.attribute("width", "1"));
            const qreal h = SvgUtil::fromPercentage(primitive.attribute("height", "1"));
            subRegion = QRectF(QPointF(x, y), QSizeF(w, h));
        }

        filterEffect->setFilterRect(subRegion);

        if (!filterStack)
            filterStack = new KoFilterEffectStack();

        filterStack->appendFilterEffect(filterEffect);
        inputs[filterEffect->output()] = filterEffect;
    }
    if (filterStack) {
        filterStack->setClipRect(objectFilterRegion);
        shape->setFilterEffectStack(filterStack);
    }
}

void SvgParser::applyClipping(KoShape *shape)
{
    SvgGraphicsContext *gc = m_context.currentGC();
    if (! gc)
        return;

    if (gc->clipPathId.isEmpty())
        return;

    SvgClipPathHelper *clipPath = findClipPath(gc->clipPathId);
    if (! clipPath)
        return;

    kDebug(30514) << "applying clip path" << gc->clipPathId << "clip rule" << gc->clipRule;

    const bool boundingBoxUnits = clipPath->clipPathUnits() == SvgClipPathHelper::ObjectBoundingBox;
    kDebug(30514) << "using" << (boundingBoxUnits ? "boundingBoxUnits" : "userSpaceOnUse");

    QTransform shapeMatrix = shape->absoluteTransformation(0);
    // TODO:
    // clip path element can have a clip-path property
    // -> clip-path = intersection of children with referenced clip-path
    // any of its children can have a clip-path property
    // -> child element is clipped and the ORed with other children
    m_context.pushGraphicsContext();

    if (boundingBoxUnits) {
        SvgGraphicsContext *gc = m_context.currentGC();
        gc->matrix.reset();
        gc->viewboxTransform.reset();
        gc->currentBoundbox = shape->boundingRect();
        gc->forcePercentage = true;
    }

    QList<KoShape*> clipShapes = parseContainer(clipPath->content());
    QList<KoPathShape*> pathShapes;
    while (!clipShapes.isEmpty()) {
        KoShape *clipShape = clipShapes.first();
        clipShapes.removeFirst();
        // remove clip shape from list of all parsed shapes
        m_shapes.removeOne(clipShape);
        // check if we have a path shape
        KoPathShape *path = dynamic_cast<KoPathShape*>(clipShape);
        if (!path) {
            // if shape is a group, ungroup and add children to lits of clip shapes
            KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(clipShape);
            if (group) {
                QList<KoShape*> groupedShapes = group->shapes();
                KoShapeUngroupCommand cmd(group, groupedShapes);
                cmd.redo();
                clipShapes.append(groupedShapes);
            } else {
                // shape is not a group shape, use its outline as clip path
                QPainterPath outline = clipShape->absoluteTransformation(0).map(clipShape->outline());
                path = KoPathShape::createShapeFromPainterPath(outline);
            }
            delete clipShape;
        }
        if (path) {
            kDebug(30514) << "using shape" << path->name() << "as clip path";
            pathShapes.append(path);
            if (boundingBoxUnits)
                path->applyAbsoluteTransformation(shapeMatrix);
        }
    }

    m_context.popGraphicsContext();

    if (pathShapes.count()) {
        QTransform transformToShape;
        if (!boundingBoxUnits)
            transformToShape = shape->absoluteTransformation(0).inverted();
        KoClipData *clipData = new KoClipData(pathShapes);
        KoClipPath *clipPath = new KoClipPath(shape, clipData);
        clipPath->setClipRule(gc->clipRule);
        shape->setClipPath(clipPath);
    }
}

QList<KoShape*> SvgParser::parseUse(const KoXmlElement &e)
{
    QList<KoShape*> shapes;

    QString id = e.attribute("xlink:href");
    //
    if (!id.isEmpty()) {
        SvgGraphicsContext *gc = m_context.pushGraphicsContext(e);

        // TODO: use width and height attributes too
        gc->matrix.translate(parseUnitX(e.attribute("x", "0")), parseUnitY(e.attribute("y", "0")));

        QString key = id.mid(1);

        if (m_context.hasDefinition(key)) {
            const KoXmlElement &a = m_context.definition(key);
            SvgStyles styles = m_context.styleParser().mergeStyles(e, a);
            if (a.tagName() == "g" || a.tagName() == "a" || a.tagName() == "symbol") {
                m_context.pushGraphicsContext(a);

                KoShapeGroup *group = new KoShapeGroup();
                group->setZIndex(m_context.nextZIndex());

                applyStyle(0, styles);
                m_context.styleParser().parseFont(styles);

                QList<KoShape*> childShapes = parseContainer(a);

                // handle id
                applyId(a.attribute("id"), group);

                addToGroup(childShapes, group);
                applyStyle(group, styles);   // apply style to group after size is set

                shapes.append(group);

                m_context.popGraphicsContext();
            } else {
                // Create the object with the merged styles.
                // The object inherits all style attributes from the use tag, but keeps it's own attributes.
                // So, not just use the style attributes of the use tag, but merge them first.
                KoShape *shape = createObject(a, styles);
                if (shape)
                    shapes.append(shape);
            }
        } else {
            // TODO: any named object can be referenced too
        }
        m_context.popGraphicsContext();
    }

    return shapes;
}

void SvgParser::addToGroup(QList<KoShape*> shapes, KoShapeGroup *group)
{
    m_shapes += shapes;

    if (! group)
        return;

    KoShapeGroupCommand cmd(group, shapes);
    cmd.redo();
}

QList<KoShape*> SvgParser::parseSvg(const KoXmlElement &e, QSizeF *fragmentSize)
{
    // check if we are the root svg element
    const bool isRootSvg = !m_context.currentGC();

    SvgGraphicsContext *gc = m_context.pushGraphicsContext();

    applyStyle(0, e);

    QRectF viewBox;

    const QString viewBoxStr = e.attribute("viewBox");
    if (!viewBoxStr.isEmpty()) {
        viewBox = SvgUtil::parseViewBox(viewBoxStr);
    }

    const QString w = e.attribute("width");
    const QString h = e.attribute("height");
    const qreal width = w.isEmpty() ? 550.0 : parseUnit(w, true, false, viewBox);
    const qreal height = h.isEmpty() ? 841.0 : parseUnit(h, false, true, viewBox);

    QSizeF svgFragmentSize(QSizeF(width, height));

    if (fragmentSize)
        *fragmentSize = svgFragmentSize;

    gc->currentBoundbox = QRectF(QPointF(0, 0), svgFragmentSize);

    if (! isRootSvg) {
        QTransform move;
        // x and y attribute has no meaning for outermost svg elements
        const qreal x = parseUnit(e.attribute("x", "0"));
        const qreal y = parseUnit(e.attribute("y", "0"));
        move.translate(x, y);
        gc->matrix = move * gc->matrix;
        gc->viewboxTransform = move *gc->viewboxTransform;
    }

    if (!viewBoxStr.isEmpty()) {
        QTransform viewTransform;
        viewTransform.translate(viewBox.x(), viewBox.y());
        viewTransform.scale(width / viewBox.width() , height / viewBox.height());
        gc->matrix = viewTransform * gc->matrix;
        gc->viewboxTransform = viewTransform *gc->viewboxTransform;
        gc->currentBoundbox.setWidth(gc->currentBoundbox.width() * (viewBox.width() / width));
        gc->currentBoundbox.setHeight(gc->currentBoundbox.height() * (viewBox.height() / height));
    }

    QList<KoShape*> shapes = parseContainer(e);

    m_context.popGraphicsContext();

    return shapes;
}

QList<KoShape*> SvgParser::parseContainer(const KoXmlElement &e)
{
    QList<KoShape*> shapes;

    // are we parsing a switch container
    bool isSwitch = e.tagName() == "switch";

    for (KoXmlNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        KoXmlElement b = n.toElement();
        if (b.isNull())
            continue;

        if (isSwitch) {
            // if we are parsing a switch check the requiredFeatures, requiredExtensions
            // and systemLanguage attributes
            // TODO: evaluate feature list
            if (b.hasAttribute("requiredFeatures")) {
                continue;
            }
            if (b.hasAttribute("requiredExtensions")) {
                // we do not support any extensions
                continue;
            }
            if (b.hasAttribute("systemLanguage")) {
                // not implemeted yet
            }
        }

        if (b.tagName() == "svg") {
            shapes += parseSvg(b);
        } else if (b.tagName() == "g" || b.tagName() == "a" || b.tagName() == "symbol") {
            // treat svg link <a> as group so we don't miss its child elements
            m_context.pushGraphicsContext(b);

            KoShapeGroup *group = new KoShapeGroup();
            group->setZIndex(m_context.nextZIndex());

            SvgStyles styles = m_context.styleParser().collectStyles(b);
            m_context.styleParser().parseFont(styles);
            applyStyle(0, styles);   // parse style for inheritance

            QList<KoShape*> childShapes = parseContainer(b);

            // handle id
            applyId(b.attribute("id"), group);

            addToGroup(childShapes, group);

            const QString viewBoxStr = b.attribute("viewBox");
            if (!viewBoxStr.isEmpty()) {
                QRectF viewBox = SvgUtil::parseViewBox(viewBoxStr);
                QTransform viewTransform;
                viewTransform.translate(viewBox.x(), viewBox.y());
                viewTransform.scale(group->size().width() / viewBox.width() , group->size().height() / viewBox.height());
                group->applyAbsoluteTransformation(viewTransform);
            }
            applyStyle(group, styles);   // apply style to this group after size is set

            shapes.append(group);

            m_context.popGraphicsContext();
        } else if (b.tagName() == "switch") {
            m_context.pushGraphicsContext(b);
            shapes += parseContainer(b);
            m_context.popGraphicsContext();
        } else if (b.tagName() == "defs") {
            parseDefs(b);
        } else if (b.tagName() == "linearGradient" || b.tagName() == "radialGradient") {
            parseGradient(b);
        } else if (b.tagName() == "pattern") {
            m_context.addDefinition(b);
        } else if (b.tagName() == "filter") {
            parseFilter(b);
        } else if (b.tagName() == "clipPath") {
            parseClipPath(b);
        } else if (b.tagName() == "style") {
            m_context.addStyleSheet(b);
        } else if (b.tagName() == "rect" ||
                   b.tagName() == "ellipse" ||
                   b.tagName() == "circle" ||
                   b.tagName() == "line" ||
                   b.tagName() == "polyline" ||
                   b.tagName() == "polygon" ||
                   b.tagName() == "path" ||
                   b.tagName() == "image" ||
                   b.tagName() == "text") {
            KoShape *shape = createObject(b);
            if (shape)
                shapes.append(shape);
        } else if (b.tagName() == "use") {
            shapes += parseUse(b);
        } else {
            // this is an unknown element, so try to load it anyway
            // there might be a shape that handles that element
            KoShape *shape = createObject(b);
            if (shape) {
                shapes.append(shape);
            } else {
                continue;
            }
        }

        // if we are parsing a switch, stop after the first supported element
        if (isSwitch)
            break;
    }

    return shapes;
}

void SvgParser::parseDefs(const KoXmlElement &e)
{
    for (KoXmlNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        KoXmlElement b = n.toElement();
        if (b.isNull())
            continue;

        if (b.tagName() == "style") {
            m_context.addStyleSheet(b);
        } else if (b.tagName() == "defs") {
            parseDefs(b);
        } else {
            m_context.addDefinition(b);
        }
    }
}

// Creating functions
// ---------------------------------------------------------------------------------------

KoShape * SvgParser::createPath(const KoXmlElement &element)
{
    KoShape *obj = 0;
    if (element.tagName() == "line") {
        KoPathShape *path = static_cast<KoPathShape*>(createShape(KoPathShapeId));
        if (path) {
            double x1 = element.attribute("x1").isEmpty() ? 0.0 : parseUnitX(element.attribute("x1"));
            double y1 = element.attribute("y1").isEmpty() ? 0.0 : parseUnitY(element.attribute("y1"));
            double x2 = element.attribute("x2").isEmpty() ? 0.0 : parseUnitX(element.attribute("x2"));
            double y2 = element.attribute("y2").isEmpty() ? 0.0 : parseUnitY(element.attribute("y2"));
            path->clear();
            path->moveTo(QPointF(x1, y1));
            path->lineTo(QPointF(x2, y2));
            path->normalize();
            obj = path;
        }
    } else if (element.tagName() == "polyline" || element.tagName() == "polygon") {
        KoPathShape *path = static_cast<KoPathShape*>(createShape(KoPathShapeId));
        if (path) {
            path->clear();

            bool bFirst = true;
            QString points = element.attribute("points").simplified();
            points.replace(',', ' ');
            points.remove('\r');
            points.remove('\n');
            QStringList pointList = points.split(' ', QString::SkipEmptyParts);
            for (QStringList::Iterator it = pointList.begin(); it != pointList.end(); ++it) {
                QPointF point;
                point.setX(SvgUtil::fromUserSpace((*it).toDouble()));
                ++it;
                if (it == pointList.end())
                    break;
                point.setY(SvgUtil::fromUserSpace((*it).toDouble()));
                if (bFirst) {
                    path->moveTo(point);
                    bFirst = false;
                } else
                    path->lineTo(point);
            }
            if (element.tagName() == "polygon")
                path->close();

            path->setPosition(path->normalize());

            obj = path;
        }
    } else if (element.tagName() == "path") {
        KoPathShape *path = static_cast<KoPathShape*>(createShape(KoPathShapeId));
        if (path) {
            path->clear();

            KoPathShapeLoader loader(path);
            loader.parseSvg(element.attribute("d"), true);
            path->setPosition(path->normalize());

            QPointF newPosition = QPointF(SvgUtil::fromUserSpace(path->position().x()),
                                          SvgUtil::fromUserSpace(path->position().y()));
            QSizeF newSize = QSizeF(SvgUtil::fromUserSpace(path->size().width()),
                                    SvgUtil::fromUserSpace(path->size().height()));

            path->setSize(newSize);
            path->setPosition(newPosition);

            obj = path;
        }
    }

    return obj;
}

KoShape * SvgParser::createObject(const KoXmlElement &b, const SvgStyles &style)
{
    m_context.pushGraphicsContext(b);

    KoShape *obj = createShapeFromElement(b, m_context);
    if (obj) {
        obj->applyAbsoluteTransformation(m_context.currentGC()->matrix);

        SvgStyles objStyle = style.isEmpty() ? m_context.styleParser().collectStyles(b) : style;
        m_context.styleParser().parseFont(objStyle);
        applyStyle(obj, objStyle);

        // handle id
        applyId(b.attribute("id"), obj);
        obj->setZIndex(m_context.nextZIndex());
    }

    m_context.popGraphicsContext();

    return obj;
}

KoShape * SvgParser::createShapeFromElement(const KoXmlElement &element, SvgLoadingContext &context)
{
    KoShape *object = 0;

    QList<KoShapeFactoryBase*> factories = KoShapeRegistry::instance()->factoriesForElement(KoXmlNS::svg, element.tagName());
    foreach (KoShapeFactoryBase *f, factories) {
        KoShape *shape = f->createDefaultShape(m_documentResourceManager);
        if (!shape)
            continue;

        SvgShape *svgShape = dynamic_cast<SvgShape*>(shape);
        if (!svgShape) {
            delete shape;
            continue;
        }

        // reset transformation that might come from the default shape
        shape->setTransformation(QTransform());

        // reset border
        KoShapeStrokeModel *oldStroke = shape->stroke();
        shape->setStroke(0);
        delete oldStroke;

        // reset fill
        KoShapeBackground *oldFill = shape->background();
        shape->setBackground(0);
        delete oldFill;

        if (!svgShape->loadSvg(element, context)) {
            delete shape;
            continue;
        }

        object = shape;
        break;
    }

    if (!object) {
        object = createPath(element);
    }

    return object;
}

KoShape * SvgParser::createShape(const QString &shapeID)
{
    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->get(shapeID);
    if (!factory) {
        kWarning(30514) << "Could not find factory for shape id" << shapeID;
        return 0;
    }

    KoShape *shape = factory->createDefaultShape(m_documentResourceManager);
    if (!shape) {
        kWarning(30514) << "Could not create Default shape for shape id" << shapeID;
        return 0;
    }
    if (shape->shapeId().isEmpty())
        shape->setShapeId(factory->id());

    // reset tranformation that might come from the default shape
    shape->setTransformation(QTransform());

    // reset border
    KoShapeStrokeModel *oldStroke = shape->stroke();
    shape->setStroke(0);
    delete oldStroke;

    // reset fill
    KoShapeBackground *oldFill = shape->background();
    shape->setBackground(0);
    delete oldFill;

    return shape;
}

void SvgParser::applyId(const QString &id, KoShape *shape)
{
    if (id.isEmpty())
        return;

    shape->setName(id);
    m_context.registerShape(id, shape);
}
