/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2002-2005, 2007 Rob Buis <buis@kde.org>
 * SPDX-FileCopyrightText: 2002-2004 Nicolas Goutte <nicolasg@snafu.de>
 * SPDX-FileCopyrightText: 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * SPDX-FileCopyrightText: 2005-2009 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2005, 2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006-2007 Inge Wallin <inge@lysator.liu.se>
 * SPDX-FileCopyrightText: 2007-2008, 2010 Thorsten Zachmann <zachmann@kde.org>

 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SvgParser.h"

#include <cmath>

#include <FlakeDebug.h>

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QDir>

#include <KoShape.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactoryBase.h>
#include <KoShapeGroup.h>
#include <KoPathShape.h>
#include <KoDocumentResourceManager.h>
#include <KoPathShapeLoader.h>
#include <commands/KoShapeGroupCommand.h>
#include <commands/KoShapeUngroupCommand.h>
#include <KoXmlReader.h>
#include <KoImageCollection.h>
#include <KoColorBackground.h>
#include <KoGradientBackground.h>
#include <KoMeshGradientBackground.h>
#include <KoPatternBackground.h>
#include <KoFilterEffectRegistry.h>
#include <KoFilterEffect.h>
#include "KoFilterEffectStack.h"
#include "KoFilterEffectLoadingContext.h"
#include <KoClipPath.h>
#include <KoClipMask.h>
#include <KoXmlNS.h>
#include <QXmlSimpleReader>

#include "SvgMeshGradient.h"
#include "SvgMeshPatch.h"
#include "SvgUtil.h"
#include "SvgShape.h"
#include "SvgGraphicContext.h"
#include "SvgFilterHelper.h"
#include "SvgGradientHelper.h"
#include "SvgClipPathHelper.h"
#include "parsers/SvgTransformParser.h"
#include "kis_pointer_utils.h"
#include <KoVectorPatternBackground.h>
#include <KoMarker.h>

#include <text/KoSvgTextShape.h>
#include <text/KoSvgTextChunkShape.h>

#include "kis_dom_utils.h"

#include "kis_algebra_2d.h"
#include "kis_debug.h"
#include "kis_global.h"
#include <algorithm>


struct SvgParser::DeferredUseStore {
    struct El {
        El(const QDomElement* ue, const QString& key) :
            m_useElement(ue), m_key(key) {
        }
        const QDomElement* m_useElement;
        QString m_key;
    };
    DeferredUseStore(SvgParser* p) :
        m_parse(p) {
    }

    void add(const QDomElement* useE, const QString& key) {
        m_uses.push_back(El(useE, key));
    }
    bool empty() const {
        return m_uses.empty();
    }

    void checkPendingUse(const QDomElement &b, QList<KoShape*>& shapes) {
        KoShape* shape = 0;
        const QString id = b.attribute("id");

        if (id.isEmpty())
            return;

        // debugFlake << "Checking id: " << id;
        auto i = std::partition(m_uses.begin(), m_uses.end(),
                                [&](const El& e) -> bool {return e.m_key != id;});

        while (i != m_uses.end()) {
            const El& el = m_uses.back();
            if (m_parse->m_context.hasDefinition(el.m_key)) {
                // debugFlake << "Found pending use for id: " << el.m_key;
                shape = m_parse->resolveUse(*(el.m_useElement), el.m_key);
                if (shape) {
                    shapes.append(shape);
                }
            }
            m_uses.pop_back();
        }
    }

    ~DeferredUseStore() {
        while (!m_uses.empty()) {
            const El& el = m_uses.back();
            debugFlake << "WARNING: could not find path in <use xlink:href=\"#xxxxx\" expression. Losing data here. Key:"
                       << el.m_key;
            m_uses.pop_back();
        }
    }
    SvgParser* m_parse;
    std::vector<El> m_uses;
};


SvgParser::SvgParser(KoDocumentResourceManager *documentResourceManager)
    : m_context(documentResourceManager)
    , m_documentResourceManager(documentResourceManager)
{
}

SvgParser::~SvgParser()
{
    for (auto it = m_symbols.begin(); it != m_symbols.end(); ++it) {
        delete it.value();
    }
    qDeleteAll(m_defsShapes);
}

QDomDocument SvgParser::createDocumentFromSvg(QIODevice *device, QString *errorMsg, int *errorLine, int *errorColumn)
{
    QXmlInputSource source(device);
    return createDocumentFromSvg(&source, errorMsg, errorLine, errorColumn);
}

QDomDocument SvgParser::createDocumentFromSvg(const QByteArray &data, QString *errorMsg, int *errorLine, int *errorColumn)
{
    QXmlInputSource source;
    source.setData(data);

    return createDocumentFromSvg(&source, errorMsg, errorLine, errorColumn);
}

QDomDocument SvgParser::createDocumentFromSvg(const QString &data, QString *errorMsg, int *errorLine, int *errorColumn)
{
    QXmlInputSource source;
    source.setData(data);

    return createDocumentFromSvg(&source, errorMsg, errorLine, errorColumn);
}

QDomDocument SvgParser::createDocumentFromSvg(QXmlInputSource *source, QString *errorMsg, int *errorLine, int *errorColumn)
{
    // we should read all spaces to parse text node correctly
    QXmlSimpleReader reader;
    reader.setFeature("http://qt-project.org/xml/features/report-whitespace-only-CharData", true);
    reader.setFeature("http://xml.org/sax/features/namespaces", false);
    reader.setFeature("http://xml.org/sax/features/namespace-prefixes", true);

    QDomDocument doc;
    if (!doc.setContent(source, &reader, errorMsg, errorLine, errorColumn)) {
        return QDomDocument();
    }

    return doc;
}

void SvgParser::setXmlBaseDir(const QString &baseDir)
{
    m_context.setInitialXmlBaseDir(baseDir);

    setFileFetcher(
        [this](const QString &name) {
            QStringList possibleNames;
            possibleNames << name;
            possibleNames << QDir::cleanPath(QDir(m_context.xmlBaseDir()).absoluteFilePath(name));
            for (QString fileName : possibleNames) {
                QFile file(fileName);
                if (file.exists()) {
                    file.open(QIODevice::ReadOnly);
                    return file.readAll();
                }
            }
            return QByteArray();
        });
}

void SvgParser::setResolution(const QRectF boundsInPixels, qreal pixelsPerInch)
{
    KIS_ASSERT(!m_context.currentGC());
    m_context.pushGraphicsContext();
    m_context.currentGC()->isResolutionFrame = true;
    m_context.currentGC()->pixelsPerInch = pixelsPerInch;

    const qreal scale = 72.0 / pixelsPerInch;
    const QTransform t = QTransform::fromScale(scale, scale);
    m_context.currentGC()->currentBoundingBox = boundsInPixels;
    m_context.currentGC()->matrix = t;
}

void SvgParser::setDefaultKraTextVersion(int version)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_context.currentGC());
    m_context.currentGC()->textProperties.setProperty(KoSvgTextProperties::KraTextVersionId, version);
}

QList<KoShape*> SvgParser::shapes() const
{
    return m_shapes;
}

QVector<KoSvgSymbol *> SvgParser::takeSymbols()
{
    QVector<KoSvgSymbol*> symbols = m_symbols.values().toVector();
    m_symbols.clear();
    return symbols;
}

// Helper functions
// ---------------------------------------------------------------------------------------

SvgGradientHelper* SvgParser::findGradient(const QString &id)
{
    SvgGradientHelper *result = 0;

    // check if gradient was already parsed, and return it
    if (m_gradients.contains(id)) {
        result = &m_gradients[ id ];
    }

    // check if gradient was stored for later parsing
    if (!result && m_context.hasDefinition(id)) {
        const QDomElement &e = m_context.definition(id);
        if (e.tagName().contains("Gradient")) {
            result = parseGradient(m_context.definition(id));
        } else if (e.tagName() == "meshgradient") {
            result = parseMeshGradient(m_context.definition(id));
        }
    }

    return result;
}

QSharedPointer<KoVectorPatternBackground> SvgParser::findPattern(const QString &id, const KoShape *shape)
{
    QSharedPointer<KoVectorPatternBackground> result;

    // check if gradient was stored for later parsing
    if (m_context.hasDefinition(id)) {
        const QDomElement &e = m_context.definition(id);
        if (e.tagName() == "pattern") {
            result = parsePattern(m_context.definition(id), shape);
        }
    }

    return result;
}

SvgFilterHelper* SvgParser::findFilter(const QString &id, const QString &href)
{
    // check if filter was already parsed, and return it
    if (m_filters.contains(id))
        return &m_filters[ id ];

    // check if filter was stored for later parsing
    if (!m_context.hasDefinition(id))
        return 0;

    const QDomElement &e = m_context.definition(id);
    if (KoXml::childNodesCount(e) == 0) {
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

    // return successfully parsed filter or 0
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

SvgClipPathHelper* SvgParser::findClipPath(const QString &id)
{
    return m_clipPaths.contains(id) ? &m_clipPaths[id] : 0;
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

qreal SvgParser::parseAngular(const QString &unit)
{
    return SvgUtil::parseUnitAngular(m_context.currentGC(), unit);
}


SvgGradientHelper* SvgParser::parseGradient(const QDomElement &e)
{
    // IMPROVEMENTS:
    // - Store the parsed colorstops in some sort of a cache so they don't need to be parsed again.
    // - A gradient inherits attributes it does not have from the referencing gradient.
    // - Gradients with no color stops have no fill or stroke.
    // - Gradients with one color stop have a solid color.

    SvgGraphicsContext *gc = m_context.currentGC();
    if (!gc) return 0;

    SvgGradientHelper gradHelper;

    QString gradientId = e.attribute("id");
    if (gradientId.isEmpty()) return 0;

    // check if we have this gradient already parsed
    // copy existing gradient if it exists
    if (m_gradients.contains(gradientId)) {
        return &m_gradients[gradientId];
    }

    if (e.hasAttribute("xlink:href")) {
        // strip the '#' symbol
        QString href = e.attribute("xlink:href").mid(1);

        if (!href.isEmpty()) {
            // copy the referenced gradient if found
            SvgGradientHelper *pGrad = findGradient(href);
            if (pGrad) {
                gradHelper = *pGrad;
            }
        }
    }

    const QGradientStops defaultStops = gradHelper.gradient()->stops();

    if (e.attribute("gradientUnits") == "userSpaceOnUse") {
        gradHelper.setGradientUnits(KoFlake::UserSpaceOnUse);
    }

    m_context.pushGraphicsContext(e);
    uploadStyleToContext(e);

    if (e.tagName() == "linearGradient") {
        QLinearGradient *g = new QLinearGradient();
        if (gradHelper.gradientUnits() == KoFlake::ObjectBoundingBox) {
            g->setCoordinateMode(QGradient::ObjectBoundingMode);
            g->setStart(QPointF(SvgUtil::fromPercentage(e.attribute("x1", "0%")),
                                SvgUtil::fromPercentage(e.attribute("y1", "0%"))));
            g->setFinalStop(QPointF(SvgUtil::fromPercentage(e.attribute("x2", "100%")),
                                    SvgUtil::fromPercentage(e.attribute("y2", "0%"))));
        } else {
            g->setStart(QPointF(parseUnitX(e.attribute("x1")),
                                parseUnitY(e.attribute("y1"))));
            g->setFinalStop(QPointF(parseUnitX(e.attribute("x2")),
                                    parseUnitY(e.attribute("y2"))));
        }
        gradHelper.setGradient(g);

    } else if (e.tagName() == "radialGradient") {
        QRadialGradient *g = new QRadialGradient();
        if (gradHelper.gradientUnits() == KoFlake::ObjectBoundingBox) {
            g->setCoordinateMode(QGradient::ObjectBoundingMode);
            g->setCenter(QPointF(SvgUtil::fromPercentage(e.attribute("cx", "50%")),
                                 SvgUtil::fromPercentage(e.attribute("cy", "50%"))));
            g->setRadius(SvgUtil::fromPercentage(e.attribute("r", "50%")));
            g->setFocalPoint(QPointF(SvgUtil::fromPercentage(e.attribute("fx", "50%")),
                                     SvgUtil::fromPercentage(e.attribute("fy", "50%"))));
        } else {
            g->setCenter(QPointF(parseUnitX(e.attribute("cx")),
                                 parseUnitY(e.attribute("cy"))));
            g->setFocalPoint(QPointF(parseUnitX(e.attribute("fx")),
                                     parseUnitY(e.attribute("fy"))));
            g->setRadius(parseUnitXY(e.attribute("r")));
        }
        gradHelper.setGradient(g);
    } else {
        debugFlake << "WARNING: Failed to parse gradient with tag" << e.tagName();
    }

    // handle spread method
    QGradient::Spread spreadMethod = QGradient::PadSpread;
    QString spreadMethodStr = e.attribute("spreadMethod");
    if (!spreadMethodStr.isEmpty()) {
        if (spreadMethodStr == "reflect") {
            spreadMethod = QGradient::ReflectSpread;
        } else if (spreadMethodStr == "repeat") {
            spreadMethod = QGradient::RepeatSpread;
        }
    }

    gradHelper.setSpreadMode(spreadMethod);

    // Parse the color stops.
    m_context.styleParser().parseColorStops(gradHelper.gradient(), e, gc, defaultStops);

    if (e.hasAttribute("gradientTransform")) {
        SvgTransformParser p(e.attribute("gradientTransform"));
        if (p.isValid()) {
            gradHelper.setTransform(p.transform());
        }
    }

    m_context.popGraphicsContext();

    m_gradients.insert(gradientId, gradHelper);

    return &m_gradients[gradientId];
}

SvgGradientHelper* SvgParser::parseMeshGradient(const QDomElement &e)
{
    SvgGradientHelper gradHelper;
    QString gradientId = e.attribute("id");
    QScopedPointer<SvgMeshGradient> g(new SvgMeshGradient);

    // check if we have this gradient already parsed
    // copy existing gradient if it exists
    if (m_gradients.contains(gradientId)) {
        return &m_gradients[gradientId];
    }

    if (e.hasAttribute("xlink:href")) {
        // strip the '#' symbol
        QString href = e.attribute("xlink:href").mid(1);

        if (!href.isEmpty()) {
            // copy the referenced gradient if found
            SvgGradientHelper *pGrad = findGradient(href);
            if (pGrad) {
                gradHelper = *pGrad;
            }
        }
    }

    if (e.attribute("gradientUnits") == "userSpaceOnUse") {
        gradHelper.setGradientUnits(KoFlake::UserSpaceOnUse);
    }

    if (e.hasAttribute("transform")) {
        SvgTransformParser p(e.attribute("transform"));
        if (p.isValid()) {
            gradHelper.setTransform(p.transform());
        }
    }

    QString type = e.attribute("type");
    g->setType(SvgMeshGradient::BILINEAR);
    if (!type.isEmpty() && type == "bicubic") {
        g->setType(SvgMeshGradient::BICUBIC);
    }

    int irow = 0, icols;
    for (int i = 0; i < e.childNodes().size(); ++i) {
        QDomNode node = e.childNodes().at(i);

        if (node.nodeName() == "meshrow") {

            SvgMeshStop startingNode;
            if (irow == 0) {
                startingNode.point = QPointF(
                            parseUnitX(e.attribute("x")),
                            parseUnitY(e.attribute(("y"))));
                startingNode.color = QColor();
            }

            icols = 0;
            g->getMeshArray()->newRow();
            for (int j = 0; j < node.childNodes().size() ; ++j) {
                QDomNode meshpatchNode = node.childNodes().at(j);

                if (meshpatchNode.nodeName() == "meshpatch") {
                    if (irow > 0) {
                        // Starting point for this would be the bottom (right) corner of the above patch
                        startingNode = g->getMeshArray()->getStop(SvgMeshPatch::Bottom, irow - 1, icols);
                    } else if (icols != 0) {
                        // Starting point for this would be the right (top) corner of the previous patch
                        startingNode = g->getMeshArray()->getStop(SvgMeshPatch::Right, irow, icols - 1);
                    }

                    QList<QPair<QString, QColor>> rawStops = parseMeshPatch(meshpatchNode);
                    // TODO handle the false result
                    if (!g->getMeshArray()->addPatch(rawStops, startingNode.point)) {
                        debugFlake << "WARNING: Failed to create meshpatch";
                    }
                    icols++;
                }
            }
            irow++;
        }
    }
    gradHelper.setMeshGradient(g.data());
    m_gradients.insert(gradientId, gradHelper);

    return &m_gradients[gradientId];
}

QList<QPair<QString, QColor>> SvgParser::parseMeshPatch(const QDomNode& meshpatchNode)
{
    // path and its associated color
    QList<QPair<QString, QColor>> rawstops;

    SvgGraphicsContext *gc = m_context.currentGC();
    if (!gc) return rawstops;

    QDomElement e = meshpatchNode.toElement();

    QDomElement stop;
    forEachElement(stop, e) {
        qreal X;    // don't care..
        QColor color = m_context.styleParser().parseColorStop(stop, gc, X).second;

        QString pathStr = stop.attribute("path");

        rawstops.append({pathStr, color});
    }

    return rawstops;
}

inline QPointF bakeShapeOffset(const QTransform &patternTransform, const QPointF &shapeOffset)
{
    QTransform result =
            patternTransform *
            QTransform::fromTranslate(-shapeOffset.x(), -shapeOffset.y()) *
            patternTransform.inverted();
    KIS_ASSERT_RECOVER_NOOP(result.type() <= QTransform::TxTranslate);

    return QPointF(result.dx(), result.dy());
}

QSharedPointer<KoVectorPatternBackground> SvgParser::parsePattern(const QDomElement &e, const KoShape *shape)
{
    /**
     * Unlike the gradient parsing function, this method is called every time we
     * *reference* the pattern, not when we define it. Therefore we can already
     * use the coordinate system of the destination.
     */

    QSharedPointer<KoVectorPatternBackground> pattHelper;

    SvgGraphicsContext *gc = m_context.currentGC();
    if (!gc) return pattHelper;

    const QString patternId = e.attribute("id");
    if (patternId.isEmpty()) return pattHelper;

    pattHelper = toQShared(new KoVectorPatternBackground);

    if (e.hasAttribute("xlink:href")) {
        // strip the '#' symbol
        QString href = e.attribute("xlink:href").mid(1);

        if (!href.isEmpty() &&href != patternId) {
            // copy the referenced pattern if found
            QSharedPointer<KoVectorPatternBackground> pPatt = findPattern(href, shape);
            if (pPatt) {
                pattHelper = pPatt;
            }
        }
    }

    pattHelper->setReferenceCoordinates(
                KoFlake::coordinatesFromString(e.attribute("patternUnits"),
                                               pattHelper->referenceCoordinates()));

    pattHelper->setContentCoordinates(
                KoFlake::coordinatesFromString(e.attribute("patternContentUnits"),
                                               pattHelper->contentCoordinates()));

    if (e.hasAttribute("patternTransform")) {
        SvgTransformParser p(e.attribute("patternTransform"));
        if (p.isValid()) {
            pattHelper->setPatternTransform(p.transform());
        }
    }

    if (pattHelper->referenceCoordinates() == KoFlake::ObjectBoundingBox) {
        QRectF referenceRect(
            SvgUtil::fromPercentage(e.attribute("x", "0%")),
            SvgUtil::fromPercentage(e.attribute("y", "0%")),
            SvgUtil::fromPercentage(e.attribute("width", "0%")), // 0% is according to SVG 1.1, don't ask me why!
            SvgUtil::fromPercentage(e.attribute("height", "0%"))); // 0% is according to SVG 1.1, don't ask me why!

        pattHelper->setReferenceRect(referenceRect);
    } else {
        QRectF referenceRect(
            parseUnitX(e.attribute("x", "0")),
            parseUnitY(e.attribute("y", "0")),
            parseUnitX(e.attribute("width", "0")), // 0 is according to SVG 1.1, don't ask me why!
            parseUnitY(e.attribute("height", "0"))); // 0 is according to SVG 1.1, don't ask me why!

        pattHelper->setReferenceRect(referenceRect);
    }

    /**
     * In Krita shapes X,Y coordinates are baked into the shape global transform, but
     * the pattern should be painted in "user" coordinates. Therefore, we should handle
     * this offfset separately.
     *
     * TODO: Please also note that this offset is different from extraShapeOffset(),
     * because A.inverted() * B != A * B.inverted(). I'm not sure which variant is
     * correct (DK)
     */

   const QTransform dstShapeTransform = shape->absoluteTransformation();
   const QTransform shapeOffsetTransform = dstShapeTransform * gc->matrix.inverted();
   KIS_SAFE_ASSERT_RECOVER_NOOP(shapeOffsetTransform.type() <= QTransform::TxTranslate);
   const QPointF extraShapeOffset(shapeOffsetTransform.dx(), shapeOffsetTransform.dy());

   m_context.pushGraphicsContext(e);
   gc = m_context.currentGC();
   gc->workaroundClearInheritedFillProperties(); // HACK!

   // start building shape tree from scratch
   gc->matrix = QTransform();

   const QRectF boundingRect = shape->outline().boundingRect()/*.translated(extraShapeOffset)*/;
   const QTransform relativeToShape(boundingRect.width(), 0, 0, boundingRect.height(),
                                    boundingRect.x(), boundingRect.y());



   // WARNING1: OBB and ViewBox transformations are *baked* into the pattern shapes!
   //          although we expect the pattern be reusable, but it is not so!
   // WARNING2: the pattern shapes are stored in *User* coordinate system, although
   //           the "official" content system might be either OBB or User. It means that
   //           this baked transform should be stripped before writing the shapes back
   //           into SVG
   if (e.hasAttribute("viewBox")) {
        gc->currentBoundingBox =
            pattHelper->referenceCoordinates() == KoFlake::ObjectBoundingBox ?
            relativeToShape.mapRect(pattHelper->referenceRect()) :
            pattHelper->referenceRect();

        applyViewBoxTransform(e);
        pattHelper->setContentCoordinates(pattHelper->referenceCoordinates());

    } else if (pattHelper->contentCoordinates() == KoFlake::ObjectBoundingBox) {
        gc->matrix = relativeToShape * gc->matrix;
    }

    // We do *not* apply patternTransform here! Here we only bake the untransformed
    // version of the shape. The transformed one will be done in the very end while rendering.

    QList<KoShape*> patternShapes = parseContainer(e);

    if (pattHelper->contentCoordinates() == KoFlake::UserSpaceOnUse) {
        // In Krita we normalize the shapes, bake this transform into the pattern shapes

        const QPointF offset = bakeShapeOffset(pattHelper->patternTransform(), extraShapeOffset);

        Q_FOREACH (KoShape *shape, patternShapes) {
            shape->applyAbsoluteTransformation(QTransform::fromTranslate(offset.x(), offset.y()));
        }
    }

    if (pattHelper->referenceCoordinates() == KoFlake::UserSpaceOnUse) {
        // In Krita we normalize the shapes, bake this transform into reference rect
        // NOTE: this is possible *only* when pattern transform is not perspective
        //       (which is always true for SVG)

        const QPointF offset = bakeShapeOffset(pattHelper->patternTransform(), extraShapeOffset);

        QRectF ref = pattHelper->referenceRect();
        ref.translate(offset);
        pattHelper->setReferenceRect(ref);
    }

    m_context.popGraphicsContext();
    gc = m_context.currentGC();

    if (!patternShapes.isEmpty()) {
        pattHelper->setShapes(patternShapes);
    }

    return pattHelper;
}

bool SvgParser::parseFilter(const QDomElement &e, const QDomElement &referencedBy)
{
    SvgFilterHelper filter;

    // Use the filter that is referencing, or if there isn't one, the original filter
    QDomElement b;
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
        filter.setFilterUnits(KoFlake::UserSpaceOnUse);
    if (b.attribute("primitiveUnits") == "objectBoundingBox")
        filter.setPrimitiveUnits(KoFlake::ObjectBoundingBox);

    // parse filter region rectangle
    if (filter.filterUnits() == KoFlake::UserSpaceOnUse) {
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

bool SvgParser::parseMarker(const QDomElement &e)
{
    const QString id = e.attribute("id");
    if (id.isEmpty()) return false;

    QScopedPointer<KoMarker> marker(new KoMarker());
    marker->setCoordinateSystem(
        KoMarker::coordinateSystemFromString(e.attribute("markerUnits", "strokeWidth")));

    marker->setReferencePoint(QPointF(parseUnitX(e.attribute("refX")),
                                      parseUnitY(e.attribute("refY"))));

    marker->setReferenceSize(QSizeF(parseUnitX(e.attribute("markerWidth", "3")),
                                     parseUnitY(e.attribute("markerHeight", "3"))));

    const QString orientation = e.attribute("orient", "0");

    if (orientation == "auto") {
        marker->setAutoOrientation(true);
    } else {
        marker->setExplicitOrientation(parseAngular(orientation));
    }

    // ensure that the clip path is loaded in local coordinates system
    m_context.pushGraphicsContext(e, false);
    m_context.currentGC()->matrix = QTransform();
    m_context.currentGC()->currentBoundingBox = QRectF(QPointF(0, 0), marker->referenceSize());

    KoShape *markerShape = parseGroup(e);

    m_context.popGraphicsContext();

    if (!markerShape) return false;

    marker->setShapes({markerShape});

    m_markers.insert(id, QExplicitlySharedDataPointer<KoMarker>(marker.take()));

    return true;
}

bool SvgParser::parseSymbol(const QDomElement &e)
{
    const QString id = e.attribute("id");

    if (id.isEmpty()) return false;

    QScopedPointer<KoSvgSymbol> svgSymbol(new KoSvgSymbol());

    // ensure that the clip path is loaded in local coordinates system
    m_context.pushGraphicsContext(e, false);
    m_context.currentGC()->matrix = QTransform();
    m_context.currentGC()->currentBoundingBox = QRectF(0.0, 0.0, 1.0, 1.0);

    QString title = e.firstChildElement("title").toElement().text();

    QScopedPointer<KoShape> symbolShape(parseGroup(e));

    m_context.popGraphicsContext();

    if (!symbolShape) return false;

    svgSymbol->shape = symbolShape.take();
    svgSymbol->title = title;
    svgSymbol->id = id;
    if (title.isEmpty()) svgSymbol->title = id;

    if (svgSymbol->shape->boundingRect() == QRectF(0.0, 0.0, 0.0, 0.0)) {
        debugFlake << "Symbol" << id << "seems to be empty, discarding";
        return false;
    }

    // TODO: out default set of symbols had duplicated ids! We should
    //       make sure they are unique!
    if (m_symbols.contains(id)) {
        delete m_symbols[id];
        m_symbols.remove(id);
    }

    m_symbols.insert(id, svgSymbol.take());

    return true;
}

bool SvgParser::parseClipPath(const QDomElement &e)
{
    SvgClipPathHelper clipPath;

    const QString id = e.attribute("id");
    if (id.isEmpty()) return false;

    clipPath.setClipPathUnits(
                KoFlake::coordinatesFromString(e.attribute("clipPathUnits"), KoFlake::UserSpaceOnUse));

    // ensure that the clip path is loaded in local coordinates system
    m_context.pushGraphicsContext(e);
    m_context.currentGC()->matrix = QTransform();
    m_context.currentGC()->workaroundClearInheritedFillProperties(); // HACK!

    KoShape *clipShape = parseGroup(e);

    m_context.popGraphicsContext();

    if (!clipShape) return false;

    clipPath.setShapes({clipShape});
    m_clipPaths.insert(id, clipPath);

    return true;
}

bool SvgParser::parseClipMask(const QDomElement &e)
{
    QSharedPointer<KoClipMask> clipMask(new KoClipMask);

    const QString id = e.attribute("id");
    if (id.isEmpty()) return false;

    clipMask->setCoordinates(KoFlake::coordinatesFromString(e.attribute("maskUnits"), KoFlake::ObjectBoundingBox));
    clipMask->setContentCoordinates(KoFlake::coordinatesFromString(e.attribute("maskContentUnits"), KoFlake::UserSpaceOnUse));

    QRectF maskRect;

    if (clipMask->coordinates() == KoFlake::ObjectBoundingBox) {
        maskRect.setRect(
            SvgUtil::fromPercentage(e.attribute("x", "-10%")),
            SvgUtil::fromPercentage(e.attribute("y", "-10%")),
            SvgUtil::fromPercentage(e.attribute("width", "120%")),
            SvgUtil::fromPercentage(e.attribute("height", "120%")));
    } else {
        maskRect.setRect(
            parseUnitX(e.attribute("x", "-10%")), // yes, percents are insane in this case,
            parseUnitY(e.attribute("y", "-10%")), // but this is what SVG 1.1 tells us...
            parseUnitX(e.attribute("width", "120%")),
            parseUnitY(e.attribute("height", "120%")));
    }

    clipMask->setMaskRect(maskRect);


    // ensure that the clip mask is loaded in local coordinates system
    m_context.pushGraphicsContext(e);
    m_context.currentGC()->matrix = QTransform();
    m_context.currentGC()->workaroundClearInheritedFillProperties(); // HACK!

    KoShape *clipShape = parseGroup(e);

    m_context.popGraphicsContext();

    if (!clipShape) return false;
    clipMask->setShapes({clipShape});

    m_clipMasks.insert(id, clipMask);
    return true;
}

void SvgParser::uploadStyleToContext(const QDomElement &e)
{
    SvgStyles styles = m_context.styleParser().collectStyles(e);
    m_context.styleParser().parseFont(styles);
    m_context.styleParser().parseStyle(styles);
}

void SvgParser::applyCurrentStyle(KoShape *shape, const QPointF &shapeToOriginalUserCoordinates)
{
    if (!shape) return;

    applyCurrentBasicStyle(shape);

    if (KoPathShape *pathShape = dynamic_cast<KoPathShape*>(shape)) {
        applyMarkers(pathShape);
    }

    applyFilter(shape);
    applyClipping(shape, shapeToOriginalUserCoordinates);
    applyMaskClipping(shape, shapeToOriginalUserCoordinates);

}

void SvgParser::applyCurrentBasicStyle(KoShape *shape)
{
    if (!shape) return;

    SvgGraphicsContext *gc = m_context.currentGC();
    KIS_ASSERT(gc);

    if (!dynamic_cast<KoShapeGroup*>(shape)) {
        applyFillStyle(shape);
        applyStrokeStyle(shape);
    }

    if (!gc->display || !gc->visible) {
        /**
         * WARNING: here is a small inconsistency with the standard:
         *          in the standard, 'display' is not inherited, but in
         *          flake it is!
         *
         * NOTE: though the standard says: "A value of 'display:none' indicates
         *       that the given element and ***its children*** shall not be
         *       rendered directly". Therefore, using setVisible(false) is fully
         *       legitimate here (DK 29.11.16).
         */
        shape->setVisible(false);
    }
    shape->setTransparency(1.0 - gc->opacity);
}


void SvgParser::applyStyle(KoShape *obj, const QDomElement &e, const QPointF &shapeToOriginalUserCoordinates)
{
    applyStyle(obj, m_context.styleParser().collectStyles(e), shapeToOriginalUserCoordinates);
}

void SvgParser::applyStyle(KoShape *obj, const SvgStyles &styles, const QPointF &shapeToOriginalUserCoordinates)
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

    if (KoPathShape *pathShape = dynamic_cast<KoPathShape*>(obj)) {
        applyMarkers(pathShape);
    }

    applyFilter(obj);
    applyClipping(obj, shapeToOriginalUserCoordinates);
    applyMaskClipping(obj, shapeToOriginalUserCoordinates);

    if (!gc->display || !gc->visible) {
        obj->setVisible(false);
    }
    obj->setTransparency(1.0 - gc->opacity);
}

QGradient* prepareGradientForShape(const SvgGradientHelper *gradient,
                                   const KoShape *shape,
                                   const SvgGraphicsContext *gc,
                                   QTransform *transform)
{
    QGradient *resultGradient = 0;
    KIS_ASSERT(transform);

    if (gradient->gradientUnits() == KoFlake::ObjectBoundingBox) {
        resultGradient = KoFlake::cloneGradient(gradient->gradient());
        *transform = gradient->transform();
    } else {
        if (gradient->gradient()->type() == QGradient::LinearGradient) {
            /**
             * Create a converted gradient that looks the same, but linked to the
             * bounding rect of the shape, so it would be transformed with the shape
             */

            const QRectF boundingRect = shape->outline().boundingRect();
            const QTransform relativeToShape(boundingRect.width(), 0, 0, boundingRect.height(),
                                             boundingRect.x(), boundingRect.y());

            const QTransform relativeToUser =
                    relativeToShape * shape->transformation() * gc->matrix.inverted();

            const QTransform userToRelative = relativeToUser.inverted();

            const QLinearGradient *o = static_cast<const QLinearGradient*>(gradient->gradient());
            QLinearGradient *g = new QLinearGradient();
            g->setStart(userToRelative.map(o->start()));
            g->setFinalStop(userToRelative.map(o->finalStop()));
            g->setCoordinateMode(QGradient::ObjectBoundingMode);
            g->setStops(o->stops());
            g->setSpread(o->spread());

            resultGradient = g;
            *transform = relativeToUser * gradient->transform() * userToRelative;

        } else if (gradient->gradient()->type() == QGradient::RadialGradient) {
            // For radial and conical gradients such conversion is not possible

            resultGradient = KoFlake::cloneGradient(gradient->gradient());
            *transform = gradient->transform() * gc->matrix * shape->transformation().inverted();

            const QRectF outlineRect = shape->outlineRect();
            if (outlineRect.isEmpty()) return resultGradient;

            /**
             * If shape outline rect is valid, convert the gradient into OBB mode by
             * doing some magic conversions: we compensate non-uniform size of the shape
             * by applying an additional pre-transform
             */

            QRadialGradient *rgradient = static_cast<QRadialGradient*>(resultGradient);

            const qreal maxDimension = KisAlgebra2D::maxDimension(outlineRect);
            const QRectF uniformSize(outlineRect.topLeft(), QSizeF(maxDimension, maxDimension));

            const QTransform uniformizeTransform =
                    QTransform::fromTranslate(-outlineRect.x(), -outlineRect.y()) *
                    QTransform::fromScale(maxDimension / shape->outlineRect().width(),
                                          maxDimension / shape->outlineRect().height()) *
                    QTransform::fromTranslate(outlineRect.x(), outlineRect.y());

            const QPointF centerLocal = transform->map(rgradient->center());
            const QPointF focalLocal = transform->map(rgradient->focalPoint());

            const QPointF centerOBB = KisAlgebra2D::absoluteToRelative(centerLocal, uniformSize);
            const QPointF focalOBB = KisAlgebra2D::absoluteToRelative(focalLocal, uniformSize);

            rgradient->setCenter(centerOBB);
            rgradient->setFocalPoint(focalOBB);

            const qreal centerRadiusOBB = KisAlgebra2D::absoluteToRelative(rgradient->centerRadius(), uniformSize);
            const qreal focalRadiusOBB = KisAlgebra2D::absoluteToRelative(rgradient->focalRadius(), uniformSize);

            rgradient->setCenterRadius(centerRadiusOBB);
            rgradient->setFocalRadius(focalRadiusOBB);

            rgradient->setCoordinateMode(QGradient::ObjectBoundingMode);

            // Warning: should it really be pre-multiplication?
            *transform = uniformizeTransform * gradient->transform();
        }
    }

    // NOTE: this is an internal API in Qt. SVG specs specify that we
    // shouldn't interpolate in pre-multiplied space.
    resultGradient->setInterpolationMode(QGradient::ComponentInterpolation);

    return resultGradient;
}

SvgMeshGradient* prepareMeshGradientForShape(SvgGradientHelper *gradient,
                                             const KoShape *shape,
                                             const SvgGraphicsContext *gc) {

    SvgMeshGradient *resultGradient = nullptr;

    if (gradient->gradientUnits() == KoFlake::ObjectBoundingBox) {

        resultGradient = new SvgMeshGradient(*gradient->meshgradient());

        const QRectF boundingRect = shape->outline().boundingRect();
        const QTransform relativeToShape(boundingRect.width(), 0, 0, boundingRect.height(),
                                         boundingRect.x(), boundingRect.y());

        // NOTE: we apply translation right away, because caching hasn't been implemented for rendering, yet.
        // So, transform is called multiple times on the mesh and that's not nice
        resultGradient->setTransform(gradient->transform() * relativeToShape);
    } else {
        // NOTE: Krita's shapes use their own coordinate system. Where origin is at the top left
        // of the SHAPE. All the mesh patches will be rendered in the global 'user' coorindate system
        // where the origin is at the top left of the LAYER/DOCUMENT.

        // Get the user coordinates of the shape
        const QTransform shapeglobal = shape->absoluteTransformation() * gc->matrix.inverted();

        // Get the translation offset to shift the origin from "Shape" to "User"
        const QTransform translationOffset = QTransform::fromTranslate(-shapeglobal.dx(), -shapeglobal.dy());

        resultGradient = new SvgMeshGradient(*gradient->meshgradient());

        // NOTE: we apply translation right away, because caching hasn't been implemented for rendering, yet.
        // So, transform is called multiple times on the mesh and that's not nice
        resultGradient->setTransform(gradient->transform() * translationOffset);
    }

    return resultGradient;
}

void SvgParser::applyFillStyle(KoShape *shape)
{
    SvgGraphicsContext *gc = m_context.currentGC();
    if (! gc)
        return;

    if (gc->fillType == SvgGraphicsContext::None) {
        shape->setBackground(QSharedPointer<KoShapeBackground>(0));
    } else if (gc->fillType == SvgGraphicsContext::Solid) {
        shape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(gc->fillColor)));
    } else if (gc->fillType == SvgGraphicsContext::Complex) {
        // try to find referenced gradient
        SvgGradientHelper *gradient = findGradient(gc->fillId);
        if (gradient) {
            QTransform transform;

            if (gradient->isMeshGradient()) {
                QSharedPointer<KoMeshGradientBackground> bg;

                QScopedPointer<SvgMeshGradient> result(prepareMeshGradientForShape(gradient, shape, gc));

                bg = toQShared(new KoMeshGradientBackground(result.data(), transform));
                shape->setBackground(bg);
            } else if (gradient->gradient()) {
                QGradient *result = prepareGradientForShape(gradient, shape, gc, &transform);
                if (result) {
                    QSharedPointer<KoGradientBackground> bg;
                    bg = toQShared(new KoGradientBackground(result));
                    bg->setTransform(transform);
                    shape->setBackground(bg);
                }
            }
        } else {
            QSharedPointer<KoVectorPatternBackground> pattern =
                findPattern(gc->fillId, shape);

            if (pattern) {
                shape->setBackground(pattern);
            } else {
                // no referenced fill found, use fallback color
                shape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(gc->fillColor)));
            }
        }
    }

    KoPathShape *path = dynamic_cast<KoPathShape*>(shape);
    if (path)
        path->setFillRule(gc->fillRule);
}

void applyDashes(const KoShapeStrokeSP srcStroke, KoShapeStrokeSP dstStroke)
{
    const double lineWidth = srcStroke->lineWidth();
    QVector<qreal> dashes = srcStroke->lineDashes();

    // apply line width to dashes and dash offset
    if (dashes.count() && lineWidth > 0.0) {
        const double dashOffset = srcStroke->dashOffset();
        QVector<qreal> dashes = srcStroke->lineDashes();

        for (int i = 0; i < dashes.count(); ++i) {
            dashes[i] /= lineWidth;
        }

        dstStroke->setLineStyle(Qt::CustomDashLine, dashes);
        dstStroke->setDashOffset(dashOffset / lineWidth);
    } else {
        dstStroke->setLineStyle(Qt::SolidLine, QVector<qreal>());
    }
}

void SvgParser::applyStrokeStyle(KoShape *shape)
{
    SvgGraphicsContext *gc = m_context.currentGC();
    if (! gc)
        return;

    if (gc->strokeType == SvgGraphicsContext::None) {
        KoShapeStrokeSP stroke(new KoShapeStroke());
        stroke->setLineWidth(0.0);
        const QColor color = Qt::transparent;
        stroke->setColor(color);
        shape->setStroke(stroke);
    } else if (gc->strokeType == SvgGraphicsContext::Solid) {
        KoShapeStrokeSP stroke(new KoShapeStroke(*gc->stroke));
        applyDashes(gc->stroke, stroke);
        shape->setStroke(stroke);
    } else if (gc->strokeType == SvgGraphicsContext::Complex) {
        // try to find referenced gradient
        SvgGradientHelper *gradient = findGradient(gc->strokeId);
        if (gradient) {
            QTransform transform;
            QGradient *result = prepareGradientForShape(gradient, shape, gc, &transform);
            if (result) {
                QBrush brush = *result;
                delete result;
                brush.setTransform(transform);

                KoShapeStrokeSP stroke(new KoShapeStroke(*gc->stroke));
                stroke->setLineBrush(brush);
                applyDashes(gc->stroke, stroke);
                shape->setStroke(stroke);
            }
        } else {
            // no referenced stroke found, use fallback color
            KoShapeStrokeSP stroke(new KoShapeStroke(*gc->stroke));
            applyDashes(gc->stroke, stroke);
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

    QDomElement content = filter->content();

    // parse filter region
    QRectF bound(shape->position(), shape->size());
    // work on bounding box without viewbox transformation applied
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
    context.enableFilterUnitsConversion(filter->filterUnits() == KoFlake::UserSpaceOnUse);
    context.enableFilterPrimitiveUnitsConversion(filter->primitiveUnits() == KoFlake::UserSpaceOnUse);

    KoFilterEffectRegistry *registry = KoFilterEffectRegistry::instance();

    KoFilterEffectStack *filterStack = 0;

    QSet<QString> stdInputs;
    stdInputs << "SourceGraphic" << "SourceAlpha";
    stdInputs << "BackgroundImage" << "BackgroundAlpha";
    stdInputs << "FillPaint" << "StrokePaint";

    QMap<QString, KoFilterEffect*> inputs;

    // create the filter effects and add them to the shape
    for (QDomNode n = content.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement primitive = n.toElement();
        KoFilterEffect *filterEffect = registry->createFilterEffectFromXml(primitive, context);
        if (!filterEffect) {
            debugFlake << "filter effect" << primitive.tagName() << "is not implemented yet";
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
        if (filter->primitiveUnits() == KoFlake::UserSpaceOnUse) {
            const QString xa = primitive.attribute("x");
            const QString ya = primitive.attribute("y");
            const QString wa = primitive.attribute("width");
            const QString ha = primitive.attribute("height");

            if (xa.isEmpty() || ya.isEmpty() || wa.isEmpty() || ha.isEmpty()) {
                bool hasStdInput = false;
                bool isFirstEffect = filterStack == 0;
                // check if one of the inputs is a standard input
                Q_FOREACH (const QString &input, filterEffect->inputs()) {
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
                    Q_FOREACH (const QString &input, filterEffect->inputs()) {
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

void SvgParser::applyMarkers(KoPathShape *shape)
{
    SvgGraphicsContext *gc = m_context.currentGC();
    if (!gc)
        return;

    if (!gc->markerStartId.isEmpty() && m_markers.contains(gc->markerStartId)) {
        shape->setMarker(m_markers[gc->markerStartId].data(), KoFlake::StartMarker);
    }

    if (!gc->markerMidId.isEmpty() && m_markers.contains(gc->markerMidId)) {
        shape->setMarker(m_markers[gc->markerMidId].data(), KoFlake::MidMarker);
    }

    if (!gc->markerEndId.isEmpty() && m_markers.contains(gc->markerEndId)) {
        shape->setMarker(m_markers[gc->markerEndId].data(), KoFlake::EndMarker);
    }

    shape->setAutoFillMarkers(gc->autoFillMarkers);
}

void SvgParser::applyClipping(KoShape *shape, const QPointF &shapeToOriginalUserCoordinates)
{
    SvgGraphicsContext *gc = m_context.currentGC();
    if (! gc)
        return;

    if (gc->clipPathId.isEmpty())
        return;

    SvgClipPathHelper *clipPath = findClipPath(gc->clipPathId);
    if (!clipPath || clipPath->isEmpty())
        return;

    QList<KoShape*> shapes;

    Q_FOREACH (KoShape *item, clipPath->shapes()) {
        KoShape *clonedShape = item->cloneShape();
        KIS_ASSERT_RECOVER(clonedShape) { continue; }

        shapes.append(clonedShape);
    }

    if (!shapeToOriginalUserCoordinates.isNull()) {
        const QTransform t =
            QTransform::fromTranslate(shapeToOriginalUserCoordinates.x(),
                                      shapeToOriginalUserCoordinates.y());

        Q_FOREACH(KoShape *s, shapes) {
            s->applyAbsoluteTransformation(t);
        }
    }

    KoClipPath *clipPathObject = new KoClipPath(shapes,
                                                clipPath->clipPathUnits() == KoFlake::ObjectBoundingBox ?
                                                KoFlake::ObjectBoundingBox : KoFlake::UserSpaceOnUse);
    shape->setClipPath(clipPathObject);
}

void SvgParser::applyMaskClipping(KoShape *shape, const QPointF &shapeToOriginalUserCoordinates)
{
    SvgGraphicsContext *gc = m_context.currentGC();
    if (!gc)
        return;

    if (gc->clipMaskId.isEmpty())
        return;


    QSharedPointer<KoClipMask> originalClipMask = m_clipMasks.value(gc->clipMaskId);
    if (!originalClipMask || originalClipMask->isEmpty()) return;

    KoClipMask *clipMask = originalClipMask->clone();

    clipMask->setExtraShapeOffset(shapeToOriginalUserCoordinates);

    shape->setClipMask(clipMask);
}

KoShape* SvgParser::parseUse(const QDomElement &e, DeferredUseStore* deferredUseStore)
{
    QString href = e.attribute("xlink:href");
    if (href.isEmpty())
        return 0;

    QString key = href.mid(1);
    const bool gotDef = m_context.hasDefinition(key);
    if (gotDef) {
        return resolveUse(e, key);
    } else if (deferredUseStore) {
        deferredUseStore->add(&e, key);
        return 0;
    }
    debugFlake << "WARNING: Did not find reference for svg 'use' element. Skipping. Id: "
             << key;
    return 0;
}

KoShape* SvgParser::resolveUse(const QDomElement &e, const QString& key)
{
    KoShape *result = 0;

    SvgGraphicsContext *gc = m_context.pushGraphicsContext(e);

    // TODO: parse 'width' and 'height' as well
    gc->matrix.translate(parseUnitX(e.attribute("x", "0")), parseUnitY(e.attribute("y", "0")));

    const QDomElement &referencedElement = m_context.definition(key);
    result = parseGroup(e, referencedElement, false);

    m_context.popGraphicsContext();
    return result;
}

void SvgParser::addToGroup(QList<KoShape*> shapes, KoShapeContainer *group)
{
    m_shapes += shapes;

    if (!group || shapes.isEmpty())
        return;

    // not normalized
    KoShapeGroupCommand cmd(group, shapes, false);
    cmd.redo();
}

QList<KoShape*> SvgParser::parseSvg(const QDomElement &e, QSizeF *fragmentSize)
{
    // check if we are the root svg element
    const bool isRootSvg = m_context.isRootContext();

    // parse 'transform' field if preset
    SvgGraphicsContext *gc = m_context.pushGraphicsContext(e);

    applyStyle(0, e, QPointF());

    const QString w = e.attribute("width");
    const QString h = e.attribute("height");

    qreal width = w.isEmpty() ? 666.0 : parseUnitX(w);
    qreal height = h.isEmpty() ? 555.0 : parseUnitY(h);

    if (w.isEmpty() || h.isEmpty()) {
        QRectF viewRect;
        QTransform viewTransform_unused;
        QRectF fakeBoundingRect(0.0, 0.0, 1.0, 1.0);

        if (SvgUtil::parseViewBox(e, fakeBoundingRect,
                                  &viewRect, &viewTransform_unused)) {

            QSizeF estimatedSize = viewRect.size();

            if (estimatedSize.isValid()) {

                if (!w.isEmpty()) {
                    estimatedSize = QSizeF(width, width * estimatedSize.height() / estimatedSize.width());
                } else if (!h.isEmpty()) {
                    estimatedSize = QSizeF(height * estimatedSize.width() / estimatedSize.height(), height);
                }

                width = estimatedSize.width();
                height = estimatedSize.height();
            }
        }
    }

    QSizeF svgFragmentSize(QSizeF(width, height));

    if (fragmentSize) {
        *fragmentSize = svgFragmentSize;
    }

    gc->currentBoundingBox = QRectF(QPointF(0, 0), svgFragmentSize);

    if (!isRootSvg) {
        // x and y attribute has no meaning for outermost svg elements
        const qreal x = parseUnit(e.attribute("x", "0"));
        const qreal y = parseUnit(e.attribute("y", "0"));

        QTransform move = QTransform::fromTranslate(x, y);
        gc->matrix = move * gc->matrix;
    }

    /**
     * In internal SVG coordinate systems pixles are linked to absolute
     * values with a fixed ratio.
     *
     * See CSS specification:
     * https://www.w3.org/TR/css-values-3/#absolute-lengths
     */
    gc->pixelsPerInch = 96.0;

    applyViewBoxTransform(e);

    QList<KoShape*> shapes;

    // First find the metadata
    for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement b = n.toElement();
        if (b.isNull())
            continue;

        if (b.tagName() == "title") {
            m_documentTitle = b.text().trimmed();
        }
        else if (b.tagName() == "desc") {
            m_documentDescription = b.text().trimmed();
        }
        else if (b.tagName() == "metadata") {
            // TODO: parse the metadata
        }
    }


    // SVG 1.1: skip the rendering of the element if it has null viewBox; however an inverted viewbox is just peachy
    // and as mother makes them -- if mother is inkscape.
    if (gc->currentBoundingBox.normalized().isValid()) {
        shapes = parseContainer(e);
    }

    m_context.popGraphicsContext();

    return shapes;
}

void SvgParser::applyViewBoxTransform(const QDomElement &element)
{
    SvgGraphicsContext *gc = m_context.currentGC();

    QRectF viewRect = gc->currentBoundingBox;
    QTransform viewTransform;

    if (SvgUtil::parseViewBox(element, gc->currentBoundingBox,
                              &viewRect, &viewTransform)) {

        gc->matrix = viewTransform * gc->matrix;
        gc->currentBoundingBox = viewRect;
    }
}

QList<QExplicitlySharedDataPointer<KoMarker> > SvgParser::knownMarkers() const
{
    return m_markers.values();
}

QString SvgParser::documentTitle() const
{
    return m_documentTitle;
}

QString SvgParser::documentDescription() const
{
    return m_documentDescription;
}

void SvgParser::setFileFetcher(SvgParser::FileFetcherFunc func)
{
    m_context.setFileFetcher(func);
}

inline QPointF extraShapeOffset(const KoShape *shape, const QTransform coordinateSystemOnLoading)
{
    const QTransform shapeToOriginalUserCoordinates =
        shape->absoluteTransformation().inverted() *
        coordinateSystemOnLoading;

    KIS_SAFE_ASSERT_RECOVER_NOOP(shapeToOriginalUserCoordinates.type() <= QTransform::TxTranslate);
    return QPointF(shapeToOriginalUserCoordinates.dx(), shapeToOriginalUserCoordinates.dy());
}

KoShape* SvgParser::parseGroup(const QDomElement &b, const QDomElement &overrideChildrenFrom, bool createContext)
{
    if (createContext) {
        m_context.pushGraphicsContext(b);
    }

    KoShapeGroup *group = new KoShapeGroup();
    group->setZIndex(m_context.nextZIndex());

    // groups should also have their own coordinate system!
    group->applyAbsoluteTransformation(m_context.currentGC()->matrix);
    const QPointF extraOffset = extraShapeOffset(group, m_context.currentGC()->matrix);

    uploadStyleToContext(b);

    QList<KoShape*> childShapes;

    if (!overrideChildrenFrom.isNull()) {
        // we upload styles from both: <use> and <defs>
        uploadStyleToContext(overrideChildrenFrom);
        childShapes = parseSingleElement(overrideChildrenFrom, 0);
    } else {
        childShapes = parseContainer(b);
    }

    // handle id
    applyId(b.attribute("id"), group);

    addToGroup(childShapes, group);

    applyCurrentStyle(group, extraOffset); // apply style to this group after size is set

    if (createContext) {
        m_context.popGraphicsContext();
    }

    return group;
}

KoShape* SvgParser::parseTextNode(const QDomText &e)
{
    QScopedPointer<KoSvgTextChunkShape> textChunk(new KoSvgTextChunkShape());
    textChunk->setZIndex(m_context.nextZIndex());

    if (!textChunk->loadSvgTextNode(e, m_context)) {
        return 0;
    }

    textChunk->applyAbsoluteTransformation(m_context.currentGC()->matrix);
    applyCurrentBasicStyle(textChunk.data()); // apply style to this group after size is set

    return textChunk.take();
}

QDomText getTheOnlyTextChild(const QDomElement &e)
{
    QDomNode firstChild = e.firstChild();
    return !firstChild.isNull() && firstChild == e.lastChild() && firstChild.isText() ?
                firstChild.toText() : QDomText();
}

KoShape *SvgParser::parseTextElement(const QDomElement &e, KoSvgTextShape *mergeIntoShape)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(e.tagName() == "text" || e.tagName() == "tspan", 0);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_isInsideTextSubtree || e.tagName() == "text", 0);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(e.tagName() == "text" || !mergeIntoShape, 0);

    KoSvgTextShape *rootTextShape  = 0;

    if (e.tagName() == "text") {
        // XXX: Shapes need to be created by their factories
        if (mergeIntoShape) {
            rootTextShape = mergeIntoShape;
        } else {
            rootTextShape = new KoSvgTextShape();
            const QString useRichText = e.attribute("krita:useRichText", "true");
            rootTextShape->setRichTextPreferred(useRichText != "false");
        }
    }

    if (rootTextShape) {
        m_isInsideTextSubtree = true;
    }

    m_context.pushGraphicsContext(e);
    uploadStyleToContext(e);

    if (e.hasAttribute("krita:textVersion")) {
        m_context.currentGC()->textProperties.setProperty(KoSvgTextProperties::KraTextVersionId, e.attribute("krita:textVersion", "1").toInt());

        if (m_isInsideTextSubtree) {
            debugFlake << "WARNING: \"krita:textVersion\" attribute appeared in non-root text shape";
        }
    }

    KoSvgTextChunkShape *textChunk = rootTextShape ? rootTextShape : new KoSvgTextChunkShape();

    if (!mergeIntoShape) {
        textChunk->setZIndex(m_context.nextZIndex());
    }

    textChunk->loadSvg(e, m_context);

    // 1) apply transformation only in case we are not overriding the shape!
    // 2) the transformation should be applied *before* the shape is added to the group!
    if (!mergeIntoShape) {
        // groups should also have their own coordinate system!
        textChunk->applyAbsoluteTransformation(m_context.currentGC()->matrix);
        const QPointF extraOffset = extraShapeOffset(textChunk, m_context.currentGC()->matrix);

        // handle id
        applyId(e.attribute("id"), textChunk);
        applyCurrentStyle(textChunk, extraOffset); // apply style to this group after size is set
    } else {
        m_context.currentGC()->matrix = mergeIntoShape->absoluteTransformation();
        applyCurrentBasicStyle(textChunk);
    }

    QDomText onlyTextChild = getTheOnlyTextChild(e);
    if (!onlyTextChild.isNull()) {
        textChunk->loadSvgTextNode(onlyTextChild, m_context);
    } else {
        QList<KoShape*> childShapes = parseContainer(e, true);
        addToGroup(childShapes, textChunk);
    }

    m_context.popGraphicsContext();

    textChunk->normalizeCharTransformations();

    if (rootTextShape) {
        textChunk->simplifyFillStrokeInheritance();

        m_isInsideTextSubtree = false;
        rootTextShape->relayout();
    }

    return textChunk;
}

QList<KoShape*> SvgParser::parseContainer(const QDomElement &e, bool parseTextNodes)
{
    QList<KoShape*> shapes;

    // are we parsing a switch container
    bool isSwitch = e.tagName() == "switch";

    DeferredUseStore deferredUseStore(this);

    for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement b = n.toElement();
        if (b.isNull()) {
            if (parseTextNodes && n.isText()) {
                KoShape *shape = parseTextNode(n.toText());

                if (shape) {
                    shapes += shape;
                }
            }

            continue;
        }

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
                // not implemented yet
            }
        }

        QList<KoShape*> currentShapes = parseSingleElement(b, &deferredUseStore);
        shapes.append(currentShapes);

        // if we are parsing a switch, stop after the first supported element
        if (isSwitch && !currentShapes.isEmpty())
            break;
    }
    return shapes;
}

void SvgParser::parseDefsElement(const QDomElement &e)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(e.tagName() == "defs");
    parseSingleElement(e);
}

QList<KoShape*> SvgParser::parseSingleElement(const QDomElement &b, DeferredUseStore* deferredUseStore)
{
    QList<KoShape*> shapes;

    // save definition for later instantiation with 'use'
    m_context.addDefinition(b);
    if (deferredUseStore) {
        deferredUseStore->checkPendingUse(b, shapes);
    }

    if (b.tagName() == "svg") {
        shapes += parseSvg(b);
    } else if (b.tagName() == "g" || b.tagName() == "a" || b.tagName() == "symbol") {
        // treat svg link <a> as group so we don't miss its child elements
        shapes += parseGroup(b);

        if (b.tagName() == "symbol") {
            parseSymbol(b);
        }

    } else if (b.tagName() == "switch") {
        m_context.pushGraphicsContext(b);
        shapes += parseContainer(b);
        m_context.popGraphicsContext();
    } else if (b.tagName() == "defs") {
        if (KoXml::childNodesCount(b) > 0) {
            /**
             * WARNING: 'defs' are basically 'display:none' style, therefore they should not play
             *          any role in shapes outline calculation. But setVisible(false) shapes do!
             *          Should be fixed in the future!
             */
            KoShape *defsShape = parseGroup(b);
            defsShape->setVisible(false);
            m_defsShapes << defsShape; // TODO: where to delete the shape!?

        }
    } else if (b.tagName() == "linearGradient" || b.tagName() == "radialGradient") {
    } else if (b.tagName() == "pattern") {
    } else if (b.tagName() == "filter") {
        parseFilter(b);
    } else if (b.tagName() == "clipPath") {
        parseClipPath(b);
    } else if (b.tagName() == "mask") {
        parseClipMask(b);
    } else if (b.tagName() == "marker") {
        parseMarker(b);
    } else if (b.tagName() == "style") {
        m_context.addStyleSheet(b);
    } else if (b.tagName() == "text" ||
               b.tagName() == "tspan") {

        shapes += parseTextElement(b);
    } else if (b.tagName() == "rect" ||
               b.tagName() == "ellipse" ||
               b.tagName() == "circle" ||
               b.tagName() == "line" ||
               b.tagName() == "polyline" ||
               b.tagName() == "polygon" ||
               b.tagName() == "path" ||
               b.tagName() == "image") {
        KoShape *shape = createObjectDirect(b);

        if (shape) {
            if (!shape->outlineRect().isNull() || !shape->boundingRect().isNull()) {
                shapes.append(shape);
            } else {
                debugFlake << "WARNING: shape is totally empty!" << shape->shapeId() << ppVar(shape->outlineRect());
                debugFlake << "    " << shape->shapeId() << ppVar(shape->outline());
                {
                    QString string;
                    QTextStream stream(&string);
                    stream << b;
                    debugFlake << "    " << string;
                }
                delete shape;
            }
        }
    } else if (b.tagName() == "use") {
        KoShape* s = parseUse(b, deferredUseStore);
        if (s) {
            shapes += s;
        }
    } else if (b.tagName() == "color-profile") {
        m_context.parseProfile(b);
    } else {
        // this is an unknown element, so try to load it anyway
        // there might be a shape that handles that element
        KoShape *shape = createObject(b);
        if (shape) {
            shapes.append(shape);
        }
    }

    return shapes;
}

// Creating functions
// ---------------------------------------------------------------------------------------

KoShape * SvgParser::createPath(const QDomElement &element)
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
            QStringList pointList = SvgUtil::simplifyList(element.attribute("points"));
            for (QStringList::Iterator it = pointList.begin(); it != pointList.end(); ++it) {
                QPointF point;
                point.setX(SvgUtil::fromUserSpace(KisDomUtils::toDouble(*it)));
                ++it;
                if (it == pointList.end())
                    break;
                point.setY(SvgUtil::fromUserSpace(KisDomUtils::toDouble(*it)));
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

            if (element.hasAttribute("sodipodi:nodetypes")) {
                path->loadNodeTypes(element.attribute("sodipodi:nodetypes"));
            }
            obj = path;
        }
    }

    return obj;
}

KoShape * SvgParser::createObjectDirect(const QDomElement &b)
{
    m_context.pushGraphicsContext(b);
    uploadStyleToContext(b);

    KoShape *obj = createShapeFromElement(b, m_context);
    if (obj) {
        obj->applyAbsoluteTransformation(m_context.currentGC()->matrix);
        const QPointF extraOffset = extraShapeOffset(obj, m_context.currentGC()->matrix);

        applyCurrentStyle(obj, extraOffset);

        // handle id
        applyId(b.attribute("id"), obj);
        obj->setZIndex(m_context.nextZIndex());
    }

    m_context.popGraphicsContext();

    return obj;
}

KoShape * SvgParser::createObject(const QDomElement &b, const SvgStyles &style)
{
    m_context.pushGraphicsContext(b);

    KoShape *obj = createShapeFromElement(b, m_context);
    if (obj) {
        obj->applyAbsoluteTransformation(m_context.currentGC()->matrix);
        const QPointF extraOffset = extraShapeOffset(obj, m_context.currentGC()->matrix);

        SvgStyles objStyle = style.isEmpty() ? m_context.styleParser().collectStyles(b) : style;
        m_context.styleParser().parseFont(objStyle);
        applyStyle(obj, objStyle, extraOffset);

        // handle id
        applyId(b.attribute("id"), obj);
        obj->setZIndex(m_context.nextZIndex());
    }

    m_context.popGraphicsContext();

    return obj;
}

KoShape * SvgParser::createShapeFromElement(const QDomElement &element, SvgLoadingContext &context)
{
    KoShape *object = 0;


    const QString tagName = SvgUtil::mapExtendedShapeTag(element.tagName(), element);
    QList<KoShapeFactoryBase*> factories = KoShapeRegistry::instance()->factoriesForElement(KoXmlNS::svg, tagName);

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
        KoShapeStrokeModelSP oldStroke = shape->stroke();
        shape->setStroke(KoShapeStrokeModelSP());

        // reset fill
        shape->setBackground(QSharedPointer<KoShapeBackground>(0));

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

KoShape *SvgParser::createShape(const QString &shapeID)
{
    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->get(shapeID);
    if (!factory) {
        debugFlake << "Could not find factory for shape id" << shapeID;
        return 0;
    }

    KoShape *shape = factory->createDefaultShape(m_documentResourceManager);
    if (!shape) {
        debugFlake << "Could not create Default shape for shape id" << shapeID;
        return 0;
    }
    if (shape->shapeId().isEmpty()) {
        shape->setShapeId(factory->id());
    }

    // reset transformation that might come from the default shape
    shape->setTransformation(QTransform());

    // reset border
    // ??? KoShapeStrokeModelSP oldStroke = shape->stroke();
    shape->setStroke(KoShapeStrokeModelSP());

    // reset fill
    shape->setBackground(QSharedPointer<KoShapeBackground>(0));

    return shape;
}

void SvgParser::applyId(const QString &id, KoShape *shape)
{
    if (id.isEmpty())
        return;

    shape->setName(id);
    m_context.registerShape(id, shape);
}
