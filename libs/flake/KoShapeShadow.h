/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2010 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPESHADOW_H
#define KOSHAPESHADOW_H

#include "kritaflake_export.h"

#include <QtGlobal>

class KoShape;
class KoShapeSavingContext;
class QPainter;
class QPointF;
class QColor;
struct KoInsets;

class KRITAFLAKE_EXPORT KoShapeShadow
{
public:
    KoShapeShadow();
    ~KoShapeShadow();

    KoShapeShadow(const KoShapeShadow &rhs);
    KoShapeShadow& operator=(const KoShapeShadow &rhs);

    /**
     * Paints the shadow of the shape.
     * @param shape the shape to paint around
     * @param painter the painter to paint shadows to canvas
     * @param converter to convert between internal and view coordinates.
     */
    void paint(KoShape *shape, QPainter &painter);

    /**
     * Sets the shadow offset from the topleft corner of the shape
     * @param offset the shadow offset
     */
    void setOffset(const QPointF &offset);

    /// Returns the shadow offset
    QPointF offset() const;

    /**
     * Sets the shadow color, including the shadow opacity.
     * @param color the shadow color and opacity
     */
    void setColor(const QColor &color);

    /// Returns the shadow color including opacity
    QColor color() const;

    /**
     * Sets the shadow blur radius of the shape
     * @param blur the shadow blur radius
     */
    void setBlur(qreal blur);

    /// Returns the shadow blur radius
    qreal blur() const;

    /// Sets the shadow visibility
    void setVisible(bool visible);

    /// Returns if shadow is visible
    bool isVisible() const;

    /// Fills the insets object with the space the shadow takes around a shape
    void insets(KoInsets &insets) const;

    /**
     * Increments the use-value.
     * Returns true if the new value is non-zero, false otherwise.
     */
    bool ref();
    /**
     * Decrements the use-value.
     * Returns true if the new value is non-zero, false otherwise.
     */
    bool deref();
    /// Return the usage count
    int useCount() const;

private:
    class Private;
    Private * const d;
};

#endif // KOSHAPESHADOW_H
