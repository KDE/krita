/*
 * SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SVGTEXTREMOVECOMMAND_H
#define SVGTEXTREMOVECOMMAND_H

#include <kundo2command.h>
#include "kritatoolsvgtext_export.h"
#include <KoSvgTextShape.h>

class KoSvgTextShape;

class KRITATOOLSVGTEXT_EXPORT SvgTextRemoveCommand : public KUndo2Command
{
public:
    /**
     * @brief SvgTextRemoveCommand
     * Remove text from shape
     * @param shape -- shape to remove text from
     * @param endIndex -- end from which to remove.
     * @param pos -- pos, only used to reset cursor pos upon undo.
     * @param anchor -- anchor, only used to reset cursor pos upon undo.
     * @param length -- length to remove from end index
     * @param allowCleanUp -- whether to allow cleaning up the text shape data. Should be set false when inserting text directly after.
     * @param parent -- parent undo command.
     */
    SvgTextRemoveCommand(KoSvgTextShape *shape,
                         int endIndex,
                         int pos,
                         int anchor,
                         int length,
                         bool allowCleanUp = true,
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
    bool m_allowCleanUp;
    KoSvgTextShapeMementoSP m_textData;
};

#endif // SVGTEXTREMOVECOMMAND_H
