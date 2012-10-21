/* This file is part of the KDE project
* Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
* Copyright (C) 2006-2009 Marco Gulino <marco.gulino@gmail.com>
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

#include "VideoThumbnailer.h"

#include "VideoData.h"

#include <Phonon/Experimental/VideoFrame2>
#include <QTime>
#include <QVBoxLayout>
#include <QVarLengthArray>

#include <KDebug>

#define THRESHOLD_FRAME_VARIANCE 40.0

VideoThumbnailer::VideoThumbnailer()
    : QObject()
{
    m_vdata.setRunning(true);
    Phonon::createPath(&m_media, &m_vdata);
    connect(&m_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this,
        SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(this,SIGNAL(signalCreateThumbnail(VideoData*,QSize)),
            this, SLOT(slotCreateThumbnail(VideoData*,QSize)), Qt::QueuedConnection);
}

VideoThumbnailer::~VideoThumbnailer()
{
}

void VideoThumbnailer::createThumbnail(VideoData *videoData, const QSize &size)
{
    emit signalCreateThumbnail(videoData, size);
}

void VideoThumbnailer::slotCreateThumbnail(VideoData *videoData, const QSize &size)
{
        m_media.setCurrentSource(videoData->playableUrl());
        m_media.play();

        m_thumbnailSize = size;


        int retcode = 0;
        for (int i = 0; i < 50; i++) {
            retcode = m_eventLoop.exec();
            if (retcode == 0) {
                break;
            }
            kDebug() << "Seeking to " << (i * 3);
            m_media.seek(i * 3);
        }
        if (retcode) {
               kWarning() << "Unable to generate thumbnail for ";
               m_media.stop();
               return;
        }
        m_media.stop();

        emit thumbnailReady();
}

void VideoThumbnailer::frameReady(const Phonon::Experimental::VideoFrame2 &frame)
{
    QImage thumb = frame.qImage().scaled(m_thumbnailSize.width(), m_thumbnailSize.height(), Qt::KeepAspectRatio);
    if (isFrameInteresting(thumb)) {
        m_thumbnailImage = thumb;
        m_vdata.disconnect(SIGNAL(frameReadySignal(const Phonon::Experimental::VideoFrame2 &)),
            this, SLOT(frameReady(const Phonon::Experimental::VideoFrame2 &)));
        m_eventLoop.quit();
        return;
    }
    m_eventLoop.exit(1);
}

void VideoThumbnailer::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    Q_UNUSED(oldState);
    if (newState == Phonon::PlayingState) {
        connect(&m_vdata, SIGNAL(frameReadySignal(const Phonon::Experimental::VideoFrame2 &)),
            this, SLOT(frameReady(const Phonon::Experimental::VideoFrame2 &)));
        m_eventLoop.exit(1);
    }
}

QImage VideoThumbnailer::thumbnail()
{
    return m_thumbnailImage;
}

bool VideoThumbnailer::isFrameInteresting(const QImage &frame)
{
    float variance = 0;
    //taken from mplayerthumbs
    uint delta=0;
    uint avg=0;
    uint bytes=frame.numBytes();
    uint STEPS=bytes/2;
    QVarLengthArray<uchar> pivot(STEPS);

    const uchar *bits=frame.bits();
    // First pass: get pivots and taking average
    for( uint i=0; i<STEPS ; i++ ){
        pivot[i]=bits[i*(bytes/STEPS)];
        avg+=pivot[i];
    }
    avg=avg/STEPS;
    // Second Step: calculate delta (average?)
    for (uint i=0; i<STEPS; i++)
    {
        int curdelta=abs(int(avg-pivot[i]));
        delta+=curdelta;
    }
    variance= delta/STEPS;

    return variance > THRESHOLD_FRAME_VARIANCE;
}


