/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOSVGTEXTFLIPSHAPETYPECOMMAND_H
#define KOSVGTEXTFLIPSHAPETYPECOMMAND_H

#include <kundo2command.h>
#include <kritaflake_export.h>

class KoSvgTextShape;
class KoShape;

class KRITAFLAKE_EXPORT KoSvgTextFlipShapeContourTypeCommand : public KUndo2Command
{
public:
    KoSvgTextFlipShapeContourTypeCommand(KoSvgTextShape* textShape, KoShape *shape, KUndo2Command *parent = nullptr);
    ~KoSvgTextFlipShapeContourTypeCommand();

    void redo() override;
    void undo() override;
private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KOSVGTEXTFLIPSHAPETYPECOMMAND_H
