/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOSHAPEBACKGROUND_H
#define KOSHAPEBACKGROUND_H

#include "kritaflake_export.h"

#include <QtGlobal>
#include <QSharedDataPointer>

class QSizeF;
class QPainter;
class QPainterPath;
class KoGenStyle;
class KoShapeSavingContext;
class KoOdfLoadingContext;
class KoShapeBackgroundPrivate;
class KoShapePaintingContext;
class KoViewConverter;

#define CONST_SHARED_D(Class) const Class##Private *d = dynamic_cast<const Class##Private *>(d_ptr.constData())
#define SHARED_D(Class) Class##Private *d = dynamic_cast<Class##Private *>(d_ptr.data())

/**
 * This is the base class for shape backgrounds.
 * Derived classes are used to paint the background of
 * a shape within a given painter path.
 */
class KRITAFLAKE_EXPORT KoShapeBackground
{
public:
    KoShapeBackground();
    virtual ~KoShapeBackground();

    /// Paints the background using the given fill path
    virtual void paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &context, const QPainterPath &fillPath) const = 0;

    /// Returns if the background has some transparency.
    virtual bool hasTransparency() const;

    virtual bool compareTo(const KoShapeBackground *other) const = 0;

    /**
     * Fills the style object
     * @param style object
     * @param context used for saving
     */
    virtual void fillStyle(KoGenStyle &style, KoShapeSavingContext &context) = 0;

    /// load background from odf styles
    virtual bool loadStyle(KoOdfLoadingContext &context, const QSizeF &shapeSize) = 0;

    virtual explicit operator bool() const { return true; }

protected:
    KoShapeBackground(KoShapeBackgroundPrivate &);
    QSharedDataPointer<KoShapeBackgroundPrivate> d_ptr;
private:
    // Q_DECLARE_PRIVATE(KoShapeBackground)


};

#endif // KOSHAPEBACKGROUND_H
