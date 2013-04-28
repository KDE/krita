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

#ifndef CHANGEVIDEOCOMMAND_H
#define CHANGEVIDEOCOMMAND_H

#include <kundo2command.h>

class VideoData;
class VideoShape;

class ChangeVideoCommand : public KUndo2Command
{
public:
    ChangeVideoCommand(VideoShape *videoShape, VideoData *newVideoData, KUndo2Command *parent = 0);
    ~ChangeVideoCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    bool m_first;
    VideoData *m_oldVideoData;
    VideoData *m_newVideoData;
    VideoShape *m_shape;
};

#endif //CHANGEVIDEOCOMMAND_H
