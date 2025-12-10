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
#include <QDir>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>

#include <KoShape.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactoryBase.h>
#include <KoShapeGroup.h>
#include <KoPathShape.h>
#include <KoDocumentResourceManager.h>
#include <KoPathShapeLoader.h>
#include <commands/KoShapeGroupCommand.h>
#include <commands/KoShapeUngroupCommand.h>
#include <KoColorBackground.h>
#include <KoGradientBackground.h>
#include <KoMeshGradientBackground.h>
#include <KoPatternBackground.h>
#include <KoClipPath.h>
#include <KoClipMask.h>
#include <KoXmlNS.h>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QXmlSimpleReader>
#include <QXmlInputSource>
#endif

#include "SvgMeshGradient.h"
#include "SvgMeshPatch.h"
#include "SvgUtil.h"
#include "SvgShape.h"
#include "SvgGraphicContext.h"
#include "SvgGradientHelper.h"
#include "SvgClipPathHelper.h"
#include "parsers/SvgTransformParser.h"
#include "kis_pointer_utils.h"
#include <KoVectorPatternBackground.h>
#include <KoMarker.h>

#include <text/KoSvgTextShape.h>
#include <text/KoSvgTextLoader.h>

#include "kis_dom_utils.h"

#include "kis_algebra_2d.h"
#include "kis_debug.h"
#include "kis_global.h"
#include <QXmlStreamReader>
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

/*
 * Qt 5.15 deprecated this way of setting the document content, however,
 * they forgot to address reading text nodes with only white spaces, which
 * results in bugs for SVG text parsing.
 *
 * See bug 513085
 *
 */
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
QDomDocument createDocumentFromXmlInputSource(QXmlInputSource *source, QString *errorMsg, int *errorLine, int *errorColumn) {
    QDomDocument doc;
    QXmlSimpleReader simpleReader;
    simpleReader.setFeature("http://qt-project.org/xml/features/report-whitespace-only-CharData", true);
    simpleReader.setFeature("http://xml.org/sax/features/namespaces", false);
    simpleReader.setFeature("http://xml.org/sax/features/namespace-prefixes", true);
    if (!doc.setContent(source, &simpleReader, errorMsg, errorLine, errorColumn)) {
        return {};
    }
    return doc;
}
#endif

QDomDocument SvgParser::createDocumentFromSvg(QIODevice *device, QString *errorMsg, int *errorLine, int *errorColumn)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QXmlInputSource source(device);
    return createDocumentFromXmlInputSource(&source, errorMsg, errorLine, errorColumn);
#else
    return createDocumentFromSvg(QXmlStreamReader(device), errorMsg, errorLine, errorColumn);
#endif
}

QDomDocument SvgParser::createDocumentFromSvg(const QByteArray &data, QString *errorMsg, int *errorLine, int *errorColumn)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QXmlInputSource source;
    source.setData(data);
    return createDocumentFromXmlInputSource(&source, errorMsg, errorLine, errorColumn);
#else
    return createDocumentFromSvg(QXmlStreamReader(data), errorMsg, errorLine, errorColumn);
#endif
}

QDomDocument SvgParser::createDocumentFromSvg(const QString &data, QString *errorMsg, int *errorLine, int *errorColumn)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QXmlInputSource source;
    source.setData(data);
    return createDocumentFromXmlInputSource(&source, errorMsg, errorLine, errorColumn);
#else
    return createDocumentFromSvg(QXmlStreamReader(data), errorMsg, errorLine, errorColumn);
#endif
}

QDomDocument SvgParser::createDocumentFromSvg(QXmlStreamReader reader, QString *errorMsg, int *errorLine, int *errorColumn)
{
    QDomDocument doc;

    reader.setNamespaceProcessing(false);
    if (!doc.setContent(&reader, false, errorMsg, errorLine, errorColumn)) {
        return {};
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

void SvgParser::setFillStrokeInheritByDefault(const bool enable)
{
    m_inheritStrokeFillByDefault = enable;
}

void SvgParser::setResolveTextPropertiesForTopLevel(const bool enable)
{
    m_resolveTextPropertiesForTopLevel = enable;
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

SvgClipPathHelper* SvgParser::findClipPath(const QString &id)
{
    return m_clipPaths.contains(id) ? &m_clipPaths[id] : 0;
}

// Parsing functions
// ---------------------------------------------------------------------------------------

qreal SvgParser::parseUnit(const QString &unit, bool horiz, bool vert, const QRectF &bbox)
{
    return SvgUtil::parseUnit(m_context.currentGC(), m_context.resolvedProperties(), unit, horiz, vert, bbox);
}

qreal SvgParser::parseUnitX(const QString &unit)
{
    return SvgUtil::parseUnitX(m_context.currentGC(), m_context.resolvedProperties(), unit);
}

qreal SvgParser::parseUnitY(const QString &unit)
{
    return SvgUtil::parseUnitY(m_context.currentGC(), m_context.resolvedProperties(), unit);
}

qreal SvgParser::parseUnitXY(const QString &unit)
{
    return SvgUtil::parseUnitXY(m_context.currentGC(), m_context.resolvedProperties(), unit);
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

#define forEachElement( elem, parent ) \
    for ( QDomNode _node = parent.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) \
    if ( ( elem = _node.toElement() ).isNull() ) {} else

QList<QPair<QString, QColor>> SvgParser::parseMeshPatch(const QDomNode& meshpatchNode)
{
    // path and its associated color
    QList<QPair<QString, QColor>> rawstops;

    SvgGraphicsContext *gc = m_context.currentGC();
    if (!gc) return rawstops;

    QDomElement e = meshpatchNode.toElement();

    QDomElement stop;

    forEachElement(stop, e) {
        qreal X = 0;    // dummy value, don't care, just to ensure the function won't blow up (also to avoid a Coverity issue)
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
     * this offset separately.
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

void SvgParser::parseMetadataApplyToShape(const QDomElement &e, KoShape *shape)
{
    const QString titleTag = "title";
    const QString descriptionTag = "desc";
    QDomElement title = e.firstChildElement(titleTag);
    if (!title.isNull()) {
        QDomText text = getTheOnlyTextChild(title);
        if (!text.data().isEmpty()) {
            shape->setAdditionalAttribute(titleTag, text.data());
        }
    }
    QDomElement description = e.firstChildElement(descriptionTag);
    if (!description.isNull()) {
        QDomText text = getTheOnlyTextChild(description);
        if (!text.data().isEmpty()) {
            shape->setAdditionalAttribute(descriptionTag, text.data());
        }
    }
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
    m_context.styleParser().parseStyle(styles, m_inheritStrokeFillByDefault);
}

void SvgParser::applyCurrentStyle(KoShape *shape, const QPointF &shapeToOriginalUserCoordinates)
{
    if (!shape) return;

    applyCurrentBasicStyle(shape);

    if (KoPathShape *pathShape = dynamic_cast<KoPathShape*>(shape)) {
        applyMarkers(pathShape);
    }

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

    applyPaintOrder(shape);
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

    m_context.styleParser().parseStyle(styles, m_inheritStrokeFillByDefault);

    if (!obj)
        return;

    if (!dynamic_cast<KoShapeGroup*>(obj)) {
        applyFillStyle(obj);
        applyStrokeStyle(obj);
    }

    if (KoPathShape *pathShape = dynamic_cast<KoPathShape*>(obj)) {
        applyMarkers(pathShape);
    }

    applyClipping(obj, shapeToOriginalUserCoordinates);
    applyMaskClipping(obj, shapeToOriginalUserCoordinates);

    if (!gc->display || !gc->visible) {
        obj->setVisible(false);
    }
    obj->setTransparency(1.0 - gc->opacity);
    applyPaintOrder(obj);
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
        // of the SHAPE. All the mesh patches will be rendered in the global 'user' coordinate system
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
    } else if (gc->fillType == SvgGraphicsContext::Inherit) {
        shape->setInheritBackground(true);
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
    } else if (gc->strokeType == SvgGraphicsContext::Inherit) {
        shape->setInheritStroke(true);
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

void SvgParser::applyPaintOrder(KoShape *shape)
{
    SvgGraphicsContext *gc = m_context.currentGC();
    if (!gc)
        return;

    if (!gc->paintOrder.isEmpty() && gc->paintOrder != "inherit") {
        QStringList paintOrder = gc->paintOrder.split(" ");
        QVector<KoShape::PaintOrder> order;
        Q_FOREACH(const QString p, paintOrder) {
            if (p == "fill") {
                order.append(KoShape::Fill);
            } else if (p == "stroke") {
                order.append(KoShape::Stroke);
            } else if (p == "markers") {
                order.append(KoShape::Markers);
            }
        }
        if (paintOrder.size() == 1 && order.isEmpty()) { // Normal
            order = KoShape::defaultPaintOrder();
        }
        if (order.size() == 1) {
            if (order.first() == KoShape::Fill) {
                shape->setPaintOrder(KoShape::Fill, KoShape::Stroke);
            } else if (order.first() == KoShape::Stroke) {
                shape->setPaintOrder(KoShape::Stroke, KoShape::Fill);
            } else if (order.first() == KoShape::Markers) {
                shape->setPaintOrder(KoShape::Markers, KoShape::Fill);
            }
        } else if (order.size() > 1) {
            shape->setPaintOrder(order.at(0), order.at(1));
        }
    }
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
     * In internal SVG coordinate systems pixels are linked to absolute
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

QStringList SvgParser::warnings() const
{
    QStringList warnings;

    Q_FOREACH (const KoID &id, m_warnings) {
        warnings << id.name();
    }

    return warnings;
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
        if (overrideChildrenFrom.tagName() == "symbol") {
            childShapes = {parseGroup(overrideChildrenFrom)};
        } else {
            childShapes = parseSingleElement(overrideChildrenFrom, 0);
        }
    } else {
        childShapes = parseContainer(b);
    }

    // handle id
    applyId(b.attribute("id"), group);

    if (b.hasAttribute(KoSvgTextShape_TEXTCONTOURGROUP)) {
        Q_FOREACH(KoShape *shape, childShapes) {
            if (shape->shapeId() == KoSvgTextShape_SHAPEID) {
                shape->setTransformation(group->transformation());

                if (createContext) {
                    m_context.popGraphicsContext();
                }
                return shape;
            }
        }
    }
    addToGroup(childShapes, group);

    applyCurrentStyle(group, extraOffset); // apply style to this group after size is set

    parseMetadataApplyToShape(b, group);

    if (createContext) {
        m_context.popGraphicsContext();
    }

    return group;
}

QDomText SvgParser::getTheOnlyTextChild(const QDomElement &e)
{
    QDomNode firstChild = e.firstChild();
    return !firstChild.isNull() && firstChild == e.lastChild() && firstChild.isText() ?
                firstChild.toText() : QDomText();
}

bool SvgParser::shapeInDefs(const KoShape *shape)
{
    for (auto defs = m_defsShapes.begin(); defs != m_defsShapes.end(); defs++) {
        KoShape *dShape = *defs;
        if (!dShape) continue;
        if (dShape->hasCommonParent(shape)) return true;
    }
    return false;
}

KoShape* SvgParser::getTextPath(const QDomElement &e, bool hideShapesFromDefs) {
    if (e.hasAttribute("path")) {
        QDomElement p = e.ownerDocument().createElement("path");
        p.setAttribute("d", e.attribute("path"));
        KoShape *s = createPath(p);
        if (hideShapesFromDefs) {
            s->setTransparency(1.0);
        }
        return s;
    } else {
        QString pathId;
        if (e.hasAttribute("href")) {
            pathId = e.attribute("href").remove(0, 1);
        } else if (e.hasAttribute("xlink:href")) {
            pathId = e.attribute("xlink:href").remove(0, 1);
        }
        if (!pathId.isNull()) {
            KoShape *s = m_context.shapeById(pathId);
            if (s) {
                KoShape *cloned = s->cloneShape();
                const QTransform absTf = s->absoluteTransformation();
                cloned->setTransformation(absTf * m_shapeParentTransform.value(s).inverted());
                if(cloned && shapeInDefs(s) && hideShapesFromDefs) {
                    cloned->setTransparency(1.0);
                }
                return cloned;
            }
        }
    }
    return nullptr;
}

void SvgParser::parseTextChildren(const QDomElement &e, KoSvgTextLoader &textLoader, bool hideShapesFromDefs) {
    QDomText t = getTheOnlyTextChild(e);
    if (!t.isNull()) {
        textLoader.loadSvgText(t, m_context);
    } else {
        textLoader.enterNodeSubtree();
        for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
            QDomElement b = n.toElement();
            if (b.tagName() == "title" || b.tagName() == "desc") continue; /// TODO: we should skip dublin core metadata too...
            textLoader.nextNode();
            if (b.isNull()) {
                textLoader.loadSvgText(n.toText(), m_context);
                KoShape *styleDummy = new KoPathShape();
                applyCurrentBasicStyle(styleDummy);
                textLoader.setStyleInfo(styleDummy);
            } else {
                m_context.pushGraphicsContext(b);
                uploadStyleToContext(b);
                textLoader.loadSvg(b, m_context);
                if (b.hasChildNodes()) {
                    parseTextChildren(b, textLoader, hideShapesFromDefs);
                }
                textLoader.setTextPathOnCurrentNode(getTextPath(b, hideShapesFromDefs));
                m_context.popGraphicsContext();
            }
        }
        textLoader.leaveNodeSubtree();
    }
    KoShape *styleDummy = new KoPathShape();
    applyCurrentBasicStyle(styleDummy);
    textLoader.setStyleInfo(styleDummy);
}

KoShape *SvgParser::parseTextElement(const QDomElement &e, KoSvgTextShape *mergeIntoShape)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(e.tagName() == "text" || e.tagName() == "tspan" || e.tagName() == "textPath", 0);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_isInsideTextSubtree || e.tagName() == "text", 0);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(e.tagName() == "text" || !mergeIntoShape, 0);

    KoSvgTextShape *rootTextShape  = 0;

    bool hideShapesFromDefs = true;
    if (mergeIntoShape) {
        rootTextShape = mergeIntoShape;
        hideShapesFromDefs = false;
    } else {
        KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value("KoSvgTextShapeID");
        rootTextShape = dynamic_cast<KoSvgTextShape*>(factory->createDefaultShape(m_documentResourceManager));
    }
    KoSvgTextLoader textLoader(rootTextShape);

    if (rootTextShape) {
        m_isInsideTextSubtree = true;
    }

    m_context.pushGraphicsContext(e);
    uploadStyleToContext(e);

    if (rootTextShape) {
        if (!m_context.currentGC()->shapeInsideValue.isEmpty()) {
            QList<KoShape*> shapesInside = createListOfShapesFromCSS(e, m_context.currentGC()->shapeInsideValue, m_context, hideShapesFromDefs);
            rootTextShape->setShapesInside(shapesInside);
        }

        if (!m_context.currentGC()->shapeSubtractValue.isEmpty()) {
            QList<KoShape*> shapesSubtract = createListOfShapesFromCSS(e, m_context.currentGC()->shapeSubtractValue, m_context, hideShapesFromDefs);
            rootTextShape->setShapesSubtract(shapesSubtract);
        }
    }

    if (e.hasAttribute("krita:textVersion")) {
        m_context.currentGC()->textProperties.setProperty(KoSvgTextProperties::KraTextVersionId, e.attribute("krita:textVersion", "1").toInt());

        if (m_isInsideTextSubtree) {
            debugFlake << "WARNING: \"krita:textVersion\" attribute appeared in non-root text shape";
        }
    }

    parseMetadataApplyToShape(e, rootTextShape);

    if (!mergeIntoShape) {
        rootTextShape->setZIndex(m_context.nextZIndex());
    }

    if (m_context.currentGC()->textProperties.hasProperty(KoSvgTextProperties::KraTextVersionId) &&
        m_context.currentGC()->textProperties.property(KoSvgTextProperties::KraTextVersionId).toInt() < 2) {

        static const KoID warning("warn_text_version_1",
                                  i18nc("warning while loading SVG text",
                                        "The document has vector text created "
                                        "in Krita 4.x. When you save the document, "
                                        "the text object will be converted into "
                                        "Krita 5 format that will no longer be "
                                        "compatible with Krita 4.x"));

        if (!m_warnings.contains(warning)) {
            m_warnings << warning;
        }
    }

    textLoader.loadSvg(e, m_context, m_resolveTextPropertiesForTopLevel);

    // 1) apply transformation only in case we are not overriding the shape!
    // 2) the transformation should be applied *before* the shape is added to the group!
    if (!mergeIntoShape) {
        // groups should also have their own coordinate system!
        rootTextShape->applyAbsoluteTransformation(m_context.currentGC()->matrix);
        const QPointF extraOffset = extraShapeOffset(rootTextShape, m_context.currentGC()->matrix);

        // handle id
        applyId(e.attribute("id"), rootTextShape);
        applyCurrentStyle(rootTextShape, extraOffset); // apply style to this group after size is set
    } else {
        m_context.currentGC()->matrix = mergeIntoShape->absoluteTransformation();
        applyCurrentBasicStyle(rootTextShape);
    }

    QDomText onlyTextChild = getTheOnlyTextChild(e);
    if (!onlyTextChild.isNull()) {
        textLoader.loadSvgText(onlyTextChild, m_context);

    } else {
        parseTextChildren(e, textLoader, hideShapesFromDefs);
    }

    m_context.popGraphicsContext();

    m_isInsideTextSubtree = false;

    //rootTextShape->debugParsing();


    return rootTextShape;
}

QList<KoShape*> SvgParser::parseContainer(const QDomElement &e)
{
    QList<KoShape*> shapes;

    // are we parsing a switch container
    bool isSwitch = e.tagName() == "switch";

    DeferredUseStore deferredUseStore(this);

    for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement b = n.toElement();
        if (b.isNull()) {
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
    } else if (b.tagName() == "g" || b.tagName() == "a") {
        // treat svg link <a> as group so we don't miss its child elements
        shapes += parseGroup(b);
    } else if (b.tagName() == "symbol") {
        parseSymbol(b);
    } else if (b.tagName() == "switch") {
        m_context.pushGraphicsContext(b);
        shapes += parseContainer(b);
        m_context.popGraphicsContext();
    } else if (b.tagName() == "defs") {
        if (b.childNodes().count() > 0) {
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
        // not supported!
    } else if (b.tagName() == "clipPath") {
        parseClipPath(b);
    } else if (b.tagName() == "mask") {
        parseClipMask(b);
    } else if (b.tagName() == "marker") {
        parseMarker(b);
    } else if (b.tagName() == "style") {
        m_context.addStyleSheet(b);
    } else if (b.tagName() == "text" || b.tagName() == "tspan" || b.tagName() == "textPath") {
        shapes += parseTextElement(b);
    } else if (b.tagName() == "rect" || b.tagName() == "ellipse" || b.tagName() == "circle" || b.tagName() == "line" || b.tagName() == "polyline"
               || b.tagName() == "polygon" || b.tagName() == "path" || b.tagName() == "image") {
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
                    KisPortingUtils::setUtf8OnStream(stream);
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
        parseMetadataApplyToShape(b, obj);
    }

    m_context.popGraphicsContext();

    if (obj) {
        m_shapeParentTransform.insert(obj, m_context.currentGC()->matrix);
    }
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
        parseMetadataApplyToShape(b, obj);
    }

    m_context.popGraphicsContext();

    if (obj) {
        m_shapeParentTransform.insert(obj, m_context.currentGC()->matrix);
    }

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

KoShape *SvgParser::createShapeFromCSS(const QDomElement e, const QString value, SvgLoadingContext &context, bool hideShapesFromDefs)
{
    if (value.isEmpty()) {
        return 0;
    }
    unsigned int start = value.indexOf('(') + 1;
    unsigned int end = value.indexOf(')', start);

    QString val = value.mid(start, end - start);
    QString fillRule;
    if (val.startsWith("evenodd,")) {
        start += QString("evenodd,").size();
        fillRule = "evenodd";
    } else if (val.startsWith("nonzero,")) {
        start += QString("nonzero,").size();
        fillRule = "nonzero";
    }
    val = value.mid(start, end - start);

    QDomElement el;
    if (value.startsWith("url(")) {
        start = value.indexOf('#') + 1;
        KoShape *s = m_context.shapeById(value.mid(start, end - start));
        if (s) {
            const QTransform absTf = s->absoluteTransformation();
            KoShape *cloned = s->cloneShape();
            cloned->setTransformation(absTf * m_shapeParentTransform.value(s).inverted());
            // When we have a parent, the shape is inside the defs, but when not,
            // it's in the group we're in the currently parsing.

            if (cloned && shapeInDefs(s) && hideShapesFromDefs) {
                cloned->setTransparency(1.0);
            }
            return cloned;
        }
    } else if (value.startsWith("circle(")) {
        el = e.ownerDocument().createElement("circle");
        QStringList params = val.split(" ");
        el.setAttribute("r", SvgUtil::parseUnitXY(context.currentGC(), context.resolvedProperties(), params.first()));
        if (params.contains("at")) {
            // 1 == "at"
            el.setAttribute("cx", SvgUtil::parseUnitX(context.currentGC(), context.resolvedProperties(), params.at(2)));
            el.setAttribute("cy", SvgUtil::parseUnitY(context.currentGC(), context.resolvedProperties(), params.at(3)));
        }
    } else if (value.startsWith("ellipse(")) {
        el = e.ownerDocument().createElement("ellipse");
        QStringList params = val.split(" ");
        el.setAttribute("rx", SvgUtil::parseUnitX(context.currentGC(), context.resolvedProperties(), params.at(0)));
        el.setAttribute("ry", SvgUtil::parseUnitY(context.currentGC(), context.resolvedProperties(), params.at(1)));
        if (params.contains("at")) {
            // 2 == "at"
            el.setAttribute("cx", SvgUtil::parseUnitX(context.currentGC(), context.resolvedProperties(), params.at(3)));
            el.setAttribute("cy", SvgUtil::parseUnitY(context.currentGC(), context.resolvedProperties(), params.at(4)));
        }
    } else if (value.startsWith("polygon(")) {
        el = e.ownerDocument().createElement("polygon");
        QStringList points;
        Q_FOREACH(QString point,  SvgUtil::simplifyList(val)) {
            bool xVal = points.size() % 2;
            if (xVal) {
                points.append(QString::number(SvgUtil::parseUnitX(context.currentGC(), context.resolvedProperties(), point)));
            } else {
                points.append(QString::number(SvgUtil::parseUnitY(context.currentGC(), context.resolvedProperties(), point)));
            }
        }
        el.setAttribute("points", points.join(" "));
    } else if (value.startsWith("path(")) {
        el = e.ownerDocument().createElement("path");
        // SVG path data is inside a string.
        start += 1;
        end -= 1;
        el.setAttribute("d", value.mid(start, end - start));
    }

    el.setAttribute("fill-rule", fillRule);
    KoShape *shape = createShapeFromElement(el, context);
    if (shape) shape->setTransparency(1.0);
    return shape;
}

QList<KoShape *> SvgParser::createListOfShapesFromCSS(const QDomElement e, const QString value, SvgLoadingContext &context, bool hideShapesFromDefs)
{
    QList<KoShape*> shapeList;
    if (value == "auto" || value == "none") {
        return shapeList;
    }
    QStringList params = value.split(")");
    Q_FOREACH(const QString param, params) {
        KoShape *s = createShapeFromCSS(e, param.trimmed()+")", context, hideShapesFromDefs);
        if (s) {
            shapeList.append(s);
        }
    }
    return shapeList;
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

    KoShape *existingShape = m_context.shapeById(id);
    if (existingShape) {
        debugFlake << "SVG contains nodes with duplicated id:" << id;
        // Generate a random name and just don't register the shape.
        // We don't use the name as a unique identifier so we don't need to
        // worry about the extremely rare case of name collision.
        const QString suffix = QString::number(QRandomGenerator::system()->bounded(0x10000000, 0x7FFFFFFF), 16);
        const QString newName = id + '_' + suffix;
        shape->setName(newName);
    } else {
        shape->setName(id);
        m_context.registerShape(id, shape);
    }
}
