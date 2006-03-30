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
#ifndef __koPictureShared_h__
#define __koPictureShared_h__

#include <q3shared.h>
#include <qstring.h>
#include <qiodevice.h>
#include <qpixmap.h>

#include "KoPictureKey.h"

class KoXmlWriter;
class QPainter;
class QSize;

class KUrl;

class KoPictureBase;

/**
 * @internal
 * KoPictureShared is the class that contains the shared part for KoPicture
 *
 * @warning As with all QShared objects, the sharing is neither automatic nor transparent!
 */
class KoPictureShared : public Q3Shared
{
public:
    /**
     * Default constructor.
     */
    KoPictureShared(void);

    /**
     * Destructor.
     */
    ~KoPictureShared(void);

    /**
     * @brief Copy constructor
     *
     * This makes a deep copy. Do not use if you want to share!
     */
    KoPictureShared(const KoPictureShared &other);

    /**
     * @brief Assignment operator
     *
     * This makes a deep copy. Do not use if you want to share!
     */
    KoPictureShared& operator=(const KoPictureShared& other);

    KoPictureType::Type getType(void) const;

    /**
     * Returns true if the picture is null.
     */
    bool isNull(void) const;

    /**
     * @brief Draw the image in a painter.
     *
     * The parameter @p fastMode allows the picture to be re-sized and drawn quicker if possible
     *
     * The parameters @p width, @p height define the desired size for the picture
     *
     * The other parameters are very similar to QPainter::drawPixmap :
     * (@p x, @p y) define the position in the painter,
     * (@p sx, @p sy) specify the top-left point in pixmap that is to be drawn. The default is (0, 0).
     * (@p sw, @p sh) specify the size of the pixmap that is to be drawn. The default, (-1, -1), means all the way to the bottom
     * right of the pixmap.
     */
    void draw(QPainter& painter, int x, int y, int width, int height, int sx = 0, int sy = 0, int sw = -1, int sh = -1, bool fastMode = false);

    /**
     * Create a dragobject containing this picture.
     * @param dragSource must be 0 when copying to the clipboard
     * @return 0L if the picture is null!
     */
    Q3DragObject* dragObject( QWidget *dragSource = 0L, const char *name = 0L );

    bool load(QIODevice* io, const QString& extension);
    bool loadFromBase64( const QByteArray& str );

    /**
     * Save picture into a QIODevice
     * @param io QIODevice used for saving
     */
    bool save(QIODevice* io) const;

    /**
     * OASIS FlatXML support:
     * Save picture as base64-encoded data into an XML writer.
     */
    bool saveAsBase64( KoXmlWriter& writer ) const;

    void setExtension(const QString& extension);

    QString getExtension(void) const;

    QSize getOriginalSize(void) const;

    /**
     * Clear and set the mode of this KoPictureShared
     *
     * @param newMode file extension (like "png") giving the wanted mode
     */
    void clearAndSetMode(const QString& newMode);

    /**
     * Reset the KoPictureShared (but not the key!)
     */
    void clear(void);

    /**
     * Load a file
     *
     * @param fileName the name of the file to load
     */
    bool loadFromFile(const QString& fileName);

    /**
     * Load a potentially broken XPM file (for KPresenter)
     */
    bool loadXpm(QIODevice* io);

    /**
     * @deprecated
     * Returns a QPixmap from an image
     *
     * @param size the wanted size for the QPixmap
     */
    QPixmap generatePixmap(const QSize& size, bool smoothScale = false);

    QString getMimeType(void) const;

    /**
     * Generate a QImage
     *
     * (always in slow mode)
     *
     * @param size the wanted size for the QImage
     */
    QImage generateImage(const QSize& size);

    bool hasAlphaBuffer() const;
    void setAlphaBuffer(bool enable);
    QImage createAlphaMask(Qt::ImageConversionFlags flags = Qt::AutoColor) const;

    /**
     * Clear any cache
     *
     * It is used to avoid using too much memory
     * especially if the application somehow caches the KoPicture too.
     */
    void clearCache(void);

    QString uniquePictureId() const;
    void assignPictureId( uint _id);

protected:
    /**
     * @internal
     * Load a WMF file
     * \note In %KOffice 1.1, a .wmf file was a QPicture file
     */
    bool loadWmf(QIODevice* io);

    /**
     * @internal
     * Loads a temporary file, probably from a downloaded file
     */
    bool loadTmp(QIODevice* io);

    /// Find type of image, create base accordingly, and load data
    bool identifyAndLoad( QByteArray data );

    /**
     * @internal
     * Loads a compressed file
     *
     * @warning risk of endless recurision, be careful when it is called from @see load
     *
     * @param io QIODevice of the compressed file/strea,
     * @param mimeType mimetype of the (de-)compressor
     * @param extension extension of the uncompressed file
     */
    bool loadCompressed( QIODevice* io, const QString& mimeType, const QString& extension );

protected:
    KoPictureBase* m_base;
    QString m_extension;
    uint m_pictureId;
};

#endif /* __koPictureShared_h__ */
