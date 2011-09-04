/* Part of Calligra Suite - Marble Map Shape
   Copyright 2007 Montel Laurent <montel@kde.org>
   Copyright 2008 Simon Schmeisser <mail_to_wrt@gmx.de>
   Copyright (C) 2008 Inge Wallin  <inge@lysator.liu.se.de>
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


// Own
#include "MarbleMapToolFactory.h"

// KDE
#include <klocale.h>

// GeoShape
#include "MarbleMapShape.h"
#include "MarbleMapTool.h"



MarbleMapToolFactory::MarbleMapToolFactory()
    : KoToolFactoryBase("MarbleMapToolFactoryId")
{
    setToolTip(i18n("Marble Map editing tool"));
    setIcon("marble"); ///FIXME: set proper icon.
    setToolType(dynamicToolType());
    setPriority(1);
    setActivationShapeId(MARBLEMAPSHAPEID);
}

MarbleMapToolFactory::~MarbleMapToolFactory()
{
}

KoToolBase *MarbleMapToolFactory::createTool(KoCanvasBase *canvas)
{
    return new MarbleMapTool(canvas);
}

#include "MarbleMapToolFactory.moc"


