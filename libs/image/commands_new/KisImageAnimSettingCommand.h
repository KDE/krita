/*
 *  SPDX-FileCopyrightText: 2025 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISIMAGEANIMSETTINGCOMMAND_H
#define KISIMAGEANIMSETTINGCOMMAND_H

#include "kundo2command.h"
#include "KisAsynchronouslyMergeableCommandInterface.h"
#include "kis_image_animation_interface.h"

/*
 * Undo-able command for setting various animation-related
 * image settings, like FPS, start/end frame, etc.
 */
class KRITAIMAGE_EXPORT KisImageAnimSettingCommand : public KUndo2Command, public KisAsynchronouslyMergeableCommandInterface
{
public:
    struct Settings {
        int FPS = 0;
        int startFrame = 0;
        int endFrame = 24;
    };

    KisImageAnimSettingCommand() = delete;
    KisImageAnimSettingCommand(const KisImageAnimSettingCommand &) = delete;
    KisImageAnimSettingCommand & operator=(const KisImageAnimSettingCommand &) = delete;

    KisImageAnimSettingCommand(KisImageAnimationInterface *const p_animInterface, Settings p_after, KUndo2Command *parent = nullptr);

    void redo() override; // SET image animation settings to after values..
    void undo() override; // RESET image animation settings back to before values..

    int id() const override; // Used for merging commands.
    bool canMergeWith(const KUndo2Command *command) const override;
    bool mergeWith(const KUndo2Command *p_next) override;

private:
    KisImageAnimationInterface *m_animInterface;

    Settings m_before;
    Settings m_after;
};

#endif // KISIMAGEANIMSETTINGCOMMAND_H
