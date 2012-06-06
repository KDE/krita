/* This file is part of the KDE project
 * Copyright (C) 2009 C. Boemann <cbo@boemann.dk>
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
#ifndef FULLSCREENPLAYER_H
#define FULLSCREENPLAYER_H

#include <QWidget>

namespace Phonon
{
    class MediaObject;
    class VideoWidget;
    class AudioOutput;
};

class QUrl;

/**
 * This class represents the click event action that starts the playing of the video.
 */
class FullScreenPlayer : public QWidget
{
    Q_OBJECT
public:
    FullScreenPlayer(const QUrl &);

    /// destructor
    virtual ~FullScreenPlayer();

protected slots:
    void stop();

protected:
    virtual void keyPressEvent(QKeyEvent *event); ///reimplemented
    virtual void mousePressEvent(QMouseEvent *event); ///reimplemented
    Phonon::MediaObject *m_mediaObject;
    Phonon::VideoWidget *m_videoWidget;
    Phonon::AudioOutput *m_audioOutput;
};

#endif
