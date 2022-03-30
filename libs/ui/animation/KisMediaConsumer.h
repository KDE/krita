/*
 *  SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMLTPLAYER_H
#define KISMLTPLAYER_H

#include <QObject>
#include <QSharedPointer>
#include <mlt++/MltProducer.h>
#include "KisPlaybackEngine.h"
#include "animation/KisFrameDisplayProxy.h"

class KisPlaybackHandle : public QObject
{
    Q_OBJECT

public:
    KisPlaybackHandle(QObject* parent = nullptr);
    ~KisPlaybackHandle();

    void seek(int p_frame);

    void pushAudio();

    void setFrameRate(int p_frameRate);
    int frameRate();

    void setMode(PlaybackMode p_setting);
    PlaybackMode getMode();

    void resync(const KisFrameDisplayProxy& displayProxy);

Q_SIGNALS:
    void sigFrameShow(int p_frame);
    void sigDesiredFrameChanged(int p_frame);
    void sigRequestPushAudio();
    void sigFrameRateChanged(int p_frameRate);
    void sigModeChange(PlaybackMode p_mode) const;

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISMLTPLAYER_H
