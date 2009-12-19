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
#include "KoCanvasController.h"
#include "KoSnapGuide.h"
#include "SnapGuideConfigWidget.h"

#include <KGlobal>
#include <KConfigGroup>
#include <KSharedPtr>
#include <KSharedConfig>

class KoCanvasBase::Private
{
public:
    Private() : shapeController(0), resourceProvider(0), controller(0) {}
    ~Private() {
        delete shapeController;
        delete resourceProvider;
        delete snapGuide;
    }
    KoShapeController *shapeController;
    KoCanvasResourceProvider * resourceProvider;
    KoCanvasController *controller;
    KoSnapGuide * snapGuide;
};

KoCanvasBase::KoCanvasBase(KoShapeControllerBase *shapeControllerBase)
        : d(new Private())
{
    d->resourceProvider = new KoCanvasResourceProvider();
    d->shapeController = new KoShapeController(this, shapeControllerBase);
    d->snapGuide = new KoSnapGuide(this);
}

KoCanvasBase::~KoCanvasBase()
{
    delete d;
}


KoShapeController *KoCanvasBase::shapeController() const
{
    return d->shapeController;
}

KoCanvasResourceProvider *KoCanvasBase::resourceProvider() const
{
    return d->resourceProvider;
}

void KoCanvasBase::ensureVisible(const QRectF &rect)
{
    if (d->controller)
        d->controller->ensureVisible(rect);
}

void KoCanvasBase::setCanvasController(KoCanvasController *controller)
{
    d->controller = controller;
}

KoCanvasController *KoCanvasBase::canvasController() const
{
    return d->controller;
}

void KoCanvasBase::clipToDocument(const KoShape *, QPointF &) const
{
}

KoSnapGuide * KoCanvasBase::snapGuide() const
{
    return d->snapGuide;
}

KoGuidesData * KoCanvasBase::guidesData()
{
    return 0;
}

QWidget *KoCanvasBase::createSnapGuideConfigWidget() const
{
    return new SnapGuideConfigWidget(d->snapGuide);
}
