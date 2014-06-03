/* This file is part of the KDE project
 *
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
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

#ifndef TEMPLATESHAPE_H
#define TEMPLATESHAPE_H

// Qt
#include <QObject>

// Calligra
#include <KoShape.h>

// This shape
//#include "Foo.h"


#define TEMPLATESHAPEID "TemplateShape"


class TemplateShape : public QObject, public KoShape
{
    Q_OBJECT

public:
    TemplateShape();
    virtual ~TemplateShape();

    // reimplemented from KoShape
    virtual void paint(QPainter &painter, const KoViewConverter &converter,
                       KoShapePaintingContext &paintcontext);
    // reimplemented from KoShape
    virtual void saveOdf(KoShapeSavingContext &context) const;
    // reimplemented from KoShape
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    // reimplemented from KoShape
    virtual void waitUntilReady(const KoViewConverter &converter, bool asynchronous) const;


private:
    // Shape members here.  D-pointer is not needed since this is not a library.
};


#endif
