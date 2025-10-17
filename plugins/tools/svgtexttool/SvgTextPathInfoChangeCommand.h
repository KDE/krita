/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGTEXTPATHINFOCHANGECOMMAND_H
#define SVGTEXTPATHINFOCHANGECOMMAND_H

#include <kundo2command.h>
#include "kritatoolsvgtext_export.h"

#include <KoSvgTextShape.h>
#include <KoSvgText.h>

class KRITATOOLSVGTEXT_EXPORT SvgTextPathInfoChangeCommand : public KUndo2Command
{
public:
    SvgTextPathInfoChangeCommand(KoSvgTextShape *shape, int pos, KoSvgText::TextOnPathInfo textPathInfo, KUndo2Command *parent = nullptr);

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *other) override;

private:
    KoSvgTextShape *m_shape;
    KoSvgTextShapeMementoSP m_textData;
    int m_pos;
    KoSvgText::TextOnPathInfo m_newInfo;
};

#endif // SVGTEXTPATHINFOCHANGECOMMAND_H
