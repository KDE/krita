 /* This file is part of the KDE project
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

#ifndef VIDEOTHUMBNAILER_H
#define VIDEOTHUMBNAILER_H

#include <phonon/MediaObject>
#include <phonon/Global>
#include <phonon/experimental/videodataoutput2.h>

#include <QObject>
#include <QImage>
#include <QSize>
#include <QEventLoop>

class VideoData;

namespace Phonon
{
    namespace Experimental
    {
        class VideoDataOutput2;
    }
    class MediaObject;
}

class VideoThumbnailer : public QObject
{
    Q_OBJECT

public:
    VideoThumbnailer();
    ~VideoThumbnailer();

    void createThumbnail(VideoData *videoData, const QSize &size);
    QImage thumbnail();

Q_SIGNALS:
    void thumbnailReady();
    void signalCreateThumbnail(VideoData *videoData, const QSize &size);

private Q_SLOTS:
    void slotCreateThumbnail(VideoData *videoData, const QSize &size);
    void frameReady(const Phonon::Experimental::VideoFrame2 & frame);
    void stateChanged(Phonon::State newState, Phonon::State oldState);

private:
    static bool isFrameInteresting(const QImage &frame);
    Phonon::MediaObject m_media;
    Phonon::Experimental::VideoDataOutput2 m_vdata;
    QSize m_thumbnailSize;
    QEventLoop m_eventLoop;
    QImage m_thumbnailImage;
};

#endif //VIDEOTHUMBNAILER_H
