/* This file is part of the KDE project
 * Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "DefaultToolWidget.h"
#include "DefaultTool.h"

#include <KoInteractionTool.h>
#include <KoCanvasBase.h>
#include <KoResourceManager.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <commands/KoShapeMoveCommand.h>
#include <commands/KoShapeSizeCommand.h>
#include <commands/KoShapeTransformCommand.h>
#include <KoPositionSelector.h>
#include "SelectionDecorator.h"
#include "DefaultToolTransformWidget.h"

#include <KAction>
#include <QSize>
#include <QtGui/QRadioButton>
#include <QtGui/QLabel>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QList>
#include <QTransform>

DefaultToolWidget::DefaultToolWidget( KoInteractionTool* tool,
                                    QWidget* parent )
    : QWidget( parent ),
    m_tool(tool),
    m_blockSignals(false)
{
    setupUi( this );

    setUnit( m_tool->canvas()->unit() );

    aspectButton->setKeepAspectRatio( false );
    updatePosition();
    updateSize();

    connect( positionSelector, SIGNAL( positionSelected(KoFlake::Position) ), 
        this, SLOT( positionSelected(KoFlake::Position) ) );

    connect( positionXSpinBox, SIGNAL( editingFinished() ), this, SLOT( positionHasChanged() ) );
    connect( positionYSpinBox, SIGNAL( editingFinished() ), this, SLOT( positionHasChanged() ) );

    connect( widthSpinBox, SIGNAL( editingFinished() ), this, SLOT( sizeHasChanged() ) );
    connect( heightSpinBox, SIGNAL( editingFinished() ), this, SLOT( sizeHasChanged() ) );

    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    connect( selection, SIGNAL( selectionChanged() ), this, SLOT( updatePosition() ) );
    connect( selection, SIGNAL( selectionChanged() ), this, SLOT( updateSize() ) );
    KoShapeManager * manager = m_tool->canvas()->shapeManager();
    connect( manager, SIGNAL( selectionContentChanged() ), this, SLOT( updatePosition() ) );
    connect( manager, SIGNAL( selectionContentChanged() ), this, SLOT( updateSize() ) );

    connect( m_tool->canvas()->resourceManager(), SIGNAL( resourceChanged( int, const QVariant& ) ),
        this, SLOT( resourceChanged( int, const QVariant& ) ) );

    connect (aspectButton, SIGNAL(keepAspectRatioChanged(bool)),
        this, SLOT(aspectButtonToggled(bool)));
}

void DefaultToolWidget::positionSelected( KoFlake::Position position )
{
    m_tool->canvas()->resourceManager()->setResource( DefaultTool::HotPosition, QVariant(position) );
    updatePosition();
}

void DefaultToolWidget::updatePosition()
{
    QPointF selPosition( 0, 0 );
    KoFlake::Position position = positionSelector->position();

    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    if( selection->count() )
        selPosition = selection->absolutePosition( position );

    positionXSpinBox->setEnabled( selection->count() );
    positionYSpinBox->setEnabled( selection->count() );

    if (m_blockSignals)
        return;
    m_blockSignals = true;
    positionXSpinBox->changeValue( selPosition.x() );
    positionYSpinBox->changeValue( selPosition.y() );

    QList<KoShape*> selectedShapes = selection->selectedShapes( KoFlake::TopLevelSelection );
    bool aspectLocked = false;
    foreach (KoShape* shape, selectedShapes)
        aspectLocked = aspectLocked | shape->keepAspectRatio();
    aspectButton->setKeepAspectRatio(aspectLocked);
    m_blockSignals = false;
}

void DefaultToolWidget::positionHasChanged()
{
    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    if( ! selection->count() )
        return;

    KoFlake::Position position = positionSelector->position();
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

void DefaultToolWidget::updateSize()
{
    QSizeF selSize( 0, 0 );
    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    uint selectionCount = selection->count();
    if( selectionCount )
        selSize = selection->boundingRect().size();

    widthSpinBox->setEnabled( selectionCount );
    heightSpinBox->setEnabled( selectionCount );

    if (m_blockSignals)
        return;
    m_blockSignals = true;
    widthSpinBox->changeValue( selSize.width() );
    heightSpinBox->changeValue( selSize.height() );
    m_blockSignals = false;
}

void DefaultToolWidget::sizeHasChanged()
{
    if (aspectButton->hasFocus())
        return;
    if (m_blockSignals)
        return;

    QSizeF newSize( widthSpinBox->value(), heightSpinBox->value() );

    KoSelection *selection = m_tool->canvas()->shapeManager()->selection();
    QRectF rect = selection->boundingRect();

    if( aspectButton->keepAspectRatio() )
    {
        qreal aspect = rect.width() / rect.height();
        if( rect.width() != newSize.width() )
            newSize.setHeight( newSize.width() / aspect );
        else if( rect.height() != newSize.height() )
            newSize.setWidth( newSize.height() * aspect );
    }

    if( rect.width() != newSize.width() || rect.height() != newSize.height() )
    {
        // get the scale/resize center from the position selector
        QPointF scaleCenter = selection->absolutePosition( positionSelector->position() );

        QTransform resizeMatrix;
        resizeMatrix.translate( scaleCenter.x(), scaleCenter.y() );
        resizeMatrix.scale( newSize.width() / rect.width(), newSize.height() / rect.height() );
        resizeMatrix.translate( -scaleCenter.x(), -scaleCenter.y() );

        QList<KoShape*> selectedShapes = selection->selectedShapes( KoFlake::StrippedSelection );
        QList<QSizeF> oldSizes, newSizes;
        QList<QTransform> oldState;
        QList<QTransform> newState;

        foreach( KoShape* shape, selectedShapes )
        {
            shape->update();
            QSizeF oldSize = shape->size();
            oldState << shape->transformation();
            QTransform shapeMatrix = shape->absoluteTransformation(0);

            // calculate the matrix we would apply to the local shape matrix
            // that tells us the effective scale values we have to use for the resizing
            QTransform localMatrix = shapeMatrix * resizeMatrix * shapeMatrix.inverted();
            // save the effective scale values
            qreal scaleX = localMatrix.m11();
            qreal scaleY = localMatrix.m22();

            // calculate the scale matrix which is equivalent to our resizing above
            QTransform scaleMatrix = (QTransform().scale( scaleX, scaleY ));
            scaleMatrix =  shapeMatrix.inverted() * scaleMatrix * shapeMatrix;

            // calculate the new size of the shape, using the effective scale values
            oldSizes << oldSize;
            QSizeF newSize = QSizeF( scaleX * oldSize.width(), scaleY * oldSize.height() );
            newSizes << newSize;
            shape->setSize( newSize );

            // apply the rest of the transformation without the resizing part
            shape->applyAbsoluteTransformation( scaleMatrix.inverted() * resizeMatrix );
            newState << shape->transformation();
        }
        m_tool->repaintDecorations();
        selection->applyAbsoluteTransformation( resizeMatrix );
        QUndoCommand * cmd = new QUndoCommand(i18n("Resize"));
        new KoShapeSizeCommand( selectedShapes, oldSizes, newSizes, cmd );
        new KoShapeTransformCommand( selectedShapes, oldState, newState, cmd );
        m_tool->canvas()->addCommand( cmd );
        updateSize();
        updatePosition();
    }
}

void DefaultToolWidget::setUnit( const KoUnit &unit )
{
    m_blockSignals = true;
    positionXSpinBox->setUnit( unit );
    positionYSpinBox->setUnit( unit );
    widthSpinBox->setUnit( unit );
    heightSpinBox->setUnit( unit );
    m_blockSignals = false;
}

void DefaultToolWidget::resourceChanged( int key, const QVariant & res )
{
    if( key == KoCanvasResource::Unit )
        setUnit(res.value<KoUnit>());
    else if( key == DefaultTool::HotPosition )
    {
        if( res.toInt() != positionSelector->position() )
        {
            positionSelector->setPosition( static_cast<KoFlake::Position>( res.toInt() ) );
            updatePosition();
        }
    }
}

void DefaultToolWidget::aspectButtonToggled(bool keepAspect)
{
    if (m_blockSignals)
        return;
    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    foreach (KoShape *shape, selection->selectedShapes(KoFlake::TopLevelSelection)) {
        shape->setKeepAspectRatio(keepAspect);
    }
}

#include <DefaultToolWidget.moc>
