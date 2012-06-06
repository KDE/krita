/* This file is part of the KDE project
 * Copyright (C) 2009-2010 C. Boemann <cbo@boemann.dk>
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

#include <KoXmlReader.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <kdebug.h>

#include <QUrl>

#include "VideoData.h"
#include "VideoShape.h"

#include "FullScreenPlayer.h"

VideoEventAction::VideoEventAction(VideoShape *parent)
    : KoEventAction()
    ,m_shape(parent)
    ,m_player(0)
{
    setId(QString("videoeventaction"));
}

VideoEventAction::~VideoEventAction()
{
}

bool VideoEventAction::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    return true;
}

void VideoEventAction::saveOdf(KoShapeSavingContext &context) const
{
    Q_UNUSED(context);
}

void VideoEventAction::start()
{
    VideoData *videoData = qobject_cast<VideoData*>(m_shape->userData());
    Q_ASSERT(videoData);
    m_player = new FullScreenPlayer(videoData->playableUrl());
}

void VideoEventAction::finish()
{
}

