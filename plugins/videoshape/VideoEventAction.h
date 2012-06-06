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
#ifndef VIDEOEVENTACTION_H
#define VIDEOEVENTACTION_H

#include <KoEventAction.h>

class VideoShape;
class FullScreenPlayer;

/**
 * This class represents the click event action that starts the playing of the video.
 */
class VideoEventAction : public KoEventAction
{
public:
    VideoEventAction(VideoShape *parent);

    /// destructor
    virtual ~VideoEventAction();

    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
    virtual void saveOdf(KoShapeSavingContext &context) const;

    virtual void start();
    virtual void finish();

protected:
    VideoShape *m_shape;
    FullScreenPlayer *m_player;
};

#endif
