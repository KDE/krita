/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COMMAND_IDS_H
#define KIS_COMMAND_IDS_H

namespace KisCommandUtils {
enum CommandId {
    MoveShapeId = 9999,
    ResizeShapeId,
    TransformShapeId,
    ChangeShapeTransparencyId,
    ChangeShapeBackgroundId,
    ChangeShapeStrokeId,
    ChangeShapeMarkersId,
    ChangeShapeParameterId,
    ChangeEllipseShapeId,
    ChangeRectangleShapeId,
    ChangePathShapePointId,
    ChangePathShapeControlPointId,
    ChangePaletteId,
    TransformToolId,
    ChangeNodeOpacityId,
    ChangeNodeNameId,
    ChangeNodeCompositeOpId,
    ChangeCurrentTimeId,
    DisableUIUpdatesCommandId,
    UpdateCommandId,
    EmitImageSignalsCommandId,
    NodePropertyListCommandId,
    ChangeStoryboardChild,
    ChangeTransformMaskCommand,
    CropReferenceImageId
};

}

#endif // KIS_COMMAND_IDS_H

