/* This file is part of the KDE project
   Copyright (c) 2001 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002, 2003, 2004 Nicolas GOUTTE <goutte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef __koPictureImage_h__
#define __koPictureImage_h__

#include "KoPictureBase.h"
#include <qstring.h>
//Added by qt3to4:
#include <QPixmap>

class KoPictureImagePrivate;
// TODO: fix documentation

/**
 * @internal
 * KoPictureImage is a container class for a QImage-based picture
 * \todo remove private class, as the header is not installed
 */
class KoPictureImage : public KoPictureBase
{
public:
    /**
     * Default constructor.
     */
    KoPictureImage();

    /**
     * Destructor.
     */
    virtual ~KoPictureImage();

    KoPictureType::Type getType(void) const;

    KoPictureBase* newCopy(void) const;

    /**
     * Returns true if the picture is null.
     */
    virtual bool isNull(void) const;

    /**
     * @brief Draw the image in a painter.
     *
     * No, this isn't as simple as painter.drawPixmap().
     * This method ensures that the best quality is used when printing, scaling the painter.
     *
     * The parameter @p fastMode allows the picture to be re-sized and drawn quicker if possible
     *
     * The parameters @p width, @p height define the desired size for the image
     * Note that the image is being scaled to that size using scale() - except when printing.
     * This avoids scaling the image at each paint event.
     *
     * The other parameters are very similar to QPainter::drawPixmap :
     * (@p x, @p y) define the position in the painter,
     * (@p sx, @p sy) specify the top-left point in pixmap that is to be drawn. The default is (0, 0).
     * (@p sw, @p sh) specify the size of the pixmap that is to be drawn. The default, (-1, -1), means all the way to the bottom
     * right of the pixmap.
     */
    virtual void draw(QPainter& painter, int x, int y, int width, int height, int sx = 0, int sy = 0, int sw = -1, int sh = -1, bool fastMode = false);

    virtual Q3DragObject* dragObject( QWidget *dragSource = 0L, const char *name = 0L );

    virtual bool loadData(const QByteArray& array, const QString& extension);

    virtual bool save(QIODevice* io) const;

    virtual QSize getOriginalSize(void) const;

    virtual QPixmap generatePixmap(const QSize& size, bool smoothScale = false);

    virtual QString getMimeType(const QString& extension) const;

    /**
     * Generate a QImage
     * (always in slow mode)
     */
    virtual QImage generateImage(const QSize& size);

    virtual bool hasAlphaBuffer() const
        { return m_originalImage.hasAlphaBuffer(); }

    virtual void setAlphaBuffer(bool enable)
        { m_originalImage.setAlphaBuffer(enable); }

    virtual QImage createAlphaMask(Qt::ImageConversionFlags flags = Qt::AutoColor) const
        { return m_originalImage.createAlphaMask(flags); }

    virtual void clearCache(void);

protected:
    QPixmap getPixmap(QImage& image);
    void scaleAndCreatePixmap(const QSize& size, bool fastMode=false);

private:
    QImage  m_originalImage; ///< Image as Qimage
    QByteArray m_rawData; ///< Binary copy of the loaded image file
    QPixmap m_cachedPixmap; ///< Cached pixmap
    QSize m_cachedSize; ///< size of the currently cached pixmap @see m_cachedPixmap
    /**
     * true, if the last cached image was done using fast mode.
     * false, if the last cached image was done using slow mode.
     */
    bool m_cacheIsInFastMode;
    class Private;
    Private* d;
};

#endif /* __koPictureImage_h__ */
