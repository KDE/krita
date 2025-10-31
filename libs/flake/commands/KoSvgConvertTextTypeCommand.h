/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGCONVERTTEXTTYPECOMMAND_H
#define SVGCONVERTTEXTTYPECOMMAND_H

#include <kundo2command.h>
#include "kritaflake_export.h"
#include <KoSvgTextShape.h>

/**
 * @brief The SvgConvertTextTypeCommand class
 * This command allows textshapes to be converted between
 * preformatted and inline size types.
 *
 * Internally, this applies white space, inserts new lines
 * for each new text chunk, and finally, assigns an inline-size if
 * necessary.
 */
class KRITAFLAKE_EXPORT KoSvgConvertTextTypeCommand : public KUndo2Command
{
public:

    KoSvgConvertTextTypeCommand(KoSvgTextShape *shape, KoSvgTextShape::TextType type, int pos, KUndo2Command *parent = 0);
    ~KoSvgConvertTextTypeCommand() override = default;

    void redo() override;

    void undo() override;

    //int id() const override;
    //bool mergeWith(const KUndo2Command *other) override;

private:
    KoSvgTextShape *m_shape;
    KoSvgTextShapeMementoSP m_textData;

    KoSvgTextShape::TextType m_conversionType;
    int m_pos;
};

#endif // SVGCONVERTTEXTTYPECOMMAND_H
