/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "ShadowDocker.h"
#include <KoShapeShadow.h>
#include <KoCanvasBase.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoToolManager.h>
#include <KoCanvasController.h>
#include <KoShadowConfigWidget.h>
#include <klocale.h>

class ShadowDocker::Private
{
public:
    Private() 
    : widget(0), canvas(0)
    {}
    KoShapeShadow shadow;
    KoShadowConfigWidget * widget;
    KoCanvasBase * canvas;
};

ShadowDocker::ShadowDocker()
: d( new Private() )
{
    setWindowTitle( i18n( "Shadow Properties" ) );

    d->widget = new KoShadowConfigWidget( this );
    setWidget( d->widget );

    connect( d->widget, SIGNAL(shadowColorChanged(const QColor&)), this, SLOT(shadowChanged()));
    connect( d->widget, SIGNAL(shadowOffsetChanged(const QPointF&)), this, SLOT(shadowChanged()));
    connect( d->widget, SIGNAL(shadowVisibilityChanged(const bool)), this, SLOT(shadowChanged()));
}

ShadowDocker::~ShadowDocker()
{
    delete d;
}

void ShadowDocker::selectionChanged()
{
    if( ! d->canvas )
        return;

    KoSelection *selection = d->canvas->shapeManager()->selection();
    KoShape * shape = selection->firstSelectedShape();
    if( ! shape )
    {
        d->widget->setShadowVisible( false );
        return;
    }
    KoShapeShadow * shadow = shape->shadow();
    if( ! shadow )
    {
        d->widget->setShadowVisible( false );
        return;
    }

    d->widget->setShadowVisible( shadow->isVisible() );
    d->widget->setShadowOffset( shadow->offset() );
    d->widget->setShadowColor( shadow->color() );
}

void ShadowDocker::setCanvas( KoCanvasBase *canvas )
{
    d->canvas = canvas;
    if( canvas )
    {
        connect( canvas->shapeManager()->selection(), SIGNAL( selectionChanged() ),
            this, SLOT( selectionChanged() ) );
        d->widget->setUnit( canvas->unit() );
    }
}

void ShadowDocker::shadowChanged()
{
    KoSelection *selection = d->canvas->shapeManager()->selection();
    KoShape * shape = selection->firstSelectedShape();
    if( ! shape )
        return;

    /// TODO use a command here

    KoShapeShadow * oldShadow = shape->shadow();
    if( ! oldShadow && ! d->widget->shadowVisible() )
        return;

    KoShapeShadow * newShadow = 0;
    if( ! oldShadow )
        newShadow = new KoShapeShadow();
    else
        newShadow = oldShadow;

    shape->update();
    newShadow->setVisibility( d->widget->shadowVisible() );
    newShadow->setColor( d->widget->shadowColor() );
    newShadow->setOffset( d->widget->shadowOffset() );
    if( ! oldShadow )
        shape->setShadow( newShadow );
    shape->update();
}

#include "ShadowDocker.moc"
