/* This file is part of the KDE project
 * Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>
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

#include "DefaultToolTransformWidget.h"

#include <KoInteractionTool.h>
#include <KoCanvasBase.h>
#include <KoResourceManager.h>
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
#include <QTransform>

DefaultToolTransformWidget::DefaultToolTransformWidget( KoInteractionTool* tool,
                                    QWidget* parent )
    : QMenu(parent)
{
    m_tool = tool;

    setupUi( this );

    setUnit( m_tool->canvas()->unit() );

    connect( m_tool->canvas()->resourceManager(), SIGNAL( resourceChanged( int, const QVariant& ) ),
        this, SLOT( resourceChanged( int, const QVariant& ) ) );

    connect( rotateButton, SIGNAL( clicked() ), this, SLOT( rotationChanged() ) );
    connect( shearXButton, SIGNAL( clicked() ), this, SLOT( shearXChanged() ) );
    connect( shearYButton, SIGNAL( clicked() ), this, SLOT( shearYChanged() ) );
    connect( scaleXButton, SIGNAL( clicked() ), this, SLOT( scaleXChanged() ) );
    connect( scaleYButton, SIGNAL( clicked() ), this, SLOT( scaleYChanged() ) );
    connect( scaleAspectCheckBox, SIGNAL( toggled( bool ) ), scaleYSpinBox, SLOT( setDisabled( bool ) ) );
    connect( scaleAspectCheckBox, SIGNAL( toggled( bool ) ), scaleYButton, SLOT( setDisabled( bool ) ) );
    connect( resetButton, SIGNAL( clicked() ), this, SLOT( resetTransformations() ) );
}


void DefaultToolTransformWidget::setUnit( const KoUnit &unit )
{
    shearXSpinBox->setUnit( unit );
    shearYSpinBox->setUnit( unit );
}

void DefaultToolTransformWidget::resourceChanged( int key, const QVariant & res )
{
    if (key == KoCanvasResource::Unit)
        setUnit(res.value<KoUnit>());
}

void DefaultToolTransformWidget::rotationChanged()
{
    QList<KoShape*> selectedShapes = m_tool->canvas()->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
    QList<QTransform> oldTransforms;

    foreach( KoShape* shape, selectedShapes )
        oldTransforms << shape->transformation();

    qreal angle = rotateSpinBox->value();
    QPointF rotationCenter = m_tool->canvas()->shapeManager()->selection()->absolutePosition( SelectionDecorator::hotPosition() );
    QTransform matrix;
    matrix.translate(rotationCenter.x(), rotationCenter.y());
    matrix.rotate(angle);
    matrix.translate(-rotationCenter.x(), -rotationCenter.y());

    foreach( KoShape * shape, selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( matrix );
        shape->update();
    }

    m_tool->canvas()->shapeManager()->selection()->applyAbsoluteTransformation( matrix );
    QList<QTransform> newTransforms;

    foreach( KoShape* shape, selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( selectedShapes, oldTransforms, newTransforms );
    cmd->setText( i18n("Rotate") );
    m_tool->canvas()->addCommand( cmd );
}

void DefaultToolTransformWidget::shearXChanged()
{
    KoSelection* selection = m_tool->canvas()->shapeManager()->selection();
    QList<KoShape*> selectedShapes = selection->selectedShapes( KoFlake::TopLevelSelection );
    QList<QTransform> oldTransforms;

    foreach( KoShape* shape, selectedShapes )
        oldTransforms << shape->transformation();

    qreal shearX = shearXSpinBox->value() / selection->size().height();
    QPointF basePoint = selection->absolutePosition( SelectionDecorator::hotPosition() );
    QTransform matrix;
    matrix.translate(basePoint.x(), basePoint.y());
    matrix.shear(shearX, 0.0);
    matrix.translate(-basePoint.x(), -basePoint.y());

    foreach( KoShape * shape, selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( matrix );
        shape->update();
    }

    selection->applyAbsoluteTransformation( matrix );
    QList<QTransform> newTransforms;

    foreach( KoShape* shape, selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( selectedShapes, oldTransforms, newTransforms );
    cmd->setText( i18n("Shear X") );
    m_tool->canvas()->addCommand( cmd );
}

void DefaultToolTransformWidget::shearYChanged()
{
    KoSelection* selection = m_tool->canvas()->shapeManager()->selection();
    QList<KoShape*> selectedShapes = selection->selectedShapes( KoFlake::TopLevelSelection );
    QList<QTransform> oldTransforms;

    foreach( KoShape* shape, selectedShapes )
        oldTransforms << shape->transformation();

    qreal shearY = shearYSpinBox->value() / selection->size().width();
    QPointF basePoint = selection->absolutePosition( SelectionDecorator::hotPosition() );
    QTransform matrix;
    matrix.translate(basePoint.x(), basePoint.y());
    matrix.shear(0.0, shearY);
    matrix.translate(-basePoint.x(), -basePoint.y());

    foreach( KoShape * shape, selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( matrix );
        shape->update();
    }

    selection->applyAbsoluteTransformation( matrix );
    QList<QTransform> newTransforms;

    foreach( KoShape* shape, selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( selectedShapes, oldTransforms, newTransforms );
    cmd->setText( i18n("Shear Y") );
    m_tool->canvas()->addCommand( cmd );
}

void DefaultToolTransformWidget::scaleXChanged()
{
    QList<KoShape*> selectedShapes = m_tool->canvas()->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
    QList<QTransform> oldTransforms;

    foreach( KoShape* shape, selectedShapes )
        oldTransforms << shape->transformation();

    qreal scale = scaleXSpinBox->value() * 0.01; // Input is in per cent
    QPointF basePoint = m_tool->canvas()->shapeManager()->selection()->absolutePosition( SelectionDecorator::hotPosition() );
    QTransform matrix;
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
    QList<QTransform> newTransforms;

    foreach( KoShape* shape, selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( selectedShapes, oldTransforms, newTransforms );
    cmd->setText( i18n("Scale") );
    m_tool->canvas()->addCommand( cmd );
}

void DefaultToolTransformWidget::scaleYChanged()
{
    QList<KoShape*> selectedShapes = m_tool->canvas()->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
    QList<QTransform> oldTransforms;

    foreach( KoShape* shape, selectedShapes )
        oldTransforms << shape->transformation();

    qreal scale = scaleYSpinBox->value() * 0.01; // Input is in per cent
    QPointF basePoint = m_tool->canvas()->shapeManager()->selection()->absolutePosition( SelectionDecorator::hotPosition() );
    QTransform matrix;
    matrix.translate(basePoint.x(), basePoint.y());
    matrix.scale(1.0, scale);
    matrix.translate(-basePoint.x(), -basePoint.y());

    foreach( KoShape * shape, selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( matrix );
        shape->update();
    }

    m_tool->canvas()->shapeManager()->selection()->applyAbsoluteTransformation( matrix );
    QList<QTransform> newTransforms;

    foreach( KoShape* shape, selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( selectedShapes, oldTransforms, newTransforms );
    cmd->setText( i18n("Scale") );
    m_tool->canvas()->addCommand( cmd );
}

void DefaultToolTransformWidget::resetTransformations()
{
    QList<KoShape*> selectedShapes = m_tool->canvas()->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
    QList<QTransform> oldTransforms;

    foreach( KoShape* shape, selectedShapes )
        oldTransforms << shape->transformation();

    QTransform matrix;

    foreach( KoShape * shape, selectedShapes ) {
        shape->update();
        shape->setTransformation( matrix );
        shape->update();
    }

    m_tool->canvas()->shapeManager()->selection()->applyAbsoluteTransformation( matrix );
    QList<QTransform> newTransforms;

    foreach( KoShape* shape, selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( selectedShapes, oldTransforms, newTransforms );
    cmd->setText( i18n("Reset Transformations") );
    m_tool->canvas()->addCommand( cmd );
}

#include <DefaultToolTransformWidget.moc>
