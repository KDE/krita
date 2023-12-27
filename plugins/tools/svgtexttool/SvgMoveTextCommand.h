/*
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SVG_MOVE_TEXT_COMMAND_H
#define SVG_MOVE_TEXT_COMMAND_H

#include <kundo2command.h>

class KoSvgTextShape;

class SvgMoveTextCommand : public KUndo2Command
{
public:
    SvgMoveTextCommand(KoSvgTextShape *shape,
                       const QPointF &newPosition,
                       const QPointF &oldPosition,
                       KUndo2Command *parent = nullptr);
    ~SvgMoveTextCommand() override = default;

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *other) override;

private:
    void applyPosition(const QPointF &position);

private:
    KoSvgTextShape *m_shape;
    QPointF m_newPosition;
    QPointF m_oldPosition;
};

#endif /* SVG_MOVE_TEXT_COMMAND_H */
