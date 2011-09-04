/*
    Part of Calligra Suite - Marble Map Shape
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


#ifndef MARBLEMAPSHAPEFACTORY_H
#define MARBLEMAPSHAPEFACTORY_H

#include <KoShapeFactoryBase.h>


class MarbleMapShapeFactory : public KoShapeFactoryBase
{

public:
    explicit MarbleMapShapeFactory();
    virtual ~MarbleMapShapeFactory();
    virtual bool supports(const KoXmlElement& element, KoShapeLoadingContext& context) const;
    virtual KoShape* createDefaultShape(KoResourceManager* documentResources = 0) const;
};

#endif // MARBLEMAPSHAPEFACTORY_H
