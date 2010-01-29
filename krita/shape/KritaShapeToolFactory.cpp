/* This file is part of the KDE project
   Copyright 2007 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KritaShapeToolFactory.h"
#include <klocale.h>

#include "KritaShape.h"
#include "KritaShapeTool.h"

KritaShapeToolFactory::KritaShapeToolFactory(QObject* parent)
        : KoToolFactoryBase(parent, "KritaShapeToolFactoryId")
{
    setToolTip(i18n("KritaShape editing tool"));
    setIcon("kritashape");
    setToolType(dynamicToolType());
    setPriority(1);
    setActivationShapeId(KritaShapeId);
}

KritaShapeToolFactory::~KritaShapeToolFactory()
{
}

KoToolBase* KritaShapeToolFactory::createTool(KoCanvasBase* canvas)
{
    return new KritaShapeTool(canvas);
}

#include "KritaShapeToolFactory.moc"


