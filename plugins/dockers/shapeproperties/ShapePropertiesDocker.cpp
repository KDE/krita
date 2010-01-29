/* This file is part of the KDE project
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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

#include "ShapePropertiesDocker.h"
#include <KoShape.h>
#include <KoPathShape.h>
#include <KoShapeConfigWidgetBase.h>
#include <KoShapeManager.h>
#include <KoShapeFactoryBase.h>
#include <KoShapeRegistry.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoToolManager.h>
#include <KoSelection.h>
#include <KoParameterShape.h>

#include <klocale.h>

#include <QtGui/QStackedWidget>

class ShapePropertiesDocker::Private {
public:
    Private() : widgetStack(0), currentShape(0), currentPanel(0), canvas(0) {}
    QStackedWidget * widgetStack;
    KoShape * currentShape;
    KoShapeConfigWidgetBase * currentPanel;
    KoCanvasBase * canvas;
};


ShapePropertiesDocker::ShapePropertiesDocker(QWidget *parent)
    : QDockWidget(i18n("Shape Properties"), parent),
    d( new Private() )
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    d->widgetStack = new QStackedWidget();
    setWidget(d->widgetStack);
}

ShapePropertiesDocker::~ShapePropertiesDocker()
{
    delete d;
}

void ShapePropertiesDocker::setCanvas( KoCanvasBase *canvas )
{
    d->canvas = canvas;
    if( d->canvas )
    {
        connect( d->canvas->shapeManager(), SIGNAL( selectionChanged() ),
            this, SLOT( selectionChanged() ) );
        connect( d->canvas->shapeManager(), SIGNAL( selectionContentChanged() ),
            this, SLOT( selectionChanged() ) );
        connect( d->canvas->resourceManager(), SIGNAL( resourceChanged( int, const QVariant& ) ),
            this, SLOT( resourceChanged( int, const QVariant& ) ) );
    }
}

void ShapePropertiesDocker::selectionChanged()
{
    if( ! d->canvas )
        return;

    KoSelection *selection = d->canvas->shapeManager()->selection();
    if( selection->count() == 1 )
        addWidgetForShape( selection->firstSelectedShape() );
    else
        addWidgetForShape( 0 );
}

void ShapePropertiesDocker::addWidgetForShape( KoShape * shape )
{
    // remove the config widget if a null shape is set, or the shape has changed
    if( ! shape || shape != d->currentShape )
    {
        while( d->widgetStack->count() )
            d->widgetStack->removeWidget( d->widgetStack->widget( 0 ) );
    }

    if( ! shape )
    {
        d->currentShape = 0;
        d->currentPanel = 0;
        return;
    }
    else if( shape != d->currentShape )
    {
        // when a shape is set and is differs from the previous one
        // get the config widget and insert it into the option widget
        d->currentShape = shape;
        if( ! d->currentShape )
            return;
        QString shapeId = shape->shapeId();
        KoPathShape * path = dynamic_cast<KoPathShape*>( shape );
        if( path )
        {
            // use the path specific shape id if shape is a path, otherwise use the shape id
            shapeId = path->pathShapeId();
            // check if we have an edited parametric shape, then we use the path shape id
            KoParameterShape * paramShape = dynamic_cast<KoParameterShape*>( shape );
            if( paramShape && ! paramShape->isParametricShape() )
                shapeId = shape->shapeId();
        }
        KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value( shapeId );
        if( ! factory )
            return;
        QList<KoShapeConfigWidgetBase*> panels = factory->createShapeOptionPanels();
        if( ! panels.count() )
            return;

        d->currentPanel = 0;
        uint panelCount = panels.count();
        for( uint i = 0; i < panelCount; ++i )
        {
            if( panels[i]->showOnShapeSelect() ) {
                d->currentPanel = panels[i];
                break;
            }
        }
        if( d->currentPanel )
        {
            if( d->canvas )
                d->currentPanel->setUnit( d->canvas->unit() );
            d->widgetStack->insertWidget( 0, d->currentPanel );
            connect( d->currentPanel, SIGNAL(propertyChanged()),
                     this, SLOT(shapePropertyChanged()));
        }
    }

    if( d->currentPanel )
        d->currentPanel->open( shape );
}

void ShapePropertiesDocker::shapePropertyChanged()
{
    if( d->canvas && d->currentPanel )
    {
        QUndoCommand * cmd = d->currentPanel->createCommand();
        if( ! cmd )
            return;
        d->canvas->addCommand( cmd );
    }
}

void ShapePropertiesDocker::resourceChanged(int key, const QVariant &variant)
{
    if (key == KoCanvasResource::Unit && d->currentPanel)
        d->currentPanel->setUnit(variant.value<KoUnit>());
}

#include <ShapePropertiesDocker.moc>
