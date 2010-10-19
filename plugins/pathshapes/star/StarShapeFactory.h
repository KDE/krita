/* This file is part of the KDE project
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>

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

#ifndef KOSTARHAPEFACTORY_H
#define KOSTARHAPEFACTORY_H

#include <KoShapeFactoryBase.h>

class KoShape;

/// Factory for path shapes
class StarShapeFactory : public KoShapeFactoryBase
{
    Q_OBJECT

public:
    /// constructor
    explicit StarShapeFactory(QObject *parent);
    ~StarShapeFactory() {}
    virtual KoShape *createDefaultShape(KoResourceManager *documentResources = 0) const;
    virtual KoShape *createShape(const KoProperties *params, KoResourceManager *documentResources = 0) const;
    virtual bool supports(const KoXmlElement &e, KoShapeLoadingContext &context) const;
    virtual QList<KoShapeConfigWidgetBase*> createShapeOptionPanels();
};

#endif // KOSTARHAPEFACTORY_H
