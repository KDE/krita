/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2002-2003, 2005 Rob Buis <buis@kde.org>
 * SPDX-FileCopyrightText: 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * SPDX-FileCopyrightText: 2005, 2007-2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SVGPARSER_H
#define SVGPARSER_H

#include <KoXmlReader.h>

#include <QMap>
#include <QSizeF>
#include <QRectF>
#include <QSharedPointer>
#include <QExplicitlySharedDataPointer>

#include "kritaflake_export.h"
#include "SvgGradientHelper.h"
#include "SvgFilterHelper.h"
#include "SvgClipPathHelper.h"
#include "SvgLoadingContext.h"
#include "SvgStyleParser.h"
#include "KoClipMask.h"
#include <resources/KoSvgSymbolCollectionResource.h>

class KoShape;
class KoShapeGroup;
class KoShapeContainer;
class KoDocumentResourceManager;
class KoVectorPatternBackground;
class KoMarker;
class KoPathShape;
class KoSvgTextShape;

class KRITAFLAKE_EXPORT SvgParser
{
    struct DeferredUseStore;

public:
    explicit SvgParser(KoDocumentResourceManager *documentResourceManager);
    virtual ~SvgParser();

    static KoXmlDocument createDocumentFromSvg(QIODevice *device, QString *errorMsg = 0, int *errorLine = 0, int *errorColumn = 0);
    static KoXmlDocument createDocumentFromSvg(const QByteArray &data, QString *errorMsg = 0, int *errorLine = 0, int *errorColumn = 0);
    static KoXmlDocument createDocumentFromSvg(const QString &data, QString *errorMsg = 0, int *errorLine = 0, int *errorColumn = 0);
    static KoXmlDocument createDocumentFromSvg(QXmlInputSource *source, QString *errorMsg = 0, int *errorLine = 0, int *errorColumn = 0);

    /// Parses a svg fragment, returning the list of top level child shapes
    QList<KoShape*> parseSvg(const KoXmlElement &e, QSizeF * fragmentSize = 0);

    /// Sets the initial xml base directory (the directory form where the file is read)
    void setXmlBaseDir(const QString &baseDir);

    void setResolution(const QRectF boundsInPixels, qreal pixelsPerInch);

    /// A special workaround coeff for using when loading old ODF-embedded SVG files,
    /// which used hard-coded 96 ppi for font size
    void setForcedFontSizeResolution(qreal value);

    /// Returns the list of all shapes of the svg document
    QList<KoShape*> shapes() const;

    /// Takes the collection of symbols contained in the svg document. The parser will
    /// no longer know about the symbols.
    QVector<KoSvgSymbol*> takeSymbols();

    QString documentTitle() const;
    QString documentDescription() const;

    
    typedef std::function<QByteArray(const QString&)> FileFetcherFunc;
    void setFileFetcher(FileFetcherFunc func);

    QList<QExplicitlySharedDataPointer<KoMarker>> knownMarkers() const;

    void parseDefsElement(const KoXmlElement &e);
    KoShape* parseTextElement(const KoXmlElement &e, KoSvgTextShape *mergeIntoShape = 0);

protected:

    /// Parses a group-like element element, saving all its topmost properties
    KoShape* parseGroup(const KoXmlElement &e, const KoXmlElement &overrideChildrenFrom = KoXmlElement(), bool createContext = true);

    // XXX
    KoShape* parseTextNode(const KoXmlText &e);
    
    /// Parses a container element, returning a list of child shapes
    QList<KoShape*> parseContainer(const KoXmlElement &, bool parseTextNodes = false);

    /// XXX
    QList<KoShape*> parseSingleElement(const KoXmlElement &b, DeferredUseStore* deferredUseStore = 0);

    /// Parses a use element, returning a list of child shapes
    KoShape* parseUse(const KoXmlElement &, DeferredUseStore* deferredUseStore);

    KoShape* resolveUse(const KoXmlElement &e, const QString& key);

    /// Parses a gradient element
    SvgGradientHelper *parseGradient(const KoXmlElement &);

    /// Parses mesh gradient element
    SvgGradientHelper* parseMeshGradient(const KoXmlElement&);
    
    /// Parses a single meshpatch and returns the pointer
    QList<QPair<QString, QColor>> parseMeshPatch(const KoXmlNode& meshpatch);

    /// Parses a pattern element
    QSharedPointer<KoVectorPatternBackground> parsePattern(const KoXmlElement &e, const KoShape *__shape);

    /// Parses a filter element
    bool parseFilter(const KoXmlElement &, const KoXmlElement &referencedBy = KoXmlElement());

    /// Parses a clip path element
    bool parseClipPath(const KoXmlElement &);
    bool parseClipMask(const KoXmlElement &e);

    bool parseMarker(const KoXmlElement &e);

    bool parseSymbol(const KoXmlElement &e);

    /// parses a length attribute
    qreal parseUnit(const QString &, bool horiz = false, bool vert = false, const QRectF &bbox = QRectF());

    /// parses a length attribute in x-direction
    qreal parseUnitX(const QString &unit);

    /// parses a length attribute in y-direction
    qreal parseUnitY(const QString &unit);

    /// parses a length attribute in xy-direction
    qreal parseUnitXY(const QString &unit);

    /// parses a angular attribute values, result in radians
    qreal parseAngular(const QString &unit);

    KoShape *createObjectDirect(const KoXmlElement &b);

    /// Creates an object from the given xml element
    KoShape * createObject(const KoXmlElement &, const SvgStyles &style = SvgStyles());

    /// Create path object from the given xml element
    KoShape * createPath(const KoXmlElement &);

    /// find gradient with given id in gradient map
    SvgGradientHelper* findGradient(const QString &id);

    /// find pattern with given id in pattern map
    QSharedPointer<KoVectorPatternBackground> findPattern(const QString &id, const KoShape *shape);

    /// find filter with given id in filter map
    SvgFilterHelper* findFilter(const QString &id, const QString &href = QString());

    /// find clip path with given id in clip path map
    SvgClipPathHelper* findClipPath(const QString &id);

    /// Adds list of shapes to the given group shape
    void addToGroup(QList<KoShape*> shapes, KoShapeContainer *group);

    /// creates a shape from the given shape id
    KoShape * createShape(const QString &shapeID);

    /// Creates shape from specified svg element
    KoShape * createShapeFromElement(const KoXmlElement &element, SvgLoadingContext &context);

    /// Builds the document from the given shapes list
    void buildDocument(QList<KoShape*> shapes);

    void uploadStyleToContext(const KoXmlElement &e);
    void applyCurrentStyle(KoShape *shape, const QPointF &shapeToOriginalUserCoordinates);
    void applyCurrentBasicStyle(KoShape *shape);

    /// Applies styles to the given shape
    void applyStyle(KoShape *, const KoXmlElement &, const QPointF &shapeToOriginalUserCoordinates);

    /// Applies styles to the given shape
    void applyStyle(KoShape *, const SvgStyles &, const QPointF &shapeToOriginalUserCoordinates);

    /// Applies the current fill style to the object
    void applyFillStyle(KoShape * shape);

    /// Applies the current stroke style to the object
    void applyStrokeStyle(KoShape * shape);

    /// Applies the current filter to the object
    void applyFilter(KoShape * shape);

    /// Applies the current clip path to the object
    void applyClipping(KoShape *shape, const QPointF &shapeToOriginalUserCoordinates);
    void applyMaskClipping(KoShape *shape, const QPointF &shapeToOriginalUserCoordinates);
    void applyMarkers(KoPathShape *shape);

    /// Applies id to specified shape
    void applyId(const QString &id, KoShape *shape);

    /// Applies viewBox transformation to the current graphical context
    /// NOTE: after applying the function currentBoundingBox can become null!
    void applyViewBoxTransform(const KoXmlElement &element);

private:
    QSizeF m_documentSize;
    SvgLoadingContext m_context;
    QMap<QString, SvgGradientHelper> m_gradients;
    QMap<QString, SvgFilterHelper> m_filters;
    QMap<QString, SvgClipPathHelper> m_clipPaths;
    QMap<QString, QSharedPointer<KoClipMask>> m_clipMasks;
    QMap<QString, QExplicitlySharedDataPointer<KoMarker>> m_markers;
    KoDocumentResourceManager *m_documentResourceManager;
    QList<KoShape*> m_shapes;
    QMap<QString, KoSvgSymbol*> m_symbols;
    QList<KoShape*> m_defsShapes;
    bool m_isInsideTextSubtree = false;
    QString m_documentTitle;
    QString m_documentDescription;
};

#endif
