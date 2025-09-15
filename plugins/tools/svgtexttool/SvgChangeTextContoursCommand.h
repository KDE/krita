/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGCHANGETEXTCONTOURSCOMMAND_H
#define SVGCHANGETEXTCONTOURSCOMMAND_H

#include <kundo2command.h>

class KoSvgTextShape;
class KoShape;

// TODO: This only supports shapeInside for now.

class SvgChangeTextContoursCommand : public KUndo2Command
{
public:
    SvgChangeTextContoursCommand(KoSvgTextShape *textShape, QList<KoShape*> shapes, KUndo2Command *parent = nullptr);

    void redo() override;
    void undo() override;
private:
    KoSvgTextShape *m_textShape;
    QList<KoShape*> m_oldShapes;
    QList<KoShape*> m_newShapes;
};

#endif // SVGCHANGETEXTCONTOURSCOMMAND_H
