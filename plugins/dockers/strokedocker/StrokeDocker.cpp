/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2005-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 C. Boemann <cbo@boemann.dk>
   Copyright (C) 2006 Peter Simonsson <psn@linux.se>
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <t.zachmann@zagge.de>
   Copyright (C) 2011 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>

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

#include "StrokeDocker.h"

#include <KoStrokeConfigWidget.h>

#include <QLabel>
#include <QRadioButton>
#include <QWidget>
#include <QGridLayout>
#include <QButtonGroup>

#include <klocale.h>

#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoCanvasResourceManager.h>
#include <KoDocumentResourceManager.h>
#include <KoDockFactoryBase.h>
//#include <KoUnitDoubleSpinBox.h>
//#include <KoLineStyleSelector.h>
#include <KoShapeManager.h>
#include <KoShapeStrokeCommand.h>
#include <KoShapeStrokeModel.h>
#include <KoSelection.h>
#include <KoShapeStroke.h>
#include <KoPathShape.h>
#include <KoMarker.h>
#include <KoPathShapeMarkerCommand.h>
#include <KoShapeController.h>
#include <KoMarkerCollection.h>

class StrokeDocker::Private
{
public:
    Private()
        : canvas(0)
        , mainWidget(0)
    {}

    KoMarker *startMarker;
    KoMarker *endMarker;
    KoShapeStroke stroke;
    KoCanvasBase *canvas;
    KoStrokeConfigWidget *mainWidget;
};


StrokeDocker::StrokeDocker()
    : d( new Private() )
{
    setWindowTitle( i18n( "Stroke Properties" ) );

    d->mainWidget = new KoStrokeConfigWidget( this );
    setWidget( d->mainWidget );

    connect( d->mainWidget, SIGNAL(currentIndexChanged()), this, SLOT(styleChanged()));
    connect( d->mainWidget, SIGNAL(widthChanged()),        this, SLOT(widthChanged()));
    connect( d->mainWidget, SIGNAL(capChanged(int)),       this, SLOT(slotCapChanged(int)));
    connect( d->mainWidget, SIGNAL(joinChanged(int)),      this, SLOT(slotJoinChanged(int)));
    connect( d->mainWidget, SIGNAL(miterLimitChanged()),   this, SLOT(miterLimitChanged()));
    connect( d->mainWidget, SIGNAL(currentStartMarkerChanged()), this, SLOT(startMarkerChanged()));
    connect( d->mainWidget, SIGNAL(currentEndMarkerChanged()), this, SLOT(endMarkerChanged()));

    d->mainWidget->updateControls(d->stroke, d->startMarker, d->endMarker);

    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(locationChanged(Qt::DockWidgetArea)));
}

StrokeDocker::~StrokeDocker()
{
    delete d;
}


// ----------------------------------------------------------------
//             Apply changes initiated from the UI


void StrokeDocker::applyChanges()
{
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    canvasController->canvas()->resourceManager()->setActiveStroke( d->stroke );

    d->mainWidget->updateControls(d->stroke, d->startMarker, d->endMarker);

    if (!selection || !selection->count())
        return;

    KoShapeStroke *newStroke = new KoShapeStroke(d->stroke);
    KoShapeStroke *oldStroke = dynamic_cast<KoShapeStroke*>( selection->firstSelectedShape()->stroke() );
    if (oldStroke) {
        newStroke->setColor(oldStroke->color());
        newStroke->setLineBrush(oldStroke->lineBrush());
    }

    KoShapeStrokeCommand *cmd = new KoShapeStrokeCommand(selection->selectedShapes(), newStroke);
    canvasController->canvas()->addCommand(cmd);
}

void StrokeDocker::applyMarkerChanges(KoMarkerData::MarkerPosition position)
{
    KoMarker *marker = 0;
    if (position == KoMarkerData::MarkerStart) {
        marker = d->startMarker;
    }
    else if (position == KoMarkerData::MarkerEnd) {
        marker = d->endMarker;
    }

    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    canvasController->canvas()->resourceManager()->setActiveStroke( d->stroke );

    if (! selection || !selection->count()) {
        return;
    }

    QList<KoShape*> shapeList = selection->selectedShapes();
    QList<KoPathShape*> pathShapeList;
    for (QList<KoShape*>::iterator itShape = shapeList.begin(); itShape != shapeList.end(); ++itShape) {
        KoPathShape* pathShape = dynamic_cast<KoPathShape*>(*itShape);
        if (pathShape) {
            pathShapeList << pathShape;
        }
    }

    if(pathShapeList.size()){
        KoPathShapeMarkerCommand* cmdMarker = new KoPathShapeMarkerCommand(pathShapeList, marker, position);
        canvasController->canvas()->addCommand( cmdMarker );
    }
}


void StrokeDocker::styleChanged()
{
    d->stroke.setLineStyle( d->mainWidget->lineStyle(), d->mainWidget->lineDashes() );
    applyChanges();
}

void StrokeDocker::widthChanged()
{
    d->stroke.setLineWidth( d->mainWidget->lineWidth() );
    applyChanges();
}

void StrokeDocker::slotCapChanged(int ID)
{
    d->stroke.setCapStyle(static_cast<Qt::PenCapStyle>(ID));
    applyChanges();
}

void StrokeDocker::slotJoinChanged( int ID )
{
    d->stroke.setJoinStyle( static_cast<Qt::PenJoinStyle>( ID ) );
    applyChanges();
}

void StrokeDocker::miterLimitChanged()
{
    d->stroke.setMiterLimit( d->mainWidget->miterLimit() );
    applyChanges();
}

void StrokeDocker::startMarkerChanged()
{
    d->startMarker = d->mainWidget->startMarker();
    applyMarkerChanges(KoMarkerData::MarkerStart);
}

void StrokeDocker::endMarkerChanged()
{
    d->endMarker = d->mainWidget->endMarker();
    applyMarkerChanges(KoMarkerData::MarkerEnd);
}
// ----------------------------------------------------------------


void StrokeDocker::setStroke( const KoShapeStrokeModel *stroke )
{
    const KoShapeStroke *lineStroke = dynamic_cast<const KoShapeStroke*>( stroke );
    if (lineStroke) {
        d->stroke.setLineWidth( lineStroke->lineWidth() );
        d->stroke.setCapStyle( lineStroke->capStyle() );
        d->stroke.setJoinStyle( lineStroke->joinStyle() );
        d->stroke.setMiterLimit( lineStroke->miterLimit() );
        d->stroke.setLineStyle( lineStroke->lineStyle(), lineStroke->lineDashes() );
    }
    else {
        d->stroke.setLineWidth( 0.0 );
        d->stroke.setCapStyle( Qt::FlatCap );
        d->stroke.setJoinStyle( Qt::MiterJoin );
        d->stroke.setMiterLimit( 0.0 );
        d->stroke.setLineStyle( Qt::NoPen, QVector<qreal>() );
    }
}

void StrokeDocker::setUnit(KoUnit unit)
{
    d->mainWidget->setUnit(unit);
}

void StrokeDocker::selectionChanged()
{
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    KoShape * shape = selection->firstSelectedShape();
    if (shape) {
        setStroke(shape->stroke());
        KoPathShape *pathShape = dynamic_cast<KoPathShape *>(shape);
        if (pathShape) {
            d->startMarker = pathShape->marker(KoMarkerData::MarkerStart);
            d->endMarker = pathShape->marker(KoMarkerData::MarkerEnd);
        }
        else {
            d->startMarker = 0;
            d->endMarker = 0;
        }
        d->mainWidget->updateControls(d->stroke, d->startMarker, d->endMarker);
    }
}

void StrokeDocker::setCanvas( KoCanvasBase *canvas )
{
    if (d->canvas) {
        d->canvas->disconnectCanvasObserver(this); // "Every connection you make emits a signal, so duplicate connections emit two signals"
    }

    if (canvas) {
        connect(canvas->shapeManager()->selection(), SIGNAL(selectionChanged()),
                this, SLOT(selectionChanged()));
        connect(canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant&)),
                this, SLOT(resourceChanged(int, const QVariant&)));
        setUnit(canvas->unit());
    }

    d->canvas = canvas;
    KoDocumentResourceManager *resourceManager = canvas->shapeController()->resourceManager();
    if (resourceManager) {
        KoMarkerCollection *collection = resourceManager->resource(KoDocumentResourceManager::MarkerCollection).value<KoMarkerCollection*>();
        if (collection) {
            d->mainWidget->updateMarkers(collection->markers());

        }
    }
}

void StrokeDocker::unsetCanvas()
{
    d->canvas = 0;
}

void StrokeDocker::resourceChanged(int key, const QVariant &value)
{
    switch (key) {
    case KoCanvasResourceManager::Unit:
        setUnit(value.value<KoUnit>());
        break;
    }
}

void StrokeDocker::locationChanged(Qt::DockWidgetArea area)
{
    d->mainWidget->locationChanged(area);
}

#include <StrokeDocker.moc>
