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
#ifndef __koPictureEps_h__
#define __koPictureEps_h__

#include <QString>
//Added by qt3to4:
#include <QPixmap>

class QPainter;
class QSize;
class KoPictureEpsPrivate;

// TODO: fix documentation

/**
 * @internal
 * KoPictureEps is a container class for a EPS picture
 */
class KoPictureEps : public KoPictureBase
{
public:
    /**
     * Default constructor.
     */
    KoPictureEps();

    /**
     * Destructor.
     */
    virtual ~KoPictureEps();

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

    virtual void clearCache(void);
protected:
    QPixmap getPixmap(QImage& image);
    void scaleAndCreatePixmap(const QSize& size, bool fastMode, const int resolutionx, const int resolutiony );
    QImage scaleWithGhostScript( const QSize& size, const int resolutionx, const int resolutiony );
    bool extractPostScriptStream( void );

private:
    int tryScaleWithGhostScript(QImage &image, const QSize& size, const int resolutionx, const int resolutiony, const char* device );
    /**
     * @brief Read a line from a PostScript stream in a QByteArray
     *
     * The PostScript stream is somewhere in the QByteArray and therefore has to be
     * determined by the start and length of this stream and the current position
     *
     * @param array the array which is read
     * @param start start position of the PostScript stream
     * @param length length of the PostScript stream
     * @param pos current position (which is also returned)
     * @param lastCharWasCr defines if the previous line ended with a Carriage Return,
     *   to skip an eventual Line Feed at the start of this line
     * @return the read line
     */
    QString readLine( const QByteArray& array, const uint start, const uint length, uint& pos, bool& lastCharWasCr );

private:
    /**
     * Copy of the loaded EPS file
     */
    QByteArray m_rawData;
    QPixmap m_cachedPixmap; ///< Cached pixmap
    QSize m_originalSize; ///< Original size of the EPS picture
    QSize m_cachedSize; ///< size of the currently cached pixmap @see m_cachedPixmap
    QRect m_boundingBox; ///< Bounding box, as read from the EPS file
    uint m_psStreamStart; ///< Start position of the PostScript stream (like if it was a MS-DOS EPS file)
    uint m_psStreamLength; ///< Lenght of the PostScript stream (like if it was a MS-DOS EPS file)
    /**
     * true, if the last cached image was done using fast mode.
     * false, if the last cached image was done using slow mode.
     */
    bool m_cacheIsInFastMode;
};

#endif /* __koPictureEps_h__ */
