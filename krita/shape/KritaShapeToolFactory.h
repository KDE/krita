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

#ifndef KRITA_SHAPE_TOOL_FACTORY
#define KRITA_SHAPE_TOOL_FACTORY

#include <KoToolFactoryBase.h>

class KritaShapeToolFactory : public KoToolFactoryBase
{
    Q_OBJECT
public:
    KritaShapeToolFactory(QObject* parent);
    ~KritaShapeToolFactory();

    KoToolBase* createTool(KoCanvasBase* canvas);
};


#endif
