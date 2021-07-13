/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATTERNBACKGROUND_H
#define KOPATTERNBACKGROUND_H

#include "KoShapeBackground.h"
#include "kritaflake_export.h"

#include <QSharedDataPointer>

class KoImageCollection;
class KoPatternBackgroundPrivate;
class KoImageData;

class QTransform;
class QImage;
class QPointF;
class QRectF;

/// A pattern shape background
class KRITAFLAKE_EXPORT KoPatternBackground : public KoShapeBackground
{
public:
    /// Pattern rendering style
    enum PatternRepeat {
        Original,
        Tiled,
        Stretched
    };
    /// Pattern reference point
    enum ReferencePoint {
        TopLeft,
        Top,
        TopRight,
        Left,
        Center,
        Right,
        BottomLeft,
        Bottom,
        BottomRight
    };

    /// Constructs a new pattern background utilizing the given image collection
    explicit KoPatternBackground(KoImageCollection *collection);

    ~KoPatternBackground() override;

    // Work around MSVC inability to generate copy ops with QSharedDataPointer.
    KoPatternBackground(const KoPatternBackground &);
    KoPatternBackground& operator=(const KoPatternBackground &);

    bool compareTo(const KoShapeBackground *other) const override;

    /// Sets the transform matrix
    void setTransform(const QTransform &matrix);

    /// Returns the transform matrix
    QTransform transform() const;

    /// Sets a new pattern
    void setPattern(const QImage &pattern);

    /// Sets a new pattern. imageData memory is deleted inside this class
    void setPattern(KoImageData *imageData);

    /// Returns the pattern
    QImage pattern() const;

    /// Sets the pattern repeatgfl
    void setRepeat(PatternRepeat repeat);

    /// Returns the pattern repeat
    PatternRepeat repeat() const;

    /// Returns the pattern reference point identifier
    ReferencePoint referencePoint() const;

    /// Sets the pattern reference point
    void setReferencePoint(ReferencePoint referencePoint);

    /// Returns reference point offset in percent of the pattern display size
    QPointF referencePointOffset() const;

    /// Sets the reference point offset in percent of the pattern display size
    void setReferencePointOffset(const QPointF &offset);

    /// Returns tile repeat offset in percent of the pattern display size
    QPointF tileRepeatOffset() const;

    /// Sets the tile repeat offset in percent of the pattern display size
    void setTileRepeatOffset(const QPointF &offset);

    /// Returns the pattern display size
    QSizeF patternDisplaySize() const;

    /// Sets pattern display size
    void setPatternDisplaySize(const QSizeF &size);

    /// Returns the original image size
    QSizeF patternOriginalSize() const;

    /// reimplemented from KoShapeBackground
    void paint(QPainter &painter, KoShapePaintingContext &context, const QPainterPath &fillPath) const override;

    /// Returns the bounding rect of the pattern image based on the given fill size
    QRectF patternRectFromFillSize(const QSizeF &size);
private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif // KOPATTERNBACKGROUND_H
