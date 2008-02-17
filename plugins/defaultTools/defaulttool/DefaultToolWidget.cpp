/* This file is part of the KDE project
 * Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include <KoInteractionTool.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <commands/KoShapeMoveCommand.h>
#include <commands/KoShapeSizeCommand.h>
#include <commands/KoShapeTransformCommand.h>
#include "SelectionDecorator.h"

#include <QSize>
#include <QtGui/QRadioButton>
#include <QtGui/QLabel>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QList>
#include <QMatrix>

DefaultToolWidget::DefaultToolWidget( KoInteractionTool* tool,
                                    QWidget* parent ) : QTabWidget( parent )
{
    m_tool = tool;

    setupUi( this );

    setUnit( m_tool->canvas()->unit() );

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

    connect( m_tool->canvas()->resourceProvider(), SIGNAL( resourceChanged( int, const QVariant& ) ),
        this, SLOT( resourceChanged( int, const QVariant& ) ) );

    bringToFront->setDefaultAction( m_tool->action( "object_move_totop" ) );
    raiseLevel->setDefaultAction( m_tool->action( "object_move_up" ) );
    lowerLevel->setDefaultAction( m_tool->action( "object_move_down" ) );
    sendBack->setDefaultAction( m_tool->action( "object_move_tobottom" ) );
    bottomAlign->setDefaultAction( m_tool->action( "object_align_vertical_bottom" ) );
    vCenterAlign->setDefaultAction( m_tool->action( "object_align_vertical_center" ) );
    topAlign->setDefaultAction( m_tool->action( "object_align_vertical_top" ) );
    rightAlign->setDefaultAction( m_tool->action( "object_align_horizontal_right" ) );
    hCenterAlign->setDefaultAction( m_tool->action( "object_align_horizontal_center" ) );
    leftAlign->setDefaultAction( m_tool->action( "object_align_horizontal_left" ) );

    aspectButton->setChecked( false );

    connect( rotateButton, SIGNAL( clicked() ), this, SLOT( rotationChanged() ) );
    connect( shearXButton, SIGNAL( clicked() ), this, SLOT( shearXChanged() ) );
    connect( shearYButton, SIGNAL( clicked() ), this, SLOT( shearYChanged() ) );
    connect( scaleXButton, SIGNAL( clicked() ), this, SLOT( scaleXChanged() ) );
    connect( scaleYButton, SIGNAL( clicked() ), this, SLOT( scaleYChanged() ) );
    connect( scaleAspectCheckBox, SIGNAL( toggled( bool ) ), scaleYSpinBox, SLOT( setDisabled( bool ) ) );
    connect( scaleAspectCheckBox, SIGNAL( toggled( bool ) ), scaleYButton, SLOT( setDisabled( bool ) ) );
    connect( resetButton, SIGNAL( clicked() ), this, SLOT( resetTransformations() ) );

    updatePosition();
    updateSize();
}

void DefaultToolWidget::positionSelected( KoFlake::Position position )
{
    m_tool->canvas()->resourceProvider()->setResource( KoCanvasResource::HotPosition, QVariant(position) );
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

    positionXSpinBox->blockSignals(true);
    positionYSpinBox->blockSignals(true);
    positionXSpinBox->changeValue( selPosition.x() );
    positionYSpinBox->changeValue( selPosition.y() );
    positionXSpinBox->blockSignals(false);
    positionYSpinBox->blockSignals(false);
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

    geometryTab->setEnabled( selectionCount );
    transformationTab->setEnabled ( selectionCount );
    advancedTab->setEnabled( selectionCount );

    widthSpinBox->blockSignals(true);
    heightSpinBox->blockSignals(true);
    widthSpinBox->changeValue( selSize.width() );
    heightSpinBox->changeValue( selSize.height() );
    widthSpinBox->blockSignals(false);
    heightSpinBox->blockSignals(false);
}

void DefaultToolWidget::sizeHasChanged()
{
    QSizeF newSize( widthSpinBox->value(), heightSpinBox->value() );

    KoSelection *selection = m_tool->canvas()->shapeManager()->selection();
    QRectF rect = selection->boundingRect();

    if( aspectButton->isChecked() )
    {
        double aspect = rect.width() / rect.height();
        if( rect.width() != newSize.width() )
            newSize.setHeight( newSize.width() / aspect );
        else if( rect.height() != newSize.height() )
            newSize.setWidth( newSize.height() * aspect );
    }

    if( rect.width() != newSize.width() || rect.height() != newSize.height() )
    {
        QMatrix resizeMatrix;
        resizeMatrix.translate( rect.x(), rect.y() );
        resizeMatrix.scale( newSize.width() / rect.width(), newSize.height() / rect.height() );
        resizeMatrix.translate( -rect.x(), -rect.y() );

        QList<KoShape*> selectedShapes = selection->selectedShapes( KoFlake::TopLevelSelection );
        QList<QSizeF> oldSizes, newSizes;
        QList<QMatrix> oldState;
        QList<QMatrix> newState;

        foreach( KoShape* shape, selectedShapes )
        {
            QSizeF oldSize = shape->size();
            oldState << shape->transformation();
            QMatrix shapeMatrix = shape->absoluteTransformation(0);

            // calculate the matrix we would apply to the local shape matrix
            // that tells us the effective scale values we have to use for the resizing
            QMatrix localMatrix = shapeMatrix * resizeMatrix * shapeMatrix.inverted();
            // save the effective scale values
            double scaleX = localMatrix.m11();
            double scaleY = localMatrix.m22();

            // calculate the scale matrix which is equivalent to our resizing above
            QMatrix scaleMatrix = (QMatrix().scale( scaleX, scaleY ));
            scaleMatrix =  shapeMatrix.inverted() * scaleMatrix * shapeMatrix;

            // calculate the new size of the shape, using the effective scale values
            oldSizes << oldSize;
            newSizes << QSizeF( scaleX * oldSize.width(), scaleY * oldSize.height() );
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
    // TODO find a way to get notified whenever the unit changes
    positionXSpinBox->setUnit( unit );
    positionYSpinBox->setUnit( unit );
    widthSpinBox->setUnit( unit );
    heightSpinBox->setUnit( unit );
    shearXSpinBox->setUnit( unit );
    shearYSpinBox->setUnit( unit );
}

void DefaultToolWidget::resourceChanged( int key, const QVariant & res )
{
    if( key == KoCanvasResource::Unit )
        setUnit( m_tool->canvas()->unit() );
    else if( key == KoCanvasResource::HotPosition )
    {
        if( res.toInt() != positionSelector->position() )
        {
            positionSelector->setPosition( static_cast<KoFlake::Position>( res.toInt() ) );
            updatePosition();
        }
    }
}

void DefaultToolWidget::rotationChanged()
{
    QList<KoShape*> selectedShapes = m_tool->canvas()->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
    QList<QMatrix> oldTransforms;

    foreach( KoShape* shape, selectedShapes )
        oldTransforms << shape->transformation();

    double angle = rotateSpinBox->value();
    QPointF rotationCenter = m_tool->canvas()->shapeManager()->selection()->absolutePosition( SelectionDecorator::hotPosition() );
    QMatrix matrix;
    matrix.translate(rotationCenter.x(), rotationCenter.y());
    matrix.rotate(angle);
    matrix.translate(-rotationCenter.x(), -rotationCenter.y());

    foreach( KoShape * shape, selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( matrix );
        shape->update();
    }

    m_tool->canvas()->shapeManager()->selection()->applyAbsoluteTransformation( matrix );
    QList<QMatrix> newTransforms;

    foreach( KoShape* shape, selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( selectedShapes, oldTransforms, newTransforms );
    cmd->setText( i18n("Rotate") );
    m_tool->canvas()->addCommand( cmd );
}

void DefaultToolWidget::shearXChanged()
{
    KoSelection* selection = m_tool->canvas()->shapeManager()->selection();
    QList<KoShape*> selectedShapes = selection->selectedShapes( KoFlake::TopLevelSelection );
    QList<QMatrix> oldTransforms;

    foreach( KoShape* shape, selectedShapes )
        oldTransforms << shape->transformation();

    double shearX = shearXSpinBox->value() / selection->size().height();
    QPointF basePoint = selection->absolutePosition( SelectionDecorator::hotPosition() );
    QMatrix matrix;
    matrix.translate(basePoint.x(), basePoint.y());
    matrix.shear(shearX, 0.0);
    matrix.translate(-basePoint.x(), -basePoint.y());

    foreach( KoShape * shape, selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( matrix );
        shape->update();
    }

    selection->applyAbsoluteTransformation( matrix );
    QList<QMatrix> newTransforms;

    foreach( KoShape* shape, selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( selectedShapes, oldTransforms, newTransforms );
    cmd->setText( i18n("Shear X") );
    m_tool->canvas()->addCommand( cmd );
}

void DefaultToolWidget::shearYChanged()
{
    KoSelection* selection = m_tool->canvas()->shapeManager()->selection();
    QList<KoShape*> selectedShapes = selection->selectedShapes( KoFlake::TopLevelSelection );
    QList<QMatrix> oldTransforms;

    foreach( KoShape* shape, selectedShapes )
        oldTransforms << shape->transformation();

    double shearY = shearYSpinBox->value() / selection->size().width();
    QPointF basePoint = selection->absolutePosition( SelectionDecorator::hotPosition() );
    QMatrix matrix;
    matrix.translate(basePoint.x(), basePoint.y());
    matrix.shear(0.0, shearY);
    matrix.translate(-basePoint.x(), -basePoint.y());

    foreach( KoShape * shape, selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( matrix );
        shape->update();
    }

    selection->applyAbsoluteTransformation( matrix );
    QList<QMatrix> newTransforms;

    foreach( KoShape* shape, selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( selectedShapes, oldTransforms, newTransforms );
    cmd->setText( i18n("Shear Y") );
    m_tool->canvas()->addCommand( cmd );
}

void DefaultToolWidget::scaleXChanged()
{
    QList<KoShape*> selectedShapes = m_tool->canvas()->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
    QList<QMatrix> oldTransforms;

    foreach( KoShape* shape, selectedShapes )
        oldTransforms << shape->transformation();

    double scale = scaleXSpinBox->value() * 0.01; // Input is in per cent
    QPointF basePoint = m_tool->canvas()->shapeManager()->selection()->absolutePosition( SelectionDecorator::hotPosition() );
    QMatrix matrix;
    matrix.translate(basePoint.x(), basePoint.y());

    if(scaleAspectCheckBox->isChecked())
        matrix.scale(scale, scale);
    else
        matrix.scale(scale, 1.0);

    matrix.translate(-basePoint.x(), -basePoint.y());

    foreach( KoShape * shape, selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( matrix );
        shape->update();
    }

    m_tool->canvas()->shapeManager()->selection()->applyAbsoluteTransformation( matrix );
    QList<QMatrix> newTransforms;

    foreach( KoShape* shape, selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( selectedShapes, oldTransforms, newTransforms );
    cmd->setText( i18n("Scale") );
    m_tool->canvas()->addCommand( cmd );
}

void DefaultToolWidget::scaleYChanged()
{
    QList<KoShape*> selectedShapes = m_tool->canvas()->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
    QList<QMatrix> oldTransforms;

    foreach( KoShape* shape, selectedShapes )
        oldTransforms << shape->transformation();

    double scale = scaleYSpinBox->value() * 0.01; // Input is in per cent
    QPointF basePoint = m_tool->canvas()->shapeManager()->selection()->absolutePosition( SelectionDecorator::hotPosition() );
    QMatrix matrix;
    matrix.translate(basePoint.x(), basePoint.y());
    matrix.scale(1.0, scale);
    matrix.translate(-basePoint.x(), -basePoint.y());

    foreach( KoShape * shape, selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( matrix );
        shape->update();
    }

    m_tool->canvas()->shapeManager()->selection()->applyAbsoluteTransformation( matrix );
    QList<QMatrix> newTransforms;

    foreach( KoShape* shape, selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( selectedShapes, oldTransforms, newTransforms );
    cmd->setText( i18n("Scale") );
    m_tool->canvas()->addCommand( cmd );
}

void DefaultToolWidget::resetTransformations()
{
    QList<KoShape*> selectedShapes = m_tool->canvas()->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
    QList<QMatrix> oldTransforms;

    foreach( KoShape* shape, selectedShapes )
        oldTransforms << shape->transformation();

    QMatrix matrix;

    foreach( KoShape * shape, selectedShapes ) {
        shape->update();
        shape->setTransformation( matrix );
        shape->update();
    }

    m_tool->canvas()->shapeManager()->selection()->applyAbsoluteTransformation( matrix );
    QList<QMatrix> newTransforms;

    foreach( KoShape* shape, selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( selectedShapes, oldTransforms, newTransforms );
    cmd->setText( i18n("Reset Transformations") );
    m_tool->canvas()->addCommand( cmd );
}

#include <DefaultToolWidget.moc>
