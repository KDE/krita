/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef VIDEOSHAPE_H
#define VIDEOSHAPE_H

#include <QPixmap>
#include <KoShape.h>
#include <KoFrameShape.h>

#define VIDEOSHAPEID "VideoShape"

class VideoCollection;

class VideoShape : public KoShape, public KoFrameShape
{
public:
    VideoShape();
    virtual ~VideoShape();

    // reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    // reimplemented
    virtual void saveOdf(KoShapeSavingContext &context) const;
    // reimplemented
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    /**
     * Get the collection used in the shape.
     */
    VideoCollection *videoCollection() const;
    /**
     * Set the collection used in the shape.
     */
    void setVideoCollection(VideoCollection *collection) { m_videoCollection = collection; }

protected:
    virtual bool loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context);

private:
    VideoCollection *m_videoCollection;
};

#endif
