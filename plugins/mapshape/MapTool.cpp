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

#include "MapTool.h"
#include "MapShape.h"

#include "MapShapeCommandZoom.h"
#include "MapShapeCommandChangeProjection.h"
#include "MapShapeCommandSetMapThemeId.h"
#include "MapShapeCommandContentChange.h"

#include <QToolButton>
#include <QGridLayout>
#include <QModelIndex>
#include <KLocale>
#include <KIconLoader>
#include <KUrl>
#include <KFileDialog>
#include <KDebug>

#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>

#include <MarbleControlBox.h>
#include <MarbleWidget.h>
#include <GeoDataLatLonAltBox.h>


#include <GeoDataCoordinates.h>

class MapToolPrivate {
public:
    MapToolPrivate()
        :m_shape(0),
         m_controlBox(0)//,
         //m_mapThemeManager(0)
    {
    }
    
    ~MapToolPrivate() {
        //if(m_mapThemeManager)
        //    delete m_mapThemeManager;
    }
    MapShape *m_shape;
    Marble::MarbleControlBox *m_controlBox;
    //Marble::MapThemeManager *m_mapThemeManager;
};

MapTool::MapTool(KoCanvasBase* canvas)
    : KoToolBase(canvas),
    d(new MapToolPrivate)
{
}

MapTool::~MapTool()
{
    delete d;
}

void MapTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    Q_UNUSED(toolActivation);

    foreach (KoShape* shape, shapes) {
        d->m_shape = dynamic_cast<MapShape*>(shape);
        if (d->m_shape)
            break;
    }
    if (!d->m_shape) {
        emit done();
        return;
    }

    if (d->m_controlBox) {
#ifdef HAVE_SETMARBLEWIDGET
        d->m_controlBox->setMarbleWidget(d->m_shape->marbleWidget());
#else
        d->m_controlBox->addMarbleWidget(d->m_shape->marbleWidget());
#endif
    }
    useCursor(Qt::ArrowCursor);
}

void MapTool::deactivate()
{
  d->m_shape = 0;
}

QWidget * MapTool::createOptionWidget()
{

    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(optionWidget);

    d->m_controlBox = new Marble::MarbleControlBox(optionWidget);
#ifdef HAVE_SETMARBLEWIDGET
    d->m_controlBox->setMarbleWidget(d->m_shape->marbleWidget());
#else
    d->m_controlBox->addMarbleWidget(d->m_shape->marbleWidget());
#endif

    layout->addWidget(d->m_controlBox, 0, 0);

    connect(d->m_shape->marbleWidget(), SIGNAL(visibleLatLonAltBoxChanged(Marble::GeoDataLatLonAltBox&)),
            this, SLOT(mapContentChanged(Marble::GeoDataLatLonAltBox&)));

    connect(d->m_shape->marbleWidget(), SIGNAL(zoomChanged(int)), this, SLOT(zoomChanged(int)));
    
    connect(d->m_shape->marbleWidget(), SIGNAL(projectionChanged(Projection)),
            this, SLOT(setProjection(Projection)));
    connect(d->m_shape->marbleWidget(), SIGNAL(setMapThemeId(QString&)),
            this, SLOT(setMapThemeId(const QString&)));

    return optionWidget;
}

// void MapTool::zoomIn() {
//     canvas()->addCommand(
//         new GeoShapeZoomCommand(m_geoshape, false, +1));
// }
// 
// void MapTool::zoomOut() {
//     canvas()->addCommand(
//         new GeoShapeZoomCommand(m_geoshape, false, -1));
// }
// 
// void MapTool::moveLeft() {
//     canvas()->addCommand(
//         new GeoShapeMoveLeftCommand(m_geoshape));
// }
// 
// void MapTool::moveRight() {
//     canvas()->addCommand(
//         new GeoShapeMoveRightCommand(m_geoshape));
// }
// 
// void MapTool::moveUp() {
//     canvas()->addCommand(
//         new GeoShapeMoveUpCommand(m_geoshape));
// }
// 
// void MapTool::moveDown() {
//     canvas()->addCommand(
//         new GeoShapeMoveDownCommand(m_geoshape));
// }
// 

void MapTool::setProjection(Projection projection) {
    canvas()->addCommand(
        new MapShapeCommandChangeProjection(d->m_shape, projection));
}
 
void MapTool::setMapThemeId(const QString& theme)
{
    canvas()->addCommand(
        new MapShapeCommandSetMapThemeId(d->m_shape, theme));
}

void MapTool::zoomChanged(int zoom)
{
    canvas()->addCommand(
        new MapShapeCommandZoom(d->m_shape, zoom));
}

void MapTool::mapContentChanged(Marble::GeoDataLatLonAltBox& geoData)
{
    QPointF newPos;
    newPos.setX(geoData.center().longitude(Marble::GeoDataCoordinates::Degree));
    newPos.setY(geoData.center().latitude(Marble::GeoDataCoordinates::Degree));
    canvas()->addCommand(
        new MapShapeCommandContentChange(d->m_shape,newPos));
}


// 
// void MapTool::centerOn(const QModelIndex& index) {
//     // FIXME: needs to be an QUndoCommand
//     if (m_geoshape) {
//         m_geoshape->marbleMap()->centerOn(index);
//         m_geoshape->update();
//     }
// }



void MapTool::mouseDoubleClickEvent(KoPointerEvent *event) {
    if (canvas()->shapeManager()->shapeAt(event->point) != d->m_shape) {
        event->ignore(); // allow the event to be used by another
        return;
    }
    //slotChangeUrl();
/*
    repaintSelection();
    updateSelectionHandler();
*/
}





#include "MapTool.moc"
