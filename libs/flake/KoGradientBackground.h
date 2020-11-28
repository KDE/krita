/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOGRADIENTBACKGROUND_H
#define KOGRADIENTBACKGROUND_H

#include "KoShapeBackground.h"
#include "kritaflake_export.h"

#include <QTransform>
#include <QSharedDataPointer>

class QGradient;

/// A gradient shape background
class KRITAFLAKE_EXPORT KoGradientBackground : public KoShapeBackground
{
public:
    /**
     * Creates new gradient background from given gradient.
     * The background takes ownership of the given gradient.
     */
    explicit KoGradientBackground(QGradient *gradient, const QTransform &matrix = QTransform());

    /**
     * Create new gradient background from the given gradient.
     * A clone of the given gradient is used.
     */
    explicit KoGradientBackground(const QGradient &gradient, const QTransform &matrix = QTransform());

    /// Destroys the background
    ~KoGradientBackground() override;

    bool compareTo(const KoShapeBackground *other) const override;

    /// Sets the transform matrix
    void setTransform(const QTransform &matrix);

    /// Returns the transform matrix
    QTransform transform() const;

    /**
     * Sets a new gradient.
     * A clone of the given gradient is used.
     */
    void setGradient(const QGradient &gradient);

    /// Returns the gradient
    const QGradient *gradient() const;

    /// reimplemented from KoShapeBackground
    void paint(QPainter &painter, KoShapePaintingContext &context, const QPainterPath &fillPath) const override;
private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif // KOGRADIENTBACKGROUND_H
