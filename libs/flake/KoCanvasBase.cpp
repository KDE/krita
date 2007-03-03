/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoCanvasBase.h"
#include "KoCanvasResourceProvider.h"
#include "KoShapeController.h"

class KoCanvasBase::Private {
public:
    Private() : shapeController(0), resourceProvider(0) {}
    ~Private() {
        delete shapeController;
        delete resourceProvider;
    }
    KoShapeController *shapeController;
    KoCanvasResourceProvider * resourceProvider;
};

KoCanvasBase::KoCanvasBase( KoShapeControllerBase * shapeControllerBase )
    : d(new Private())
{
    d->resourceProvider = new KoCanvasResourceProvider(0);
    d->shapeController = new KoShapeController( this, shapeControllerBase );
}

KoCanvasBase::~KoCanvasBase()
{
    delete d;
}


KoShapeController * KoCanvasBase::shapeController() const {
    return d->shapeController;
}

KoCanvasResourceProvider * KoCanvasBase::resourceProvider() const
{
    return d->resourceProvider;
}
