/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoShapeResizeCommand.h"

#include <KoShape.h>
#include "kis_command_ids.h"
#include <KoShapeBulkActionLock.h>


struct Q_DECL_HIDDEN KoShapeResizeCommand::Private
{
    QList<KoShape *> shapes;
    qreal scaleX;
    qreal scaleY;
    QPointF absoluteStillPoint;
    bool useGlobalMode;
    bool usePostScaling;
    QTransform postScalingCoveringTransform;

    QList<QSizeF> oldSizes;
    QList<QTransform> oldTransforms;
};


KoShapeResizeCommand::KoShapeResizeCommand(const QList<KoShape*> &shapes,
                                           qreal scaleX, qreal scaleY,
                                           const QPointF &absoluteStillPoint,
                                           bool useGLobalMode,
                                           bool usePostScaling,
                                           const QTransform &postScalingCoveringTransform,
                                           KUndo2Command *parent)
    : SkipFirstRedoBase(false, kundo2_i18n("Resize"), parent),
      m_d(new Private)
{
    m_d->shapes = shapes;
    m_d->scaleX = scaleX;
    m_d->scaleY = scaleY;
    m_d->absoluteStillPoint = absoluteStillPoint;
    m_d->useGlobalMode = useGLobalMode;
    m_d->usePostScaling = usePostScaling;
    m_d->postScalingCoveringTransform = postScalingCoveringTransform;

    Q_FOREACH (KoShape *shape, m_d->shapes) {
        m_d->oldSizes << shape->size();
        m_d->oldTransforms << shape->transformation();
    }
}

KoShapeResizeCommand::~KoShapeResizeCommand()
{
}

void KoShapeResizeCommand::redoImpl()
{
    KoShapeBulkActionLock lock(m_d->shapes);

    redoNoUpdate();

    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}

void KoShapeResizeCommand::undoImpl()
{
    KoShapeBulkActionLock lock(m_d->shapes);

    undoNoUpdate();

    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}

void KoShapeResizeCommand::redoNoUpdate()
{
    Q_FOREACH (KoShape *shape, m_d->shapes) {
        KoFlake::resizeShapeCommon(shape,
                             m_d->scaleX, m_d->scaleY,
                             m_d->absoluteStillPoint,
                             m_d->useGlobalMode,
                             m_d->usePostScaling,
                             m_d->postScalingCoveringTransform);
    }
}

void KoShapeResizeCommand::undoNoUpdate()
{
    for (int i = 0; i < m_d->shapes.size(); i++) {
        KoShape *shape = m_d->shapes[i];

        shape->setSize(m_d->oldSizes[i]);
        shape->setTransformation(m_d->oldTransforms[i]);
    }
}

int KoShapeResizeCommand::id() const
{
    return KisCommandUtils::ResizeShapeId;
}

bool KoShapeResizeCommand::mergeWith(const KUndo2Command *command)
{
    const KoShapeResizeCommand *other = dynamic_cast<const KoShapeResizeCommand*>(command);

    if (!other ||
        other->m_d->absoluteStillPoint != m_d->absoluteStillPoint ||
        other->m_d->shapes != m_d->shapes ||
        other->m_d->useGlobalMode != m_d->useGlobalMode ||
        other->m_d->usePostScaling != m_d->usePostScaling) {

        return false;
    }

    // check if the significant orientations coincide
    if (m_d->useGlobalMode && !m_d->usePostScaling) {
        Qt::Orientation our = KoFlake::significantScaleOrientation(m_d->scaleX, m_d->scaleY);
        Qt::Orientation their = KoFlake::significantScaleOrientation(other->m_d->scaleX, other->m_d->scaleY);

        if (our != their) {
            return false;
        }
    }

    m_d->scaleX *= other->m_d->scaleX;
    m_d->scaleY *= other->m_d->scaleY;
    return true;
}

void KoShapeResizeCommand::replaceResizeAction(qreal scaleX, qreal scaleY, const QPointF &absoluteStillPoint)
{
    KoShapeBulkActionLock lock(m_d->shapes);

    undoNoUpdate();

    m_d->scaleX = scaleX;
    m_d->scaleY = scaleY;
    m_d->absoluteStillPoint = absoluteStillPoint;

    redoNoUpdate();

    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}
