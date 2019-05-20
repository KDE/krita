/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
 *
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

#ifndef KOODFGRADIENTBACKGROUND_H
#define KOODFGRADIENTBACKGROUND_H

#include "KoShapeBackground.h"
#include "kritaflake_export.h"

class QImage;

class KoOdfGradientBackgroundPrivate;
#include <KoXmlReaderForward.h>
class KoGenStyles;
class KoGenStyle;

/// Gradients from odf that are not native to Qt
class KoOdfGradientBackground : public KoShapeBackground {
public:
    // constructor
    KoOdfGradientBackground();
    // destructor
    ~KoOdfGradientBackground() override;

    bool compareTo(const KoShapeBackground *other) const override;

    /// reimplemented from KoShapeBackground
    void fillStyle(KoGenStyle& style, KoShapeSavingContext& context) override;
    /// reimplemented from KoShapeBackground
    bool loadStyle(KoOdfLoadingContext& context, const QSizeF& shapeSize) override;
    /// reimplemented from KoShapeBackground
    void paint(QPainter& painter, const KoViewConverter &converter, KoShapePaintingContext &context, const QPainterPath& fillPath) const override;

private:
    bool loadOdf(const KoXmlElement &element);
    void saveOdf(KoGenStyle& styleFill, KoGenStyles& mainStyles) const;

    void renderSquareGradient(QImage &buffer) const;
    void renderRectangleGradient(QImage &buffer) const;

private:
    void debug() const;

private:
    // Q_DECLARE_PRIVATE(KoOdfGradientBackground)
    Q_DISABLE_COPY(KoOdfGradientBackground)
};

#endif
