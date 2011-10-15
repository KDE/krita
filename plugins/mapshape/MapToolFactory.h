/* Part of Calligra Suite - Map Shape
   Copyright 2007 Montel Laurent <montel@kde.org>
   Copyright 2008 Simon Schmeisser <mail_to_wrt@gmx.de>
   Copyright (C) 2011  Radosław Wicik <radoslaw@wicik.pl>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef MAPTOOLFACTORY_H
#define MAPTOOLFACTORY_H

#include <KoToolFactoryBase.h>

class MapToolFactory : public KoToolFactoryBase
{
public:
    MapToolFactory();
    ~MapToolFactory();

    KoToolBase *createTool(KoCanvasBase *canvas);
};

#endif
