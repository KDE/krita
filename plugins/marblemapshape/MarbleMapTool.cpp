/* Part of Calligra Suite - Marble Map Shape
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

#include "MarbleMapTool.h"
#include "MarbleMapShape.h"

#include "MarbleMapShapeCommandZoom.h"
#include "MarbleMapShapeCommandChangeProjection.h"
#include "MarbleMapShapeCommandSetMapThemeId.h"
#include "MarbleMapShapeCommandContentChange.h"

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

class MarbleMapToolPrivate{
public:
    MarbleMapToolPrivate()
        :m_marbleMapShape(0),
         m_marbleControlBox(0)//,
         //m_mapThemeManager(0)
    {
    }
    
    ~MarbleMapToolPrivate(){
        //if(m_mapThemeManager)
        //    delete m_mapThemeManager;
    }
    MarbleMapShape *m_marbleMapShape;
    Marble::MarbleControlBox *m_marbleControlBox;
    //Marble::MapThemeManager *m_mapThemeManager;
};

MarbleMapTool::MarbleMapTool(KoCanvasBase* canvas)
    : KoToolBase(canvas),
    d(new MarbleMapToolPrivate)
{
}

MarbleMapTool::~MarbleMapTool()
{
    delete d;
}

void MarbleMapTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    Q_UNUSED(toolActivation);

    foreach (KoShape* shape, shapes) {
        d->m_marbleMapShape = dynamic_cast<MarbleMapShape*>(shape);
        if (d->m_marbleMapShape)
            break;
    }
    if (!d->m_marbleMapShape) {
        emit done();
        return;
    }

    if (d->m_marbleControlBox) {
        d->m_marbleControlBox->addMarbleWidget(d->m_marbleMapShape->marbleWidget());
    }
    useCursor(Qt::ArrowCursor);
}

void MarbleMapTool::deactivate()
{
  d->m_marbleMapShape = 0;
}

QWidget * MarbleMapTool::createOptionWidget()
{

    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(optionWidget);

    d->m_marbleControlBox = new Marble::MarbleControlBox(optionWidget);
    d->m_marbleControlBox->addMarbleWidget(d->m_marbleMapShape->marbleWidget());

    layout->addWidget(d->m_marbleControlBox, 0, 0);

    connect(d->m_marbleMapShape->marbleWidget(), SIGNAL(visibleLatLonAltBoxChanged(Marble::GeoDataLatLonAltBox&)),
            this, SLOT(mapContentChanged(Marble::GeoDataLatLonAltBox&)));

    connect(d->m_marbleMapShape->marbleWidget(), SIGNAL(zoomChanged(int)), this, SLOT(zoomChanged(int)));
    
    connect(d->m_marbleMapShape->marbleWidget(), SIGNAL(projectionChanged(Projection)),
            this, SLOT(setProjection(Projection)));
    connect(d->m_marbleMapShape->marbleWidget(), SIGNAL(setMapThemeId(QString&)),
            this, SLOT(setMapThemeId(const QString&)));

    return optionWidget;
}

// void MarbleMapTool::zoomIn() {
//     canvas()->addCommand(
//         new GeoShapeZoomCommand(m_geoshape, false, +1));
// }
// 
// void MarbleMapTool::zoomOut() {
//     canvas()->addCommand(
//         new GeoShapeZoomCommand(m_geoshape, false, -1));
// }
// 
// void MarbleMapTool::moveLeft() {
//     canvas()->addCommand(
//         new GeoShapeMoveLeftCommand(m_geoshape));
// }
// 
// void MarbleMapTool::moveRight() {
//     canvas()->addCommand(
//         new GeoShapeMoveRightCommand(m_geoshape));
// }
// 
// void MarbleMapTool::moveUp() {
//     canvas()->addCommand(
//         new GeoShapeMoveUpCommand(m_geoshape));
// }
// 
// void MarbleMapTool::moveDown() {
//     canvas()->addCommand(
//         new GeoShapeMoveDownCommand(m_geoshape));
// }
// 

void MarbleMapTool::setProjection(Projection projection) {
    canvas()->addCommand(
        new MarbleMapShapeCommandChangeProjection(d->m_marbleMapShape, projection));
}
 
void MarbleMapTool::setMapThemeId(const QString& theme)
{
    canvas()->addCommand(
        new MarbleMapShapeCommandSetMapThemeId(d->m_marbleMapShape, theme));
}

void MarbleMapTool::zoomChanged(int zoom)
{
    canvas()->addCommand(
        new MarbleMapShapeCommandZoom(d->m_marbleMapShape, zoom));
}

void MarbleMapTool::mapContentChanged(Marble::GeoDataLatLonAltBox& geoData)
{
    QPointF newPos;
    newPos.setX(geoData.center().longitude(Marble::GeoDataCoordinates::Degree));
    newPos.setY(geoData.center().latitude(Marble::GeoDataCoordinates::Degree));
    canvas()->addCommand(
        new MarbleMapShapeCommandContentChange(d->m_marbleMapShape,newPos));
}


// 
// void MarbleMapTool::centerOn(const QModelIndex& index) {
//     // FIXME: needs to be an QUndoCommand
//     if (m_geoshape) {
//         m_geoshape->marbleMap()->centerOn(index);
//         m_geoshape->update();
//     }
// }



void MarbleMapTool::mouseDoubleClickEvent(KoPointerEvent *event) {
    if (canvas()->shapeManager()->shapeAt(event->point) != d->m_marbleMapShape) {
        event->ignore(); // allow the event to be used by another
        return;
    }
    //slotChangeUrl();
/*
    repaintSelection();
    updateSelectionHandler();
*/
}





#include "MarbleMapTool.moc"
