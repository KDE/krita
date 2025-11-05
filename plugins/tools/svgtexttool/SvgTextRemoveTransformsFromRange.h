/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGTEXTREMOVETRANSFORMSFROMRANGE_H
#define SVGTEXTREMOVETRANSFORMSFROMRANGE_H

#include <kundo2command.h>
#include <KoSvgTextShape.h>

/**
 * @brief The SvgTextRemoveTransformsFromRange class
 * Removes the SVG 1.1 character transforms from the range.
 */

class SvgTextRemoveTransformsFromRange : public KUndo2Command
{
public:
    SvgTextRemoveTransformsFromRange(KoSvgTextShape *shape, int pos, int anchor, KUndo2Command *parent = nullptr);

    void undo() override;
    void redo() override;
private:

    KoSvgTextShape *m_shape;
    int m_pos;
    int m_anchor;
    KoSvgTextShapeMementoSP m_textData;
};

#endif // SVGTEXTREMOVETRANSFORMSFROMRANGE_H
