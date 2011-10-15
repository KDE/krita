/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2005-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2006 Peter Simonsson <psn@linux.se>
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <t.zachmann@zagge.de>

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
#include <kiconloader.h>

#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoCanvasResourceManager.h>
#include <KoDockFactoryBase.h>
//#include <KoUnitDoubleSpinBox.h>
//#include <KoLineStyleSelector.h>
#include <KoShapeManager.h>
#include <KoShapeBorderCommand.h>
#include <KoShapeBorderModel.h>
#include <KoSelection.h>
#include <KoLineBorder.h>


class StrokeDocker::Private
{
public:
    Private()
        : canvas(0)
        , mainWidget(0)
    {}

    KoLineBorder border;
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

    d->mainWidget->updateControls(d->border);

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

    canvasController->canvas()->resourceManager()->setActiveBorder( d->border );

    d->mainWidget->updateControls(d->border);

    if (!selection || !selection->count())
        return;

    KoLineBorder *newBorder = new KoLineBorder(d->border);
    KoLineBorder *oldBorder = dynamic_cast<KoLineBorder*>( selection->firstSelectedShape()->border() );
    if (oldBorder) {
        newBorder->setColor(oldBorder->color());
        newBorder->setLineBrush(oldBorder->lineBrush());
    }

    KoShapeBorderCommand *cmd = new KoShapeBorderCommand(selection->selectedShapes(), newBorder);
    canvasController->canvas()->addCommand(cmd);
}


void StrokeDocker::styleChanged()
{
    d->border.setLineStyle( d->mainWidget->lineStyle(), d->mainWidget->lineDashes() );
    applyChanges();
}

void StrokeDocker::widthChanged()
{
    d->border.setLineWidth( d->mainWidget->lineWidth() );
    applyChanges();
}

void StrokeDocker::slotCapChanged(int ID)
{
    d->border.setCapStyle(static_cast<Qt::PenCapStyle>(ID));
    applyChanges();
}

void StrokeDocker::slotJoinChanged( int ID )
{
    d->border.setJoinStyle( static_cast<Qt::PenJoinStyle>( ID ) );
    applyChanges();
}

void StrokeDocker::miterLimitChanged()
{
    d->border.setMiterLimit( d->mainWidget->miterLimit() );
    applyChanges();
}

// ----------------------------------------------------------------


void StrokeDocker::setStroke( const KoShapeBorderModel *border )
{
    const KoLineBorder *lineBorder = dynamic_cast<const KoLineBorder*>( border );
    if (lineBorder) {
        d->border.setLineWidth( lineBorder->lineWidth() );
        d->border.setCapStyle( lineBorder->capStyle() );
        d->border.setJoinStyle( lineBorder->joinStyle() );
        d->border.setMiterLimit( lineBorder->miterLimit() );
        d->border.setLineStyle( lineBorder->lineStyle(), lineBorder->lineDashes() );
    }
    else {
        d->border.setLineWidth( 0.0 );
        d->border.setCapStyle( Qt::FlatCap );
        d->border.setJoinStyle( Qt::MiterJoin );
        d->border.setMiterLimit( 0.0 );
        d->border.setLineStyle( Qt::NoPen, QVector<qreal>() );
    }

    d->mainWidget->updateControls(d->border);
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
    if (shape)
        setStroke(shape->border());
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

