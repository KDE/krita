/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CHANGE_PROJECTION_COLOR_COMMAND_H
#define __KIS_CHANGE_PROJECTION_COLOR_COMMAND_H

#include "kritaimage_export.h"
#include "kis_types.h"

#include <KoColor.h>

#include <kundo2command.h>


class KRITAIMAGE_EXPORT KisChangeProjectionColorCommand : public KUndo2Command
{
public:
    KisChangeProjectionColorCommand(KisImageSP image, const KoColor &newColor, KUndo2Command *parent = 0);
    ~KisChangeProjectionColorCommand() override;

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command* command) override;

private:
    KisImageWSP m_image;
    KoColor m_oldColor;
    KoColor m_newColor;
};

#endif /* __KIS_CHANGE_PROJECTION_COLOR_COMMAND_H */
