/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include <QMap>

#include "KoCommand.h"
#include "KoShape.h"
#include "KoShapeGroup.h"
#include "KoShapeContainer.h"
#include "KoShapeControllerBase.h"

#include <klocale.h>

KoShapeMoveCommand::KoShapeMoveCommand(const KoSelectionSet &shapes, QList<QPointF> &previousPositions, QList<QPointF> &newPositions)
: m_previousPositions(previousPositions)
, m_newPositions(newPositions)
{
    m_shapes = shapes.toList();
    Q_ASSERT(m_shapes.count() == m_previousPositions.count());
    Q_ASSERT(m_shapes.count() == m_newPositions.count());
}

KoShapeMoveCommand::KoShapeMoveCommand(const QList<KoShape*> &shapes, QList<QPointF> &previousPositions, QList<QPointF> &newPositions)
: m_shapes(shapes)
, m_previousPositions(previousPositions)
, m_newPositions(newPositions)
{
    Q_ASSERT(m_shapes.count() == m_previousPositions.count());
    Q_ASSERT(m_shapes.count() == m_newPositions.count());
}

void KoShapeMoveCommand::execute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->setPosition( m_newPositions.at(i) );
        m_shapes.at(i)->repaint();
    }
}

void KoShapeMoveCommand::unexecute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->setPosition( m_previousPositions.at(i) );
        m_shapes.at(i)->repaint();
    }
}

QString KoShapeMoveCommand::name () const {
    return i18n( "Move shapes" );
}


KoShapeRotateCommand::KoShapeRotateCommand(const KoSelectionSet &shapes, QList<double> &previousAngles, QList<double> &newAngles)
: m_previousAngles(previousAngles)
, m_newAngles(newAngles)
{
    m_shapes = shapes.toList();
    Q_ASSERT(m_shapes.count() == m_previousAngles.count());
    Q_ASSERT(m_shapes.count() == m_newAngles.count());
}

void KoShapeRotateCommand::execute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->rotate( m_newAngles.at(i) );
        m_shapes.at(i)->repaint();
    }
}

void KoShapeRotateCommand::unexecute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->rotate( m_previousAngles.at(i) );
        m_shapes.at(i)->repaint();
    }
}

QString KoShapeRotateCommand::name () const {
    return i18n( "Rotate shapes" );
}


KoShapeShearCommand::KoShapeShearCommand(const KoSelectionSet &shapes, QList<double> &previousShearXs, QList<double> &previousShearYs, QList<double> &newShearXs, QList<double> &newShearYs)
: m_previousShearXs(previousShearXs)
, m_previousShearYs(previousShearYs)
, m_newShearXs(newShearXs)
, m_newShearYs(newShearYs)
{
    m_shapes = shapes.toList();
    Q_ASSERT(m_shapes.count() == m_previousShearXs.count());
    Q_ASSERT(m_shapes.count() == m_previousShearYs.count());
    Q_ASSERT(m_shapes.count() == m_newShearXs.count());
    Q_ASSERT(m_shapes.count() == m_newShearYs.count());
}

void KoShapeShearCommand::execute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->shear( m_newShearXs.at(i), m_newShearYs.at(i));
        m_shapes.at(i)->repaint();
    }
}

void KoShapeShearCommand::unexecute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->shear( m_previousShearXs.at(i), m_previousShearYs.at(i) );
        m_shapes.at(i)->repaint();
    }
}

QString KoShapeShearCommand::name () const {
    return i18n( "Shear shapes" );
}


KoShapeSizeCommand::KoShapeSizeCommand(const KoSelectionSet &shapes, QList<QSizeF> &previousSizes, QList<QSizeF> &newSizes)
: m_previousSizes(previousSizes)
, m_newSizes(newSizes)
{
    m_shapes = shapes.toList();
    Q_ASSERT(m_shapes.count() == m_previousSizes.count());
    Q_ASSERT(m_shapes.count() == m_newSizes.count());
}

void KoShapeSizeCommand::execute () {
    int i=0;
    foreach(KoShape *shape, m_shapes) {
        shape->repaint();
        shape->resize(m_newSizes[i++]);
        shape->repaint();
    }
}

void KoShapeSizeCommand::unexecute () {
    int i=0;
    foreach(KoShape *shape, m_shapes) {
        shape->repaint();
        shape->resize(m_previousSizes[i++]);
        shape->repaint();
    }
}

QString KoShapeSizeCommand::name () const {
    return i18n( "Resize shapes" );
}


KoGroupShapesCommand::KoGroupShapesCommand(KoShapeContainer *container, QList<KoShape *> shapes, QList<bool> clipped)
: m_shapes(shapes)
, m_clipped(clipped)
, m_container(container)
{
    Q_ASSERT(m_clipped.count() == m_shapes.count());
}

KoGroupShapesCommand::KoGroupShapesCommand(KoShapeGroup *container, QList<KoShape *> shapes)
: m_shapes(shapes)
, m_container(container)
{
    for(int i=m_shapes.count(); i > 0; i--)
        m_clipped.append(false);
}

KoGroupShapesCommand::KoGroupShapesCommand() {
}

void KoGroupShapesCommand::execute () {
    foreach(KoShape *shape, m_shapes) {
        m_container->addChild(shape);
        shape->setPosition(shape->position() - m_container->position());
    }
}

void KoGroupShapesCommand::unexecute () {
    foreach(KoShape *shape, m_shapes) {
        m_container->removeChild(shape);
        shape->setPosition(shape->position() + m_container->position());
    }
}

QString KoGroupShapesCommand::name () const {
    return i18n( "Group shapes" );
}


KoUngroupShapesCommand::KoUngroupShapesCommand(KoShapeContainer *container, QList<KoShape *> shapes)
: KoGroupShapesCommand()
{
    m_shapes = shapes;
    m_container = container;
    foreach(KoShape *shape, m_shapes) {
        m_clipped.append( m_container->childClipped(shape) );
    }
}

void KoUngroupShapesCommand::execute () {
    KoGroupShapesCommand::unexecute();
}

void KoUngroupShapesCommand::unexecute () {
    KoGroupShapesCommand::execute();
}

QString KoUngroupShapesCommand::name () const {
    return i18n( "Ungroup shapes" );
}

KoShapeCreateCommand::KoShapeCreateCommand( KoShapeControllerBase *controller, KoShape *shape )
: m_controller( controller )
, m_shape( shape )
, m_deleteShape( true )
{
}

KoShapeCreateCommand::~KoShapeCreateCommand() {
    if( m_shape && m_deleteShape )
        delete m_shape;
}

void KoShapeCreateCommand::execute () {
    Q_ASSERT(m_shape);
    Q_ASSERT(m_controller);
    m_controller->addShape( m_shape );
    m_deleteShape = false;
}

void KoShapeCreateCommand::unexecute () {
    Q_ASSERT(m_shape);
    Q_ASSERT(m_controller);
    m_controller->removeShape( m_shape );
    m_deleteShape = true;
}

QString KoShapeCreateCommand::name () const {
    return i18n( "Create shape" );
}

KoShapeDeleteCommand::KoShapeDeleteCommand( KoShapeControllerBase *controller, KoShape *shape )
: m_controller( controller )
, m_deleteShapes( false )
{
    m_shapes.append( shape );
}

KoShapeDeleteCommand::KoShapeDeleteCommand( KoShapeControllerBase *controller, const KoSelectionSet &shapes )
: m_controller( controller )
, m_deleteShapes( false )
{
    m_shapes = shapes.toList();
}

KoShapeDeleteCommand::~KoShapeDeleteCommand() {
    if( ! m_deleteShapes )
        return;

    foreach (KoShape *shape, m_shapes ) {
        delete shape;
    }
}

void KoShapeDeleteCommand::execute () {
    if( ! m_controller )
        return;

    foreach (KoShape *shape, m_shapes ) {
        m_controller->removeShape( shape );
    }
    m_deleteShapes = true;
}

void KoShapeDeleteCommand::unexecute () {
    if( ! m_controller )
        return;

    foreach (KoShape *shape, m_shapes ) {
        m_controller->addShape( shape );
    }
    m_deleteShapes = false;
}

QString KoShapeDeleteCommand::name () const {
    return i18n( "Delete shapes" );
}

KoShapeBackgroundCommand::KoShapeBackgroundCommand( const KoSelectionSet &shapes, const QBrush &brush )
: m_newBrush( brush )
{
    m_shapes = shapes.toList();
}

KoShapeBackgroundCommand::~KoShapeBackgroundCommand() {
}

void KoShapeBackgroundCommand::execute () {
    foreach( KoShape *shape, m_shapes ) {
        m_oldBrushes.append( shape->background() );
        shape->setBackground( m_newBrush );
        shape->repaint();
    }
}

void KoShapeBackgroundCommand::unexecute () {
    QList<QBrush>::iterator brushIt = m_oldBrushes.begin();
    foreach( KoShape *shape, m_shapes ) {
        shape->setBackground( *brushIt );
        shape->repaint();
        brushIt++;
    }
}

QString KoShapeBackgroundCommand::name () const {
    return i18n( "Set background" );
}

KoShapeAlignCommand::KoShapeAlignCommand( const KoSelectionSet &shapes, Align align, QRectF boundingRect )
{
    QList<QPointF> previousPositions;
    QList<QPointF> newPositions;
    QPointF position;
    QPointF delta;
    QRectF bRect;
    foreach( KoShape *shape, shapes ) {
        position = shape->position();
        previousPositions  << position;
        bRect = shape->boundingRect();
        switch( align )
        {
            case ALIGN_HORIZONTAL_LEFT:
                delta = QPointF( boundingRect.left(), bRect.y()) - bRect.topLeft();
                break;
            case ALIGN_HORIZONTAL_CENTER:
                delta = QPointF( boundingRect.center().x() - bRect.width()/2, bRect.y()) - bRect.topLeft();
                break;
            case ALIGN_HORIZONTAL_RIGHT:
                delta = QPointF( boundingRect.right() - bRect.width(), bRect.y()) - bRect.topLeft();
                break;
            case ALIGN_VERTICAL_TOP:
                delta = QPointF( bRect.x(), boundingRect.top()) - bRect.topLeft();
                break;
            case ALIGN_VERTICAL_CENTER:
                delta = QPointF(  bRect.x(), boundingRect.center().y() - bRect.height()/2) - bRect.topLeft();
                break;
            case ALIGN_VERTICAL_BOTTOM:
                delta = QPointF(  bRect.x(), boundingRect.bottom() - bRect.height()) - bRect.topLeft();
                break;
        };
        newPositions  << position + delta;
    }
    m_command = new KoShapeMoveCommand(shapes, previousPositions, newPositions);
}

KoShapeAlignCommand::~KoShapeAlignCommand()
{
    delete m_command;
}

void KoShapeAlignCommand::execute()
{
    m_command->execute();
}

void KoShapeAlignCommand::unexecute()
{
    m_command->unexecute();
}

QString KoShapeAlignCommand::name () const {
    return i18n( "Align shapes" );
}

KoShapeDistributeCommand::KoShapeDistributeCommand( const KoSelectionSet &shapes, Distribute distribute,  QRectF boundingRect )
: m_distribute( distribute )
{
    QMap<double,KoShape*> sortedPos;
    QRectF bRect;
    double extent = 0.0;
    // sort by position and calculate sum of objects widht/height
    foreach( KoShape *shape, shapes ) {
        bRect = shape->boundingRect();
        switch( m_distribute ) {
            case DISTRIBUTE_HORIZONTAL_CENTER:
                sortedPos[bRect.center().x()] = shape;
                break;
            case DISTRIBUTE_HORIZONTAL_GAP:
            case DISTRIBUTE_HORIZONTAL_LEFT:
                sortedPos[bRect.left()] = shape;
                extent += bRect.width();
                break;
            case DISTRIBUTE_HORIZONTAL_RIGHT:
                sortedPos[bRect.right()] = shape;
                break;
            case DISTRIBUTE_VERTICAL_CENTER:
                sortedPos[bRect.center().y()] = shape;
                break;
             case DISTRIBUTE_VERTICAL_GAP:
             case DISTRIBUTE_VERTICAL_BOTTOM:
                sortedPos[bRect.bottom()] = shape;
                extent += bRect.height();
                break;
             case DISTRIBUTE_VERTICAL_TOP:
                sortedPos[bRect.top()] = shape;
                break;
        }
    }
    KoShape* first = sortedPos.begin().value();
    KoShape* last = (--sortedPos.end()).value();

    // determine the available space to distribute
    double space = getAvailableSpace( first, last, extent, boundingRect);
    double pos = 0.0, step = space / double(shapes.count() - 1);

    QList<QPointF> previousPositions;
    QList<QPointF> newPositions;
    QPointF position;
    QPointF delta;
    QMapIterator<double,KoShape*> it(sortedPos);
    while(it.hasNext())
    {
        it.next();
        position = it.value()->position();
        previousPositions  << position;

        bRect = it.value()->boundingRect();
        switch( m_distribute )        {
            case DISTRIBUTE_HORIZONTAL_CENTER:
                delta = QPointF( boundingRect.x() + first->boundingRect().width()/2 + pos - bRect.width()/2, bRect.y() ) - bRect.topLeft();
                break;
            case DISTRIBUTE_HORIZONTAL_GAP:
                delta = QPointF( boundingRect.left() + pos, bRect.y() ) - bRect.topLeft();
                pos += bRect.width();
                break;
            case DISTRIBUTE_HORIZONTAL_LEFT:
                delta = QPointF( boundingRect.left() + pos, bRect.y() ) - bRect.topLeft();
                break;
            case DISTRIBUTE_HORIZONTAL_RIGHT:
                delta = QPointF( boundingRect.left() + first->boundingRect().width() + pos - bRect.width(), bRect.y() ) - bRect.topLeft();
                break;
            case DISTRIBUTE_VERTICAL_CENTER:
                delta = QPointF( bRect.x(), boundingRect.y() + first->boundingRect().height()/2 + pos - bRect.height()/2 ) - bRect.topLeft();
                break;
            case DISTRIBUTE_VERTICAL_GAP:
                delta = QPointF( bRect.x(), boundingRect.top() + pos ) - bRect.topLeft();
                pos += bRect.height();
                break;
            case DISTRIBUTE_VERTICAL_BOTTOM:
                delta = QPointF( bRect.x(), boundingRect.top() + first->boundingRect().height() + pos - bRect.height() ) - bRect.topLeft();
                break;
            case DISTRIBUTE_VERTICAL_TOP:
                delta = QPointF( bRect.x(), boundingRect.top() + pos ) - bRect.topLeft();
                break;
        };
        newPositions  << position + delta;
        pos += step;
    }
    m_command = new KoShapeMoveCommand(sortedPos.values(), previousPositions, newPositions);
}

KoShapeDistributeCommand::~KoShapeDistributeCommand()
{
    delete m_command;
}

void KoShapeDistributeCommand::execute()
{
    m_command->execute();
}

void KoShapeDistributeCommand::unexecute()
{
    m_command->unexecute();
}

QString KoShapeDistributeCommand::name () const {
    return i18n( "Distribute shapes" );
}

double KoShapeDistributeCommand::getAvailableSpace( KoShape *first, KoShape *last, double extent, QRectF boundingRect  )
{
    switch( m_distribute ) {
        case DISTRIBUTE_HORIZONTAL_CENTER:
            return boundingRect.width() - last->boundingRect().width()/2 - first->boundingRect().width()/2;
            break;
        case DISTRIBUTE_HORIZONTAL_GAP:
            return boundingRect.width() - extent;
            break;
        case DISTRIBUTE_HORIZONTAL_LEFT:
            return boundingRect.width() - last->boundingRect().width();
            break;
        case DISTRIBUTE_HORIZONTAL_RIGHT:
            return boundingRect.width() - first->boundingRect().width();
            break;
        case DISTRIBUTE_VERTICAL_CENTER:
            return boundingRect.height() - last->boundingRect().height()/2 - first->boundingRect().height()/2;
            break;
        case DISTRIBUTE_VERTICAL_GAP:
            return boundingRect.height() - extent;
            break;
        case DISTRIBUTE_VERTICAL_BOTTOM:
            return boundingRect.height() - first->boundingRect().height();
            break;
        case DISTRIBUTE_VERTICAL_TOP:
            return boundingRect.height() - last->boundingRect().height();
            break;
    }
    return 0.0;
}
