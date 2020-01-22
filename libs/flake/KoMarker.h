/* This file is part of the KDE project
   Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOMARKER_H
#define KOMARKER_H

#include <QMetaType>
#include <QSharedData>

#include "kritaflake_export.h"
#include <KoFlake.h>

#include <KoXmlReaderForward.h>

class KoShapeLoadingContext;
class KoShapeSavingContext;
class QString;
class QPainterPath;
class KoShape;
class QPainter;
class KoShapeStroke;

class  KRITAFLAKE_EXPORT KoMarker : public QSharedData
{
public:
    KoMarker();
    ~KoMarker();

    /**
     * Display name of the marker
     *
     * @return Display name of the marker
     */
    QString name() const;

    KoMarker(const KoMarker &rhs);
    bool operator==(const KoMarker &other) const;

    enum MarkerCoordinateSystem {
        StrokeWidth,
        UserSpaceOnUse
    };

    void setCoordinateSystem(MarkerCoordinateSystem value);
    MarkerCoordinateSystem coordinateSystem() const;

    static MarkerCoordinateSystem coordinateSystemFromString(const QString &value);
    static QString coordinateSystemToString(MarkerCoordinateSystem value);

    void setReferencePoint(const QPointF &value);
    QPointF referencePoint() const;

    void setReferenceSize(const QSizeF &size);
    QSizeF referenceSize() const;

    bool hasAutoOtientation() const;
    void setAutoOrientation(bool value);

    // measured in radians!
    qreal explicitOrientation() const;

    // measured in radians!
    void setExplicitOrientation(qreal value);

    void setShapes(const QList<KoShape*> &shapes);
    QList<KoShape*> shapes() const;

    /**
     * @brief paintAtOrigin paints the marker at the position \p pos.
     *        Scales and rotates the masrker if needed.
     */
    void paintAtPosition(QPainter *painter, const QPointF &pos, qreal strokeWidth, qreal nodeAngle);

    /**
     * Return maximum distance that the marker can take outside the shape itself
     */
    qreal maxInset(qreal strokeWidth) const;

    /**
     * Bounding rect of the marker in local coordinates. It is assumed that the marker
     * is painted with the reference point placed at position (0,0)
     */
    QRectF boundingRect(qreal strokeWidth, qreal nodeAngle) const;

    /**
     * Outline of the marker in local coordinates. It is assumed that the marker
     * is painted with the reference point placed at position (0,0)
     */
    QPainterPath outline(qreal strokeWidth, qreal nodeAngle) const;

    /**
     * Draws a preview of the marker in \p previewRect of \p painter
     */
    void drawPreview(QPainter *painter, const QRectF &previewRect,
                     const QPen &pen, KoFlake::MarkerPosition position);


    void applyShapeStroke(const KoShape *shape, KoShapeStroke *stroke, const QPointF &pos, qreal strokeWidth, qreal nodeAngle);

private:
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(KoMarker*)

#endif /* KOMARKER_H */
