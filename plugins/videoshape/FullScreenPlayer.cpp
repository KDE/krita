/* This file is part of the KDE project
 * Copyright (C) 2010 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
 
#include "FullScreenPlayer.h"

#include <KoIcon.h>

#include <klocalizedstring.h>

#include <phonon/videowidget.h>
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>
#include <phonon/VolumeSlider>
#include <phonon/SeekSlider>

#include <QUrl>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QToolButton>

FullScreenPlayer::FullScreenPlayer(const QUrl &url)
    : QWidget(0)
    , m_seekSlider(new Phonon::SeekSlider(this))
    , m_volumeSlider(new Phonon::VolumeSlider(this))
{
    m_mediaObject = new Phonon::MediaObject();
    m_mediaObject->setTickInterval(1000);

    m_videoWidget = new Phonon::VideoWidget(this);
    Phonon::createPath(m_mediaObject, m_videoWidget);

    m_audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory);
    connect(m_audioOutput, SIGNAL(mutedChanged(bool)), this, SLOT(muteStateChanged(bool)));

    Phonon::createPath(m_mediaObject, m_audioOutput);

    m_seekSlider->setMediaObject(m_mediaObject);
    m_seekSlider->setIconVisible(false);

    m_volumeSlider->setAudioOutput(m_audioOutput);
    m_volumeSlider->setMuteVisible(false);
    m_volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    m_playbackTime = new QLabel(QString("00:00:00"), this);

    m_play = new QToolButton(this);
    m_play->setIcon(koIcon("media-playback-start"));
    m_play->setToolTip(i18n("Play"));
    connect(m_play, SIGNAL(clicked()), this, SLOT(play()));

    m_pause = new QToolButton(this);
    m_pause->setIcon(koIcon("media-playback-pause"));
    m_pause->setToolTip(i18n("Pause"));
    connect(m_pause, SIGNAL(clicked()), this, SLOT(pause()));

    m_stop = new QToolButton(this);
    m_stop->setIcon(koIcon("media-playback-stop"));
    m_stop->setToolTip(i18n("Stop"));
    connect(m_stop, SIGNAL(clicked()), this, SLOT(stop()));

    m_volumeIconMuted = new QToolButton(this);
    m_volumeIconMuted->setIcon(koIcon("audio-volume-muted"));
    m_volumeIconMuted->setToolTip(i18n("Unmute"));
    connect(m_volumeIconMuted, SIGNAL(clicked()), this, SLOT(unmute()));

    m_volumeIconUnmuted = new QToolButton(this);
    m_volumeIconUnmuted->setIcon(koIcon("audio-volume-medium"));
    m_volumeIconUnmuted->setToolTip(i18n("Mute"));
    connect(m_volumeIconUnmuted, SIGNAL(clicked()), this, SLOT(mute()));

    QHBoxLayout *playbackControls = new QHBoxLayout();
    playbackControls->addWidget(m_play);
    playbackControls->addWidget(m_pause);
    playbackControls->addWidget(m_stop);
    playbackControls->addWidget(m_seekSlider);
    playbackControls->addWidget(m_playbackTime);
    playbackControls->addWidget(m_volumeIconMuted);
    playbackControls->addWidget(m_volumeIconUnmuted);
    playbackControls->addWidget(m_volumeSlider);
    playbackControls->setSizeConstraint(QLayout::SetFixedSize);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_videoWidget);
    layout->addLayout(playbackControls);
    layout->setMargin(0);
    setLayout(layout);
    show();
    setWindowState(Qt::WindowFullScreen);

    m_mediaObject->setCurrentSource(url);
    connect(m_mediaObject, SIGNAL(finished()), this, SLOT(stop()));
    connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(playStateChanged(Phonon::State,Phonon::State)));
    connect(m_mediaObject, SIGNAL(tick(qint64)), this, SLOT(updatePlaybackTime(qint64)));

    play();

    mute();
    unmute();
}

FullScreenPlayer::~FullScreenPlayer()
{
}

void FullScreenPlayer::play()
{

    m_mediaObject->play();
}

void FullScreenPlayer::pause()
{
    m_mediaObject->pause();
}

void FullScreenPlayer::stop()
{
    m_mediaObject->stop();
    deleteLater();
}

void FullScreenPlayer::mute()
{
    qreal volume = m_audioOutput->volume();
    m_audioOutput->setMuted(true);
    m_audioOutput->setVolume(volume); //
}

void FullScreenPlayer::unmute()
{
    m_audioOutput->setMuted(false);
}

void FullScreenPlayer::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_mediaObject->stop();
    deleteLater();
}

void FullScreenPlayer::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Escape) {
       m_mediaObject->stop();
       deleteLater();
    }
}

void FullScreenPlayer::playStateChanged(Phonon::State newState, Phonon::State oldState)
{
    Q_UNUSED(oldState);

    switch (newState) {
        case Phonon::PlayingState:
                m_play->setVisible(false);
                m_pause->setVisible(true);
                break;
        case Phonon::PausedState:
                m_play->setVisible(true);
                m_pause->setVisible(false);
                break;
        default:
            ;
    }
}

void FullScreenPlayer::updatePlaybackTime(qint64 currentTime)
{
    QString currentPlayTime = QString("%1:%2:%3")
            .arg((currentTime / 3600000) % 60, 2, 10, QChar('0'))
            .arg((currentTime / 60000) % 60, 2, 10, QChar('0'))
            .arg((currentTime / 1000) % 60, 2, 10, QChar('0'));

    qint64 time = m_mediaObject->totalTime();
    QString totalTime = QString("%1:%2:%3")
            .arg((time / 3600000) % 60, 2, 10, QChar('0'))
            .arg((time / 60000) % 60, 2, 10, QChar('0'))
            .arg((time / 1000) % 60, 2, 10, QChar('0'));

    m_playbackTime->setText(QString("%1/%2").arg(currentPlayTime).arg(totalTime));
}


void FullScreenPlayer::muteStateChanged(bool muted)
{
    if (muted) {
        m_volumeIconMuted->setVisible(true);
        m_volumeIconUnmuted->setVisible(false);
    } else {
        m_volumeIconMuted->setVisible(false);
        m_volumeIconUnmuted->setVisible(true);
    }
}
