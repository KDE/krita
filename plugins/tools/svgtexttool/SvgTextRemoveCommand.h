/*
 * SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SVGTEXTREMOVECOMMAND_H
#define SVGTEXTREMOVECOMMAND_H

#include <kundo2command.h>
#include "kritatoolsvgtext_export.h"

class KoSvgTextShape;

class KRITATOOLSVGTEXT_EXPORT SvgTextRemoveCommand : public KUndo2Command
{
public:
    SvgTextRemoveCommand(KoSvgTextShape *shape,
                         int endIndex,
                         int pos,
                         int anchor,
                         int length,
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
    int m_index;
    int m_originalPos;
    int m_anchor;
    int m_length;
    QString m_oldSvg;
    QString m_oldDefs;
};

#endif // SVGTEXTREMOVECOMMAND_H
