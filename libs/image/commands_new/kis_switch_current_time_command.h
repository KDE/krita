/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SWITCH_CURRENT_TIME_COMMAND_H
#define __KIS_SWITCH_CURRENT_TIME_COMMAND_H

#include "kritaimage_export.h"
#include "kis_types.h"

#include <kundo2command.h>

class KisImageAnimationInterface;


class KRITAIMAGE_EXPORT KisSwitchCurrentTimeCommand : public KUndo2Command
{
public:
    KisSwitchCurrentTimeCommand(KisImageAnimationInterface *animation, int oldTime, int newTime, KUndo2Command *parent = 0);
    ~KisSwitchCurrentTimeCommand() override;

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command* command) override;

private:
    KisImageAnimationInterface *m_animation;
    int m_oldTime;
    int m_newTime;
};

#endif /* __KIS_SWITCH_CURRENT_TIME_COMMAND_H */
