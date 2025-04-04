#include "KisImageAnimSettingCommand.h"
#include "kis_time_span.h"

KisImageAnimSettingCommand::KisImageAnimSettingCommand(KisImageAnimationInterface *const p_animInterface, Settings p_after, KUndo2Command *parent = nullptr)
    : KUndo2Command(parent),
      m_animInterface(p_animInterface),
      m_after(p_after)
{
    m_before = {
                    p_animInterface->framerate(),
                    p_animInterface->documentPlaybackRange().start(),
                    p_animInterface->documentPlaybackRange().end()
               };
}

void KisImageAnimSettingCommand::redo()
{
    // SET image animation settings to after values..
    KIS_ASSERT(m_animInterface);

    m_animInterface->setFramerate(m_after.FPS);
    m_animInterface->setDocumentRange(KisTimeSpan::fromTimeToTime(m_after.startFrame, m_after.endFrame));
}

void KisImageAnimSettingCommand::undo()
{
    // RESET image animation settings back to before values..
    KIS_ASSERT(m_animInterface);

    m_animInterface->setFramerate(m_before.FPS);
    m_animInterface->setDocumentRange(KisTimeSpan::fromTimeToTime(m_before.startFrame, m_before.endFrame));
}
