/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

    int id() const override;
    bool mergeWith(const KUndo2Command *command) override;

private:
    struct Private;
    QScopedPointer<Private> const m_d;

};

#endif // KOSHAPERESIZECOMMAND_H
