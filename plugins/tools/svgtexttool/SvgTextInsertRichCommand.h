/*
 * SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGTEXTINSERTRICHTCOMMAND_H
#define SVGTEXTINSERTRICHTCOMMAND_H

#include <kundo2command.h>
#include "kritatoolsvgtext_export.h"
#include <KoSvgTextShape.h>

class KRITATOOLSVGTEXT_EXPORT SvgTextInsertRichCommand : public KUndo2Command
{
public:
    SvgTextInsertRichCommand(KoSvgTextShape *shape, KoSvgTextShape *insert, int pos, int anchor, bool inheritPropertiesIfPossible = false, KUndo2Command *parent = 0);
    ~SvgTextInsertRichCommand() override = default;

    void redo() override;

    void undo() override;

    //int id() const override;
    //bool mergeWith(const KUndo2Command *other) override;

private:
    KoSvgTextShape *m_shape;
    KoSvgTextShape *m_insert;

    int m_pos;
    int m_anchor;
    bool m_inheritPropertiesIfPossible;
    KoSvgTextShapeMementoSP m_textData;
};

#endif // SVGTEXTINSERTRICHTCOMMAND_H
