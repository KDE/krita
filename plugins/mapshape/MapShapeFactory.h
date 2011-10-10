/*
    Part of Calligra Suite - Map Shape
    Copyright (C) 2007 Thomas Zander <zander@kde.org>
    Copyright (C) 2008 Simon Schmeißer <mail_to_wrt@gmx.de>
    Copyright (C) 2011  Radosław Wicik <radoslaw@wicik.pl>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef MAPSHAPEFACTORY_H
#define MAPSHAPEFACTORY_H

#include <KoShapeFactoryBase.h>


class MapShapeFactory : public KoShapeFactoryBase
{

public:
    explicit MapShapeFactory();
    virtual ~MapShapeFactory();
    virtual bool supports(const KoXmlElement& element, KoShapeLoadingContext& context) const;
    virtual KoShape* createDefaultShape(KoDocumentResourceManager* documentResources = 0) const;
};

#endif // MAPSHAPEFACTORY_H
