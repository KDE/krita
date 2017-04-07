/* This file is part of the KDE project
 *
 * Copyright 2017 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef VECTORSHAPE_H
#define VECTORSHAPE_H

// Qt
#include <QByteArray>
#include <QCache>
#include <QSize>
#include <QRunnable>
#include <QMutex>

// Calligra
#include <KoShape.h>
#include <SvgShape.h>

class QPainter;
class TextNGShape;

#define TextNGShape_SHAPEID "TextNGShapeID"

class TextNGShape : public QObject, public KoShape, public SvgShape
{
    Q_OBJECT
public:
    TextNGShape();
    virtual ~TextNGShape();

    void paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext);
    virtual void saveOdf(KoShapeSavingContext &Context) const;
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &Context);
    virtual bool loadOdfFrameElement(const KoXmlElement &frameElement,
                                     KoShapeLoadingContext &Context);
    /// Saves data utilizing specified svg saving context
    virtual bool saveSvg(SvgSavingContext &context);

    /// Loads data from specified svg element
    virtual bool loadSvg(const KoXmlElement &element, SvgLoadingContext &context);

private:
    struct Private;
    Private * const d;

};

#endif
