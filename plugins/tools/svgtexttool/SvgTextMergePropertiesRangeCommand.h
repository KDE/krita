/*
 * SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGTEXTMERGEPROPERTIESRANGECOMMAND_H
#define SVGTEXTMERGEPROPERTIESRANGECOMMAND_H

#include <kundo2command.h>
#include "kritatoolsvgtext_export.h"

#include "KoSvgTextProperties.h"

class KoSvgTextShape;
class KoSvgTextProperties;

class KRITATOOLSVGTEXT_EXPORT SvgTextMergePropertiesRangeCommand : public KUndo2Command
{
public:
    SvgTextMergePropertiesRangeCommand(KoSvgTextShape *shape, KoSvgTextProperties props, int pos, int anchor, KUndo2Command *parent = 0);
    ~SvgTextMergePropertiesRangeCommand() override = default;

    void redo() override;

    void undo() override;
private:
    KoSvgTextShape *m_shape;
    KoSvgTextProperties m_props;

    int m_pos;
    int m_anchor;
    QString m_oldSvg;
    QString m_oldDefs;
};

#endif // SVGTEXTMERGEPROPERTIESRANGECOMMAND_H
