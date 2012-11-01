/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef VIDEOSHAPE_H
#define VIDEOSHAPE_H

#include <KoShape.h>
#include <KoFrameShape.h>

#include <KIcon>

#include <QPixmap>

#define VIDEOSHAPEID "VideoShape"

class VideoCollection;
class VideoEventAction;
class VideoData;
#ifdef SHOULD_BUILD_THUMBNAIL
class VideoThumbnailer;
#endif

class QImage;

class VideoShape : public KoShape, public KoFrameShape
{
public:
    VideoShape();
    virtual ~VideoShape();

    // reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintcontext);
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
    void setVideoCollection(VideoCollection *collection);

    /**
     * return the video data used in this shape. Returns 0 if not set
     */
    VideoData *videoData() const;

protected:
    virtual bool loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context);

private slots:
    void updateThumbnail();

signals:
    void createThumbnail(VideoData *videoData, const QSize &size);

private:
    VideoCollection *m_videoCollection;
    VideoEventAction *m_videoEventAction;
#ifdef SHOULD_BUILD_THUMBNAIL
    VideoThumbnailer *m_thumbnailer;
#endif
    VideoData *m_oldVideoData;
    KIcon m_icon;
    QRectF m_playIconArea;
};

#endif
