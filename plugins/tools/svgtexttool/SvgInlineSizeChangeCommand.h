/*
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SVG_INLINE_SIZE_CHANGE_COMMAND_H
#define SVG_INLINE_SIZE_CHANGE_COMMAND_H

#include <kundo2command.h>

class KoSvgTextShape;

class SvgInlineSizeChangeCommand : public KUndo2Command
{
public:
    SvgInlineSizeChangeCommand(KoSvgTextShape *shape, double inlineSize, KUndo2Command *parent = nullptr);
    SvgInlineSizeChangeCommand(KoSvgTextShape *shape,
                               double inlineSize,
                               double oldInlineSize,
                               KUndo2Command *parent = nullptr);
    ~SvgInlineSizeChangeCommand() override = default;

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *other) override;

private:
    void applyInlineSize(double inlineSize);

private:
    KoSvgTextShape *m_shape;
    double m_inlineSize;
    double m_oldInlineSize;
};

#endif /* SVG_INLINE_SIZE_CHANGE_COMMAND_H */
