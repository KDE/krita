/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2005-2007 Jan Hambrecht <jaham@gmx.net>
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

#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoDockFactory.h>
#include <KoUnitDoubleSpinBox.h>
#include <KoShapeManager.h>
#include <KoShapeBorderCommand.h>
#include <KoShapeBorderModel.h>
#include <KoSelection.h>
#include <KoLineBorder.h>
#include <KoLineStyleSelector.h>

#include <kiconloader.h>
#include <klocale.h>

#include <QLabel>
#include <QRadioButton>
#include <QWidget>
#include <QGridLayout>
#include <QButtonGroup>

class StrokeDocker::Private
{
public:
    Private() {}
    QButtonGroup * capGroup;
    QButtonGroup * joinGroup;
    KoUnitDoubleSpinBox * setLineWidth;
    KoUnitDoubleSpinBox * miterLimit;
    KoLineStyleSelector * lineStyle;
    KoLineBorder border;
};

StrokeDocker::StrokeDocker()
    : d( new Private() )
{
    setWindowTitle( i18n( "Stroke Properties" ) );

    QWidget *mainWidget = new QWidget( this );
    QGridLayout *mainLayout = new QGridLayout( mainWidget );

    QLabel* widthLabel = new QLabel( i18n ( "Width:" ), mainWidget );
    mainLayout->addWidget( widthLabel, 0, 0 );
    // set min/max/step and value in points, then set actual unit
    d->setLineWidth = new KoUnitDoubleSpinBox( mainWidget );
    d->setLineWidth->setMinMaxStep( 0.0, 1000.0, 0.5 );
    d->setLineWidth->setDecimals( 1 );
    d->setLineWidth->setUnit( KoUnit(KoUnit::Point) );
    d->setLineWidth->setToolTip( i18n( "Set line width of actual selection" ) );
    mainLayout->addWidget( d->setLineWidth, 0, 1, 1, 3 );
    connect( d->setLineWidth, SIGNAL( valueChanged( double ) ), this, SLOT( widthChanged() ) );

    QLabel* capLabel = new QLabel( i18n ( "Cap:" ), mainWidget );
    mainLayout->addWidget( capLabel, 1, 0 );
    d->capGroup = new QButtonGroup( mainWidget );
    d->capGroup->setExclusive( true );
    d->capGroup->setExclusive( true );

    QRadioButton *button = 0;

    button = new QRadioButton( mainWidget );
    button->setIcon( SmallIcon( "cap_butt" ) );
    button->setCheckable( true );
    button->setToolTip( i18n( "Butt cap" ) );
    d->capGroup->addButton( button, Qt::FlatCap );
    mainLayout->addWidget( button, 1, 1 );

    button = new QRadioButton( mainWidget );
    button->setIcon( SmallIcon( "cap_round" ) );
    button->setCheckable( true );
    button->setToolTip( i18n( "Round cap" ) );
    d->capGroup->addButton( button, Qt::RoundCap );
    mainLayout->addWidget( button, 1, 2 );

    button = new QRadioButton( mainWidget );
    button->setIcon( SmallIcon( "cap_square" ) );
    button->setCheckable( true );
    button->setToolTip( i18n( "Square cap" ) );
    d->capGroup->addButton( button, Qt::SquareCap );
    mainLayout->addWidget( button, 1, 3 );

    connect( d->capGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( slotCapChanged( int ) ) );

    QLabel* joinLabel = new QLabel( i18n ( "Join:" ), mainWidget );
    mainLayout->addWidget( joinLabel, 2, 0 );

    d->joinGroup = new QButtonGroup( mainWidget );
    d->joinGroup->setExclusive( true );

    button = new QRadioButton( mainWidget );
    button->setIcon( SmallIcon( "join_miter" ) );
    button->setCheckable( true );
    button->setToolTip( i18n( "Miter join" ) );
    d->joinGroup->addButton( button, Qt::MiterJoin );
    mainLayout->addWidget( button, 2, 1 );

    button = new QRadioButton( mainWidget );
    button->setIcon( SmallIcon( "join_round" ) );
    button->setCheckable( true );
    button->setToolTip( i18n( "Round join" ) );
    d->joinGroup->addButton( button, Qt::RoundJoin );
    mainLayout->addWidget( button, 2, 2 );

    button = new QRadioButton( mainWidget );
    button->setIcon( SmallIcon( "join_bevel" ) );
    button->setCheckable( true );
    button->setToolTip( i18n( "Bevel join" ) );
    d->joinGroup->addButton( button, Qt::BevelJoin );
    mainLayout->addWidget( button, 2, 3 );

    connect( d->joinGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( slotJoinChanged( int ) ) );

    QLabel* miterLabel = new QLabel( i18n ( "Miter limit:" ), mainWidget );
    mainLayout->addWidget( miterLabel, 3, 0 );
    // set min/max/step and value in points, then set actual unit
    d->miterLimit = new KoUnitDoubleSpinBox( mainWidget );
    d->miterLimit->setMinMaxStep( 0.0, 1000.0, 0.5 );
    d->miterLimit->setDecimals( 1 );
    d->miterLimit->setUnit( KoUnit(KoUnit::Point) );
    d->miterLimit->setToolTip( i18n( "Set miter limit" ) );
    mainLayout->addWidget( d->miterLimit, 3, 1, 1, 3 );
    connect( d->miterLimit, SIGNAL( valueChanged( double ) ), this, SLOT( miterLimitChanged() ) );

    QLabel * styleLabel = new QLabel( i18n( "Style:" ), mainWidget );
    mainLayout->addWidget( styleLabel, 4, 0 );
    d->lineStyle = new KoLineStyleSelector( mainWidget );
    mainLayout->addWidget( d->lineStyle, 4, 1, 1, 3 );

    connect( d->lineStyle, SIGNAL(currentIndexChanged( int ) ), this, SLOT( styleChanged() ) );

    mainLayout->setRowStretch( 5, 1 );
    mainLayout->setColumnStretch( 1, 1 );
    mainLayout->setColumnStretch( 2, 1 );
    mainLayout->setColumnStretch( 3, 1 );

    setWidget( mainWidget );

    updateControls();
}

StrokeDocker::~StrokeDocker()
{
    delete d;
}

void StrokeDocker::applyChanges()
{
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    if( ! selection || ! selection->count() )
        return;

    KoLineBorder * newBorder = new KoLineBorder(d->border);
    KoLineBorder * oldBorder = dynamic_cast<KoLineBorder*>( selection->firstSelectedShape()->border() );
    if( oldBorder )
        newBorder->setColor( oldBorder->color() );

    KoShapeBorderCommand *cmd = new KoShapeBorderCommand( selection->selectedShapes(), newBorder );
    canvasController->canvas()->addCommand( cmd );
}

void StrokeDocker::slotCapChanged( int ID )
{
    d->border.setCapStyle( static_cast<Qt::PenCapStyle>( ID ) );
    applyChanges();
}

void StrokeDocker::slotJoinChanged( int ID )
{
    d->border.setJoinStyle( static_cast<Qt::PenJoinStyle>( ID ) );
    applyChanges();
}

void StrokeDocker::updateControls()
{
    blockChildSignals( true );

    d->capGroup->button( d->border.capStyle() )->setChecked( true );
    d->joinGroup->button( d->border.joinStyle() )->setChecked( true );
    d->setLineWidth->changeValue( d->border.lineWidth() );
    d->miterLimit->changeValue( d->border.miterLimit() );
    d->lineStyle->setLineStyle( d->border.lineStyle(), d->border.lineDashes() );

    blockChildSignals( false );
}

void StrokeDocker::widthChanged()
{
    d->border.setLineWidth( d->setLineWidth->value() );
    applyChanges();
}

void StrokeDocker::miterLimitChanged()
{
    d->border.setMiterLimit( d->miterLimit->value() );
    applyChanges();
}

void StrokeDocker::styleChanged()
{
    d->border.setLineStyle( d->lineStyle->lineStyle(), d->lineStyle->lineDashes() );
    applyChanges();
}

void StrokeDocker::setStroke( const KoShapeBorderModel *border )
{
    const KoLineBorder *lineBorder = dynamic_cast<const KoLineBorder*>( border );
    if( lineBorder )
    {
        d->border.setLineWidth( lineBorder->lineWidth() );
        d->border.setCapStyle( lineBorder->capStyle() );
        d->border.setJoinStyle( lineBorder->joinStyle() );
        d->border.setMiterLimit( lineBorder->miterLimit() );
        d->border.setLineStyle( lineBorder->lineStyle(), lineBorder->lineDashes() );
    }
    else
    {
        d->border.setLineWidth( 0.0 );
        d->border.setCapStyle( Qt::FlatCap );
        d->border.setJoinStyle( Qt::MiterJoin );
        d->border.setMiterLimit( 0.0 );
        d->border.setLineStyle( Qt::NoPen, QVector<qreal>() );
    }
    updateControls();
}

void StrokeDocker::setUnit( KoUnit unit )
{
    // TODO this has to be connect to a unit changed signal
    blockChildSignals( true );
    d->setLineWidth->setUnit( unit );
    d->miterLimit->setUnit( unit );
    blockChildSignals( false );
}

void StrokeDocker::blockChildSignals( bool block )
{
    d->setLineWidth->blockSignals( block );
    d->capGroup->blockSignals( block );
    d->joinGroup->blockSignals( block );
    d->miterLimit->blockSignals( block );
    d->lineStyle->blockSignals( block );
}

void StrokeDocker::selectionChanged()
{
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    KoShape * shape = selection->firstSelectedShape();
    if( shape )
        setStroke( shape->border() );
}

void StrokeDocker::setCanvas( KoCanvasBase *canvas )
{
    if( canvas )
    {
        connect( canvas->shapeManager()->selection(), SIGNAL( selectionChanged() ), this, SLOT( selectionChanged() ) );
        setUnit( canvas->unit() );
    }
}

// #include "StrokeDocker.moc"

