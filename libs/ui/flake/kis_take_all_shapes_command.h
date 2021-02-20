/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TAKE_ALL_SHAPES_COMMAND_H
#define __KIS_TAKE_ALL_SHAPES_COMMAND_H

#include <QList>

#include "kundo2command.h"

#include "kritaui_export.h"
#include "kis_types.h"

class KoShape;
class KisShapeSelection;


class KisTakeAllShapesCommand : public KUndo2Command
{
public:
    KisTakeAllShapesCommand(KisShapeSelection *shapeSelection,
                            bool takeSilently,
                            bool restoreSilently);
    ~KisTakeAllShapesCommand() override;

    void redo() override;
    void undo() override;

private:
    KisShapeSelection *m_shapeSelection;
    QList<KoShape*> m_shapes;
    bool m_takeSilently;
    bool m_restoreSilently;
};

#endif /* __KIS_TAKE_ALL_SHAPES_COMMAND_H */
