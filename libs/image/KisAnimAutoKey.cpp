/*
 *  SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimAutoKey.h"

#include <QReadWriteLock>

#include <QApplication>
#include <QThread>

#include "kis_image_config.h"
#include "KisImageConfigNotifier.h"

#include "kis_transaction.h"
#include "kis_raster_keyframe_channel.h"


namespace KisAutoKey
{

class AutoKeyFrameStateHolder : public QObject
{
    Q_OBJECT
public:

    AutoKeyFrameStateHolder() {
        KIS_SAFE_ASSERT_RECOVER_NOOP(qApp->thread() == QThread::currentThread());

        connect(KisImageConfigNotifier::instance(), SIGNAL(autoKeyFrameConfigurationChanged()),
                this, SLOT(slotAutoKeyFrameSettingChanged()));
        slotAutoKeyFrameSettingChanged();
    }

    Mode mode() const {
        QReadLocker l(&m_lock);
        return m_mode;
    }

    void testingSetActiveMode(Mode mode) {
        QWriteLocker l(&m_lock);
        m_mode = mode;
    }

private Q_SLOTS:
    void slotAutoKeyFrameSettingChanged() {
        QWriteLocker l(&m_lock);

        KisImageConfig cfg(true);

        if (cfg.autoKeyEnabled()) {
            m_mode = cfg.autoKeyModeDuplicate() ? KisAutoKey::DUPLICATE
                                                : KisAutoKey::BLANK;
        } else {
            m_mode = KisAutoKey::NONE;
        }
    }

private:
    mutable QReadWriteLock m_lock;
    KisAutoKey::Mode m_mode {NONE};
};

QScopedPointer<AutoKeyFrameStateHolder> s_holder;
void initHolder () {
    if (!s_holder) {
        s_holder.reset(new AutoKeyFrameStateHolder());
    }
}
Q_COREAPP_STARTUP_FUNCTION(initHolder)

Mode activeMode()
{
    return s_holder->mode();
}

KUndo2Command* tryAutoCreateDuplicatedFrame(KisPaintDeviceSP device, AutoCreateKeyframeFlags flags) {
    KUndo2Command* undo = nullptr;
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(device, undo);

    const bool isLodNMode = device->defaultBounds()->currentLevelOfDetail() > 0;

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!isLodNMode || flags & SupportsLod, undo);

    const KisAutoKey::Mode autoKeyMode = KisAutoKey::activeMode();
    if (autoKeyMode == KisAutoKey::NONE) return undo;

    KisRasterKeyframeChannel *channel = device->keyframeChannel();
    if (!channel) return undo;

    const int activeKeyframe = channel->activeKeyframeTime();
    const int targetKeyframe = device->defaultBounds()->currentTime();

    if (activeKeyframe == targetKeyframe) return undo;

    if (isLodNMode) {
        if (flags & AllowBlankMode && autoKeyMode == KisAutoKey::BLANK) {
            const QRect originalDirtyRect = device->exactBounds();
            KisTransaction transaction(device);
            device->clear();
            device->setDirty(originalDirtyRect);
            undo = transaction.endAndTake();
        }
        // for KisAutoKey::DUPLICATE mode we do nothing and just reuse
        // the LoDN device
    } else {
        undo = new KUndo2Command;

        if (flags & AllowBlankMode && autoKeyMode == KisAutoKey::BLANK) {
            channel->addKeyframe(targetKeyframe, undo);

            /// Use the same color label as previous keyframe, the duplicate
            /// action duplicates the label automatically, but addKeyframe not

            KisKeyframeSP newKeyframe = channel->keyframeAt(targetKeyframe);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(newKeyframe, undo);

            KisKeyframeSP oldKeyframe = channel->keyframeAt(activeKeyframe);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(oldKeyframe, undo);

            newKeyframe->setColorLabel(oldKeyframe->colorLabel());
        } else {
            channel->copyKeyframe(activeKeyframe, targetKeyframe, undo);
        }
    }

    return undo;
}

void testingSetActiveMode(Mode mode)
{
    s_holder->testingSetActiveMode(mode);
}


} // namespace KisAutoKey

#include "KisAnimAutoKey.moc"
