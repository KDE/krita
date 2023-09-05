/*
 * SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SVGTEXTREMOVECOMMAND_H
#define SVGTEXTREMOVECOMMAND_H

#include <kundo2command.h>

class KoSvgTextShape;
class SvgTextCursor;

class SvgTextRemoveCommand : public KUndo2Command
{
public:
    SvgTextRemoveCommand(KoSvgTextShape *shape,
                         int pos,
                         int originalPos,
                         int anchor,
                         int length,
                         SvgTextCursor *cursor = 0,
                         KUndo2Command *parent = 0);
    ~SvgTextRemoveCommand() override = default;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *other) override;

private:
    KoSvgTextShape *m_shape;
    int m_pos;
    int m_originalPos;
    int m_anchor;
    int m_length;
    SvgTextCursor *m_cursor;
    QString m_oldSvg;
    QString m_oldDefs;
};

#endif // SVGTEXTREMOVECOMMAND_H
