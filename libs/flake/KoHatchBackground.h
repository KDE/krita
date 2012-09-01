/* This file is part of the KDE project
 *
 * Copyright (C) 2012 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOHATCHBACKGROUND_H
#define KOHATCHBACKGROUND_H

#include "KoColorBackground.h"

class KoHatchBackgroundPrivate;

/**
 * A hatch shape background
 */
class KoHatchBackground : public KoColorBackground
{
public:
    enum HatchStyle {
        Single,
        Double,
        Triple
    };

    KoHatchBackground();

    // reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &context, const QPainterPath &fillPath) const;

    // reimplemented
    virtual void fillStyle(KoGenStyle &style, KoShapeSavingContext &context);

    // reimplemented
    virtual bool loadStyle(KoOdfLoadingContext &context, const QSizeF &shapeSize);

private:
    QString saveHatchStyle(KoShapeSavingContext &context) const;

    Q_DECLARE_PRIVATE(KoHatchBackground)
};

#endif /* KOHATCHBACKGROUND_H */
