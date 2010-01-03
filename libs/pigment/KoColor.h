/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef KOCOLOR_H
#define KOCOLOR_H

#include <QtGui/QColor>
#include <QtCore/QMetaType>
#include "pigment_export.h"

class QDomDocument;
class QDomElement;

class KoColorProfile;
class KoColorSpace;

/**
 * A KoColor describes a color in a certain colorspace. The color is stored in a buffer
 * that can be manipulated by the function of the color space.
 */
class PIGMENTCMS_EXPORT KoColor
{

public:
    /// Create an empty KoColor. It will be valid, but also black and transparent
    KoColor();

    ~KoColor();

    /// Create a null KoColor. It will be valid, but all channels will be set to 0
    explicit KoColor(const KoColorSpace * colorSpace);
    /// Create a KoColor from a QColor. The QColor is immediately converted to native. The QColor
    /// is assumed to have the current monitor profile.
    KoColor(const QColor & color, const KoColorSpace * colorSpace);

    /// Create a KoColor using a native color strategy. The data is copied.
    KoColor(const quint8 * data, const KoColorSpace * colorSpace);

    /// Create a KoColor by converting src into another colorspace
    KoColor(const KoColor &src, const KoColorSpace * colorSpace);

    /// Copy constructor -- deep copies the colors.
    KoColor(const KoColor & rhs);

    /**
     * assignment operator to copy the data from the param color into this one.
     * @param other the color we are going to copy
     * @return this color
     */
    KoColor &operator=(const KoColor &other);

    bool operator==(const KoColor &other) const;

    /// return the current colorSpace
    const KoColorSpace * colorSpace() const;

    /// return the current profile
    const KoColorProfile *  profile() const;

    /// Convert this KoColor to the specified colorspace. If the specified colorspace is the
    /// same as the original colorspace, do nothing. Returns the converted KoColor.
    void convertTo(const KoColorSpace * cs);

    /// Replace the existing color data, and colorspace with the specified data.
    void setColor(quint8 * data, const KoColorSpace * colorSpace = 0);

    /// Convert the color from src and replace the value of the current color with the converted data.
    /// Don't convert the color if src and this have the same colorspace.
    void fromKoColor(const KoColor& src);

    /// a convenience method for the above.
    void toQColor(QColor *c) const;
    /// a convenience method for the above.
    QColor toQColor() const;

    /**
     * Convenient function to set the opacity of the color.
     */
    void setOpacity(quint8 alpha);
    /**
     * Convenient function that return the opacity of the color
     */
    quint8 opacity() const;

// what about making the next two methods static factory methods?
    /// Convenient function for converting from a QColor
    void fromQColor(const QColor& c) const;

    /**
     * @return the buffer associated with this color object to be used with the
     *         transformation object created by the color space of this KoColor
     *         or to copy to a different buffer from the same color space
     */
    quint8 * data();

    /**
     * @return the buffer associated with this color object to be used with the
     *         transformation object created by the color space of this KoColor
     *         or to copy to a different buffer from the same color space
     */
    const quint8 * data() const;

    /**
     * Serialize this color following Create's swatch color specification available
     * at http://create.freedesktop.org/wiki/index.php/Swatches_-_colour_file_format
     *
     * This function doesn't create the <color /> element but rather the <CMYK />,
     * <sRGB />, <RGB /> ... elements. It is assumed that colorElt is the <color />
     * element.
     *
     * @param colorElt root element for the serialization, it is assumed that this
     *                 element is <color />
     * @param doc is the document containing colorElt
     */
    void toXML(QDomDocument& doc, QDomElement& colorElt) const;

    /**
     * Unserialize a color following Create's swatch color specification available
     * at http://create.freedesktop.org/wiki/index.php/Swatches_-_colour_file_format
     *
     * @param elt the element to unserialize (<CMYK />, <sRGB />, <RGB />)
     * @param bitDepthId the bit depth is unspecified by the spec, this allow to select
     *                   a preferred bit depth for creating the KoColor object (if that
     *                   bit depth isn't available, this function will randomly select
     *                   an other bit depth)
     * @param profileAliases alias between the profile name specified by the "space"
     *                       attribute and the profile name used inside pigment
     * @return the unserialize color, or an empty color object if the function failed
     *         to unserialize the color
     */
    static KoColor fromXML(const QDomElement& elt, const QString & bitDepthId, const QHash<QString, QString> & aliases);

#ifndef NODEBUG
    /// use kDebug calls to print internal info
    void dump() const;
#endif

private:
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(KoColor)

#endif
