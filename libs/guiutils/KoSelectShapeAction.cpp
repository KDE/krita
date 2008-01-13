/*  This file is part of the KDE libraries
    Copyright (C) 2008 Martin Pfeiffer <hubipete@gmx.net>

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

#include "KoSelectShapeAction.h"

#include <QToolButton>
#include <QMenu>
#include <KoShapeRegistry.h>
#include <kicon.h>

KoSelectShapeAction::KoSelectShapeAction( QObject* parent ) : QWidgetAction( parent )
{}

QWidget* KoSelectShapeAction::createWidget( QWidget* parent )
{
    QToolButton* toolButton = new QToolButton( parent );
    toolButton->setPopupMode( QToolButton::InstantPopup );
    toolButton->setMenu( createShapeMenu() );
    return toolButton;
}

QMenu* KoSelectShapeAction::createShapeMenu()
{
    // get all available shapes from the registry and fill the menu
    QMenu* menu = new QMenu();
    foreach( QString shapeId, KoShapeRegistry::instance()->keys() ) {
        KoShapeFactory* factory = KoShapeRegistry::instance()->value( shapeId );

        // if there are templates create a submenu
        if( !factory->templates().isEmpty() ) {
            QMenu* tmp = menu->addMenu( KIcon( factory->icon() ), factory->name() );
            tmp->setToolTip( factory->toolTip() );
            foreach( KoShapeTemplate shapeTemplate, factory->templates() ) {
                QAction* action = tmp->addAction( shapeTemplate.name );
                action->setIcon( KIcon( shapeTemplate.icon ) );
                action->setToolTip( shapeTemplate.toolTip );
                action->setData( shapeTemplate.id );
            }
        }
        else {
            QAction* action = menu->addAction( factory->name() );
            action->setIcon( KIcon( factory->icon() ) );
            action->setToolTip( factory->toolTip() );
            action->setData( factory->id() );
        }
    }
    connect( menu, SIGNAL( triggered( QAction* ) ), this, SLOT( addShape( QAction* ) ) );
    return menu;
}

void KoSelectShapeAction::addShape( QAction* action )
{
    Q_UNUSED( action )
}

/*
void KoSelectShapeAction::startDrag()
{
    QPixmap shapePreview;
    QPainter p(  shapePreview );
    KoShape::paint(  &p, activeCanvas.viewConverter() );

    QDrag drag;
    drag.setHotSpot(  QPoint(  shapePreview.width() / 2, shapePreivew.height() / 2 ) );
    drag.setPixmap(  shapePreview );
}*/

#include "KoSelectShapeAction.moc"
