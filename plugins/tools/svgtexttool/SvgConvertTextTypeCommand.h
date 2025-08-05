/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGCONVERTTEXTTYPECOMMAND_H
#define SVGCONVERTTEXTTYPECOMMAND_H

#include <kundo2command.h>
#include "kritatoolsvgtext_export.h"
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
class KRITATOOLSVGTEXT_EXPORT SvgConvertTextTypeCommand : public KUndo2Command
{
public:
    enum ConversionType {
        ToPreFormatted, ///< Apply whitespace, convert whitespace rule to pre-wrapped, insert newlines at chunk starts and remove character transforms. Removes InlineSize.
        ToInlineSize ///< Same as ToPreFormatted, except adds an inline-size.
    };

    SvgConvertTextTypeCommand(KoSvgTextShape *shape, ConversionType type, KUndo2Command *parent = 0);
    ~SvgConvertTextTypeCommand() override = default;

    void redo() override;

    void undo() override;

    //int id() const override;
    //bool mergeWith(const KUndo2Command *other) override;

private:
    KoSvgTextShape *m_shape;
    KoSvgTextShapeMementoSP m_textData;

    ConversionType m_conversionType;
};

#endif // SVGCONVERTTEXTTYPECOMMAND_H
