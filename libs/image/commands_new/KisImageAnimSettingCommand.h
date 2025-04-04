#ifndef KISIMAGEANIMSETTINGCOMMAND_H
#define KISIMAGEANIMSETTINGCOMMAND_H

#include "kundo2command.h"
#include "kis_image_animation_interface.h"

/*
 * Undo-able command for setting various animation-related
 * image settings, like FPS, start/end frame, etc.
 */
class KRITAIMAGE_EXPORT KisImageAnimSettingCommand : public KUndo2Command
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

    KisImageAnimSettingCommand(KisImageAnimationInterface *const p_animInterface, Settings p_after, KUndo2Command *parent);

    void redo(); // SET image animation settings to after values..
    void undo(); // RESET image animation settings back to before values..

    // TODO: Add command merge support...

private:
    KisImageAnimationInterface *m_animInterface;
    Settings m_before;
    Settings m_after;
};

#endif // KISIMAGEANIMSETTINGCOMMAND_H
