/* This file is part of the KDE project
 * Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>
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

#include "KoInteractionToolWidget.h"
#include "KoInteractionTool.h"
#include "KoCanvasBase.h"
#include "KoShapeManager.h"
#include "KoSelection.h"
#include "commands/KoShapeMoveCommand.h"

#include <QPainter>
#include <QSize>
#include <QtGui/QRadioButton>
#include <QtGui/QLabel>

KoInteractionToolWidget::KoInteractionToolWidget( KoInteractionTool* tool,
                                    QWidget* parent ) : QTabWidget( parent )
{
    m_tool = tool;

    setupUi( this );

    topLeft = new QRadioButton( "", rectWidget );
    topLeft->setChecked( true );
    topLeft->setToolTip( i18n( "Top-Left Corner" ) );
    topRight = new QRadioButton( "", rectWidget );
    topRight->setToolTip( i18n( "Top-Right Corner" ) );
    bottomLeft = new QRadioButton( "", rectWidget );
    bottomLeft->setToolTip( i18n( "Bottom-Left Corner" ) );
    bottomRight = new QRadioButton( "", rectWidget );
    bottomRight->setToolTip( i18n( "Bottom-Right Corner" ) );
    center = new QRadioButton( "", rectWidget );
    center->setToolTip( i18n( "Center Point" ) );

    QGridLayout * g = new QGridLayout( rectWidget );
    g->addWidget( topLeft, 0, 0, 1, 1, Qt::AlignLeft );
    g->addWidget( topRight, 0, 2, 1, 1, Qt::AlignRight );
    g->addWidget( center, 1, 1, 1, 1, Qt::AlignCenter );
    g->addWidget( bottomLeft, 2, 0, 1, 1, Qt::AlignLeft );
    g->addWidget( bottomRight, 2, 2, 1, 1, Qt::AlignRight );

    g->setRowStretch( 0, 0 );
    g->setRowStretch( 1, 1 );
    g->setRowStretch( 2, 0 );
    g->setColumnStretch( 0, 0 );
    g->setColumnStretch( 1, 1 );
    g->setColumnStretch( 2, 0 );

    connect( topLeft, SIGNAL(clicked()), this, SLOT(updatePosition()) );
    connect( topRight, SIGNAL(clicked()), this, SLOT(updatePosition()) );
    connect( center, SIGNAL(clicked()), this, SLOT(updatePosition()) );
    connect( bottomLeft, SIGNAL(clicked()), this, SLOT(updatePosition()) );
    connect( bottomRight, SIGNAL(clicked()), this, SLOT(updatePosition()) );

    connect( positionXSpinBox, SIGNAL( editingFinished() ), this, SLOT( positionHasChanged() ) );
    connect( positionYSpinBox, SIGNAL( editingFinished() ), this, SLOT( positionHasChanged() ) );

    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    connect( selection, SIGNAL( selectionChanged() ), this, SLOT( updatePosition() ) );

    rectWidget->installEventFilter( this );

    bringToFront->setDefaultAction( m_tool->action( "object_move_totop" ) );
    raiseLevel->setDefaultAction( m_tool->action( "object_move_up" ) );
    lowerLevel->setDefaultAction( m_tool->action( "object_move_down" ) );
    sendBack->setDefaultAction( m_tool->action( "object_move_up" ) );
    bottomAlign->setDefaultAction( m_tool->action( "object_align_vertical_bottom" ) );
    vCenterAlign->setDefaultAction( m_tool->action( "object_align_vertical_center" ) );
    topAlign->setDefaultAction( m_tool->action( "object_align_vertical_top" ) );
    rightAlign->setDefaultAction( m_tool->action( "object_align_horizontal_right" ) );
    hCenterAlign->setDefaultAction( m_tool->action( "object_align_horizontal_center" ) );
    leftAlign->setDefaultAction( m_tool->action( "object_align_horizontal_left" ) );
}

bool KoInteractionToolWidget::eventFilter( QObject* object, QEvent* event )
{
    if( event->type() == QEvent::MouseButtonPress ) {
        return true;
    }
    else if( event->type() == QEvent::Paint ) {
        QPainter p( rectWidget );
        int offset = topLeft->height() >> 1;
        int left = topLeft->pos().x() + offset;
        int right = topRight->pos().x() + offset;
        int top = topLeft->pos().y() + offset;
        int bottom = bottomLeft->pos().y() + offset;
        QRect r( left, top, right-left, bottom-top );
        p.drawRect( r );
        return true;
    }
    else
        return QObject::eventFilter( object, event ); // standart event processing
}

KoFlake::Position KoInteractionToolWidget::selectedPosition()
{
    KoFlake::Position position = KoFlake::TopLeftCorner;
    if( topRight->isChecked() )
        position = KoFlake::TopRightCorner;
    else if( bottomLeft->isChecked() )
        position = KoFlake::BottomLeftCorner;
    else if( bottomRight->isChecked() )
        position = KoFlake::BottomRightCorner;
    else if( center->isChecked() )
        position = KoFlake::CenteredPosition;

    return position;
}

void KoInteractionToolWidget::updatePosition()
{
    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    if( selection->count() )
    {
        KoFlake::Position position = selectedPosition();
        QPointF p = selection->absolutePosition( position );

        positionXSpinBox->blockSignals(true);
        positionYSpinBox->blockSignals(true);
        positionXSpinBox->setValue( p.x() );
        positionYSpinBox->setValue( p.y() );
        positionXSpinBox->blockSignals(false);
        positionYSpinBox->blockSignals(false);

        emit hotPositionChanged( position );
    }
}

void KoInteractionToolWidget::positionHasChanged()
{
    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    if( ! selection->count() )
        return;

    KoFlake::Position position = selectedPosition();
    QPointF newPos( positionXSpinBox->value(), positionYSpinBox->value() );
    QPointF oldPos = selection->absolutePosition( position );
    if( oldPos == newPos )
        return;

    QList<KoShape*> selectedShapes = selection->selectedShapes( KoFlake::TopLevelSelection );
    QPointF moveBy = newPos - oldPos;
    QList<QPointF> oldPositions;
    QList<QPointF> newPositions;
    foreach( KoShape* shape, selectedShapes )
    {
        oldPositions.append( shape->position() );
        newPositions.append( shape->position() + moveBy );
    }
    selection->setPosition( selection->position() + moveBy );
    m_tool->canvas()->addCommand( new KoShapeMoveCommand( selectedShapes, oldPositions, newPositions ) );
    updatePosition();
}

#include <KoInteractionToolWidget.moc>
