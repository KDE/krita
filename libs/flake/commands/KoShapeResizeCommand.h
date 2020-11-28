/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOSHAPERESIZECOMMAND_H
#define KOSHAPERESIZECOMMAND_H

#include "kritaflake_export.h"
#include "kundo2command.h"
#include "kis_command_utils.h"

#include <QList>
#include <QPointF>
#include <KoFlake.h>

#include <QScopedPointer>

class KoShape;


class KRITAFLAKE_EXPORT KoShapeResizeCommand : public KisCommandUtils::SkipFirstRedoBase
{
public:
    KoShapeResizeCommand(const QList<KoShape*> &shapes,
                         qreal scaleX, qreal scaleY,
                         const QPointF &absoluteStillPoint, bool useGLobalMode,
                         bool usePostScaling, const QTransform &postScalingCoveringTransform,
                         KUndo2Command *parent = 0);

    ~KoShapeResizeCommand() override;
    void redoImpl() override;
    void undoImpl() override;

    QMap<KoShape*, QRectF> redoNoUpdate();
    QMap<KoShape*, QRectF> undoNoUpdate();

    int id() const override;
    bool mergeWith(const KUndo2Command *command) override;

    void replaceResizeAction(qreal scaleX, qreal scaleY,
                             const QPointF &absoluteStillPoint);

private:
    struct Private;
    QScopedPointer<Private> const m_d;

};

#endif // KOSHAPERESIZECOMMAND_H
