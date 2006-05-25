#include "KoCommand.h"
#include "KoShape.h"
#include "KoShapeGroup.h"
#include "KoShapeContainer.h"
#include "KoShapeControllerInterface.h"

#include <QString>

KoShapeMoveCommand::KoShapeMoveCommand(const KoSelectionSet &shapes, QList<QPointF> &previousPositions, QList<QPointF> &newPositions)
: m_previousPositions(previousPositions)
, m_newPositions(newPositions)
{
    m_shapes = shapes.toList();
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
    return "Move";
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
    return "Rotate";
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
    return "Group Shapes";
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
    return "Ungroup shapes";
}

KoShapeCreateCommand::KoShapeCreateCommand( KoShapeControllerInterface *controller, KoShape *shape )
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
    if( ! m_shape || ! m_controller )
        return;

    m_controller->addShape( m_shape );
    m_deleteShape = false;
}

void KoShapeCreateCommand::unexecute () {
    if( ! m_shape || ! m_controller )
        return;

    m_controller->removeShape( m_shape );
    m_deleteShape = true;
}

QString KoShapeCreateCommand::name () const {
    return "Create shape";
}
