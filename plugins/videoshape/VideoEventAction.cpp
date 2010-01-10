/* This file is part of the KDE project
 * Copyright (C) 2009 Casper Boemann <cbo@boemann.dk>
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

#include "VideoEventAction.h"

#include <kdebug.h>

#include <QString>
#include <QUrl>
#include <Phonon/VideoWidget>
#include <Phonon/AudioOutput>
#include <Phonon/MediaObject>

#include "VideoData.h"
#include "VideoShape.h"

VideoEventAction::VideoEventAction(VideoShape *parent)
    : KoEventAction()
    ,m_shape(parent)
    ,m_mediaObject(0)
    ,m_videoWidget(0)
    ,m_audioOutput(0)
{
    setId(QString("videoeventaction"));
}

VideoEventAction::~VideoEventAction()
{
}

bool VideoEventAction::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    return true;
}

void VideoEventAction::saveOdf(KoShapeSavingContext &context) const
{
}

void VideoEventAction::start()
{
    qDebug() << "action activated" << endl;
    m_mediaObject = new Phonon::MediaObject();

    m_videoWidget = new Phonon::VideoWidget();
    Phonon::createPath(m_mediaObject, m_videoWidget);

    m_audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory);
    Phonon::createPath(m_mediaObject, m_audioOutput);

    VideoData *videoData = qobject_cast<VideoData*>(m_shape->userData());
m_videoWidget->show();
m_videoWidget->setFullScreen(true);
    m_mediaObject->setCurrentSource(QUrl(":/home/cbo/FALL98.MPG"));//videoData->videoLocation);
    m_mediaObject->play();
}

void VideoEventAction::finish()
{
}

