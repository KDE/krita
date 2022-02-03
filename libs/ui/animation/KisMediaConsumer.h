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
#include "animation/KisFrameDisplayProxy.h"

class KisMediaConsumer : public QObject
{
    Q_OBJECT

public:
    enum Mode {
        PUSH,
        PULL
    };

    KisMediaConsumer(QObject* parent = nullptr);
    ~KisMediaConsumer();

    void seek(int p_frame);
    void pushAudio();
    int playhead();

    void setFrameRate(int fps);
    int getFrameRate();

    void setMode(Mode setting);
    Mode getMode();

    void resync(const KisFrameDisplayProxy& displayProxy);
    QString debugInfo();

    Mlt::Profile* getProfile();

    void setProducer(QSharedPointer<Mlt::Producer> p_producer);

Q_SIGNALS:
    void sigFrameShow(int frameIndex) const;

private:
    QScopedPointer<struct Private> m_d;

};

#endif // KISMLTPLAYER_H
