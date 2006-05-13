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
#ifndef __koPicture_h__
#define __koPicture_h__

#include <QString>
#include <QIODevice>
#include <QPixmap>
#include <koffice_export.h>

#include "KoPictureKey.h"

class KoXmlWriter;
class QPainter;
class QSize;
class Q3DragObject;
class KUrl;

class KoPictureShared;

/**
 * KoPicture is a container class for various types of pictures supported by %KOffice.
 *
 * @short A picture container class
 */
class KOFFICECORE_EXPORT KoPicture
{
public:
    /**
     * Default constructor.
     */
    KoPicture(void);

    /**
     * Destructor.
     */
    ~KoPicture(void);

    /**
     * Copy constructor
     */
    KoPicture(const KoPicture &other);

    /**
     * Assignment operator
     */
    KoPicture& operator=(const KoPicture& other);

    KoPictureType::Type getType(void) const;

    /**
     * Retrieve the key structure describing the picture in a unique way.
     */
    KoPictureKey getKey(void) const;

    /**
     * Set the key structure describing the picture in a unique way
     */
    void setKey(const KoPictureKey& key);

    /**
     * Returns true if the picture is null.
     */
    bool isNull(void) const;

    /**
     * @brief Draw the picture in a painter.
     *
     * The parameter @p fastMode allows the picture to be re-sized and drawn quicker if possible
     *
     * The parameters @p width, @p height define the desired size for the picture
     *
     * The other parameters are very similar to QPainter::drawPixmap :
     * (@p x, @p y) define the position in the painter,
     * (@p sx, @p sy) specify the top-left point in picture that is to be drawn. The default is (0, 0).
     * (@p sw, @p sh) specify the size of the picture that is to be drawn. The default, (-1, -1), means all the way to the bottom
     * right of the pixmap.
     *
     */
    void draw(QPainter& painter, int x, int y, int width, int height, int sx = 0, int sy = 0,
              int sw = -1, int sh = -1, bool fastMode = false);

    /**
     * Create a dragobject containing this picture.
     * @param dragSource must be 0 when copying to the clipboard
     * @param name name for the QDragObject
     * @return 0L if the picture is null, or if a dragobject for it isn't implemented [yet]
     */
    Q3DragObject* dragObject( QWidget *dragSource = 0L, const char *name = 0L );

    bool load(QIODevice* io, const QString& extension);

    /**
     * Save picture into a QIODevice
     * @param io QIODevice used for saving
     */
    bool save(QIODevice* io) const;

    /**
     * OASIS FlatXML support:
     * Save picture as base64-encoded data into an XML writer.
     * The caller will usually do something like
     * @code
     *  writer.startElement( "office:binary-data" );
     *  m_picture.saveAsBase64( writer );
     *  writer.endElement();
     * @endcode
     */
    bool saveAsBase64( KoXmlWriter& writer ) const;

    /**
     * @return the image extension (e.g. png)
     */
    QString getExtension(void) const;

    /**
     * @return the image MIME type
     */
    QString getMimeType(void) const;

    /**
     * @return the original image size
     */
    QSize getOriginalSize(void) const;

    /**
     * Clear and set the mode of this KoPicture
     *
     * @param newMode a file extension (like "png") giving the wanted mode
     */
    void clearAndSetMode(const QString& newMode);

    /**
     * Reset the KoPicture (but not the key!)
     */
    void clear(void);

    /**
     * Load the picture from a file named @p fileName
     */
    bool loadFromFile(const QString& fileName);

    /**
     * Load the picture from base64-encoded data
     */
    bool loadFromBase64(const QByteArray& str);

    /**
     * Load a potentially broken XPM file (for old files of KPresenter)
     */
    bool loadXpm(QIODevice* io);

    /**
     * @deprecated To be replaced by @ref KoPicture::draw
     *
     * Returns a QPixmap from an image
     * Returns an empty QPixmap if the KoPicture is not an image.
     */
    QPixmap generatePixmap(const QSize& size, bool smoothScale = false);

    /**
     * Download and set the key for a possibly remote file.
     *
     * @param url the url to download from
     * @param window the parent widget for the download. You can pass
     *               NULL (0) if you absolutely cannot find a parent
     *               widget to use.
     */
    bool setKeyAndDownloadPicture(const KUrl& url, QWidget *window);

    /**
     * Generate a QImage
     * (always in slow mode)
     *
     * @param size the wanted size for the QImage
     */
    QImage generateImage(const QSize& size);

    /**
     * @return TRUE if the alpha channel processing has been enabled
     */
    bool hasAlphaBuffer() const;

    /**
     * Respect the image alpha buffer
     */
    void setAlphaBuffer(bool enable);

    /**
     * Creates an alpha mask for the picture
     * (first you have to call @ref #setAlphaBuffer).
     *
     * @see hasAlphaBuffer() setAlphaBuffer()
     */
    QImage createAlphaMask(Qt::ImageConversionFlags flags = Qt::AutoColor) const;

    /**
     * @brief Clear any cache
     *
     * This is used to avoid using too much memory
     * especially if the application somehow also caches the KoPicture's output
     */
    void clearCache(void);

    QString uniquePictureId() const;
    void assignPictureId( uint _id);

protected:
    /**
     * @internal
     * Unregister shared data
     */
    void unlinkSharedData(void);
    /**
     * @internal
     * Register shared data
     */
    void linkSharedData(void) const;
    /**
     * @internal
     * Creare the shared data if needed
     */
    void createSharedData(void);
    QString uniqueName() const;

protected:
    /**
     * @internal
     * The key
     */
    KoPictureKey m_key;
    /**
     * @internal
     * The shared data
     */
    KoPictureShared* m_sharedData;
    static uint uniqueValue;

    QString m_uniqueName;
};

#endif /* __koPicture_h__ */
