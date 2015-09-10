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

#include "ChangeVideoCommand.h"

#include "VideoData.h"
#include "VideoShape.h"
#include "VideoCollection.h"

#include <klocalizedstring.h>

ChangeVideoCommand::ChangeVideoCommand(VideoShape *videoShape, VideoData *newVideoData, KUndo2Command *parent)
    : KUndo2Command(parent),
      m_first(true),
      m_newVideoData(newVideoData),
      m_shape(videoShape)
{
    setText(kundo2_i18n("Change video"));

    m_oldVideoData = m_shape->videoData() ? new VideoData(*(m_shape->videoData())) : 0;
}

ChangeVideoCommand::~ChangeVideoCommand()
{
    delete m_oldVideoData;
    delete m_newVideoData;
}

void ChangeVideoCommand::redo()
{
    // we need new here as setUserData deletes the old data
    m_shape->setUserData(m_newVideoData ? new VideoData(*m_newVideoData): 0);
}

void ChangeVideoCommand::undo()
{
    // we need new here as setUserData deletes the old data
    m_shape->setUserData(m_oldVideoData ? new VideoData(*m_oldVideoData): 0);
}
