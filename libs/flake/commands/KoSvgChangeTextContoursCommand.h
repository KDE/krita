/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOSVGCHANGETEXTCONTOURSCOMMAND_H
#define KOSVGCHANGETEXTCONTOURSCOMMAND_H

#include <kundo2command.h>
#include <kritaflake_export.h>

class KoSvgTextShape;
class KoShape;

/**
 * @brief The KoSvgChangeTextContoursCommand class
 * This replaces the given shapes on the text shape.
 *
 * bool inside indicates whether it will set on shapesInside or shapesSubtract.
 */

class KRITAFLAKE_EXPORT KoSvgChangeTextContoursCommand : public KUndo2Command
{
public:
    KoSvgChangeTextContoursCommand(KoSvgTextShape *textShape, QList<KoShape*> shapes, bool inside, KUndo2Command *parent = nullptr);
    ~KoSvgChangeTextContoursCommand();

    void redo() override;
    void undo() override;
private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KOSVGCHANGETEXTCONTOURSCOMMAND_H
