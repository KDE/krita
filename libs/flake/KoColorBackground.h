/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOCOLORBACKGROUND_H
#define KOCOLORBACKGROUND_H

#include "KoShapeBackground.h"
#include "kritaflake_export.h"
#include <Qt>
#include <QSharedDataPointer>

class KoColorBackgroundPrivate;
class QColor;
class QBrush;

/// A simple solid color shape background
class KRITAFLAKE_EXPORT KoColorBackground : public KoShapeBackground
{
public:
    KoColorBackground();

    /// Creates background from given color and style
    explicit KoColorBackground(const QColor &color, Qt::BrushStyle style = Qt::SolidPattern);

    ~KoColorBackground() override;

    // Work around MSVC inability to generate copy ops with QSharedDataPointer.
    KoColorBackground(const KoColorBackground &);
    KoColorBackground &operator=(const KoColorBackground &);

    bool compareTo(const KoShapeBackground *other) const override;

    /// Returns the background color
    QColor color() const;

    /// Sets the background color
    void setColor(const QColor &color);

    /// Returns the background style
    Qt::BrushStyle style() const;

    QBrush brush() const;

    // reimplemented from KoShapeBackground
    void paint(QPainter &painter, KoShapePaintingContext &context, const QPainterPath &fillPath) const override;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif // KOCOLORBACKGROUND_H
