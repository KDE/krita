/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
   SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KISPLAYBACKENGINEQT_H
#define KISPLAYBACKENGINEQT_H

#include "KisPlaybackEngine.h"

#include <kritaui_export.h>

/**
 * @brief The KisPlaybackEngineQT class is an implementation of KisPlaybackEngine
 * that drives animation playback using simple Qt functionality alone.
 *
 * As of right now, this playback engine is used as a fallback for when KisPlaybackEngineMLT is unavailable,
 * invalid, or otherwise unwanted.
 */
class KRITAUI_EXPORT KisPlaybackEngineQT : public KisPlaybackEngine
{
    Q_OBJECT
public:
    explicit KisPlaybackEngineQT(QObject *parent = nullptr);
    ~KisPlaybackEngineQT();

    virtual void seek(int frameIndex, SeekOptionFlags flags = SEEK_FINALIZE | SEEK_PUSH_AUDIO) override;

    virtual void setPlaybackSpeedPercent(int percentage) override;
    virtual void setPlaybackSpeedNormalized(double value) override;

    virtual void setMute(bool) override {}
    virtual bool isMute() override { return true; }

    virtual bool supportsAudio() override { return false; }
    virtual bool supportsVariablePlaybackSpeed() override { return true; }

protected Q_SLOTS:
    /**
     * @brief throttledQtFrameTimeExpired handles signals from the Qt timer
     * that drives playback within this engine. Increments frame time,
     * wrapping within bounds, and communicates with KisFrameDisplayProxy.
     */
    void throttledQtFrameTimeExpired();

protected:
    virtual void setCanvas(KoCanvasBase* canvas) override;
    virtual void unsetCanvas() override;

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISPLAYBACKENGINEQT_H
