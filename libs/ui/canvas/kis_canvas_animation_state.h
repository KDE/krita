/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ANIMATION_PLAYER_H
#define KIS_ANIMATION_PLAYER_H

#include <QScopedPointer>
#include <QObject>
#include <QFileInfo>

#include <kis_time_span.h>
#include <boost/optional.hpp>

#include "kritaui_export.h"


class KisCanvas2;

enum PlaybackState : unsigned int {
    STOPPED,
    PAUSED,
    PLAYING
};

/**
 * @brief The KisAnimationPlayer class is Krita's high-level
 * animation playback and navigation interface.
 * Its main clients are Krita's Timeline and Curves dockers.
 * It makes heavy use of KisImageAnimationInterface.
 */
class KRITAUI_EXPORT KisCanvasAnimationState : public QObject
{
    Q_OBJECT

public:
    KisCanvasAnimationState(KisCanvas2 *canvas);
    ~KisCanvasAnimationState() override;

    void setPlaybackState(PlaybackState state);
    PlaybackState playbackState();
    boost::optional<QFileInfo> mediaInfo();

    boost::optional<int> playbackOrigin();

    class KisFrameDisplayProxy const *displayProxy() const;

public Q_SLOTS:
    void showFrame(int frame);

Q_SIGNALS:
    void sigPlaybackStateChanged(PlaybackState state);
    void sigPlaybackStatisticsUpdated();
    void sigFrameChanged();
    void sigPlaybackMediaChanged();

    void sigCancelPlayback();

private:
    void updateDropFramesMode();
    KisTimeSpan activePlaybackRange();

    void setupAudioTracks();

    struct Private;
    QScopedPointer<Private> m_d;
};


class KRITAUI_EXPORT SingleShotSignal : public QObject {
    Q_OBJECT
public:
    SingleShotSignal(QObject* parent = nullptr)
        : QObject(parent)
        , lock(false)
    {
    }

    ~SingleShotSignal() {}

public Q_SLOTS:
    void tryFire() {
        if (!lock) {
            lock = true;
            emit output();
        }
    }

Q_SIGNALS:
    void output();

private:
    bool lock;

};


#endif
