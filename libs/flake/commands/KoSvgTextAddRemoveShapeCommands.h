/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOSVGTEXTADDREMOVESHAPECOMMANDS_H
#define KOSVGTEXTADDREMOVESHAPECOMMANDS_H

#include "kis_command_utils.h"
#include <kundo2command.h>
#include <kritaflake_export.h>

class KoSvgTextShape;
class KoShape;

class KRITAFLAKE_EXPORT KoSvgTextAddRemoveShapeCommandImpl : public KisCommandUtils::FlipFlopCommand
{
public:
    enum ContourType {
        Unknown,
        Inside,
        Subtract,
        TextPath
    };

    KoSvgTextAddRemoveShapeCommandImpl(KoSvgTextShape *textShape, KoShape *shape, ContourType type, State state, int startPos, int endPos, KUndo2Command *parent = nullptr);
    ~KoSvgTextAddRemoveShapeCommandImpl();
    void partA() override;
    void partB() override;
private:
    struct Private;
    QScopedPointer<Private> d;
};

class KRITAFLAKE_EXPORT KoSvgTextAddShapeCommand : public KoSvgTextAddRemoveShapeCommandImpl {
public:
    KoSvgTextAddShapeCommand(KoSvgTextShape *textShape, KoShape *shape, bool inside, KUndo2Command *parentCommand = 0);
    ~KoSvgTextAddShapeCommand();
};

class KRITAFLAKE_EXPORT KoSvgTextRemoveShapeCommand : public KoSvgTextAddRemoveShapeCommandImpl {
public:
    KoSvgTextRemoveShapeCommand(KoSvgTextShape *textShape, KoShape *shape, KUndo2Command *parentCommand = 0);
    ~KoSvgTextRemoveShapeCommand();

    /**
     * @brief removeContourShapesFromFlow
     * Create a command to remove all contour shapes of a certain type from the flow
     * and add the command to the parent.
     * @param textShape -- textShape to work on.
     * @param parent -- parentCommand to add remove commands to.
     * @param textInShape -- remove all text-in-shape shapeInside and ShapeSubtract shapes.
     * @param textPaths -- remove all text paths.
     */
    static void removeContourShapesFromFlow(KoSvgTextShape *textShape, KUndo2Command *parent, bool textInShape, bool textPaths);
};

class KRITAFLAKE_EXPORT KoSvgTextSetTextPathOnRangeCommand : public KoSvgTextAddRemoveShapeCommandImpl {
public:
    KoSvgTextSetTextPathOnRangeCommand(KoSvgTextShape *textShape, KoShape *shape, int startPos, int endPos, KUndo2Command *parentCommand = 0);
    ~KoSvgTextSetTextPathOnRangeCommand();
};

#endif // KOSVGTEXTADDREMOVESHAPECOMMANDS_H
