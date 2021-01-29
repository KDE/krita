/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KOCOLOR_H
#define KOCOLOR_H

#include <QColor>
#include <QMetaType>
#include <QtGlobal>
#include "kritapigment_export.h"
#include "KoColorConversionTransformation.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorSpaceTraits.h"
#include <boost/operators.hpp>


class QDomDocument;
class QDomElement;

class KoColorProfile;
class KoColorSpace;

/**
 * A KoColor describes a color in a certain colorspace. The color is stored in a buffer
 * that can be manipulated by the function of the color space.
 */
class KRITAPIGMENT_EXPORT KoColor : public boost::equality_comparable<KoColor>
{

public:
    /// Create an empty KoColor. It will be valid, but also black and transparent
    KoColor();

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
    KoColor(const KoColor & rhs) {
        *this = rhs;
    }

    /**
     * assignment operator to copy the data from the param color into this one.
     * @param rhs the color we are going to copy
     * @return this color
     */
    inline KoColor &operator=(const KoColor &rhs) {
        if (&rhs == this) {
            return *this;
        }

        m_colorSpace = rhs.m_colorSpace;
        m_size = rhs.m_size;
        memcpy(m_data, rhs.m_data, m_size);

        assertPermanentColorspace();

        return *this;
    }

    bool operator==(const KoColor &other) const;

    /// return the current colorSpace
    const KoColorSpace * colorSpace() const {
        return m_colorSpace;
    }

    /// return the current profile
    const KoColorProfile *profile() const;

    /// Convert this KoColor to the specified colorspace. If the specified colorspace is the
    /// same as the original colorspace, do nothing
    void convertTo(const KoColorSpace * cs,
                   KoColorConversionTransformation::Intent renderingIntent,
                   KoColorConversionTransformation::ConversionFlags conversionFlags);

    void convertTo(const KoColorSpace * cs);

    /// Copies this color and converts it to the specified colorspace. If the specified colorspace is the
    /// same as the original colorspace, just returns a copy
    KoColor convertedTo(const KoColorSpace * cs,
                        KoColorConversionTransformation::Intent renderingIntent,
                        KoColorConversionTransformation::ConversionFlags conversionFlags) const;

    /// Copies this color and converts it to the specified colorspace. If the specified colorspace is the
    /// same as the original colorspace, just returns a copy
    KoColor convertedTo(const KoColorSpace * cs) const;



    /// assign new profile without converting pixel data
    void setProfile(const KoColorProfile *profile);

    /// Replace the existing color data, and colorspace with the specified data.
    /// The data is copied.
    void setColor(const quint8 * data, const KoColorSpace * colorSpace = 0);

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
    void setOpacity(qreal alpha);
    /**
     * Convenient function that return the opacity of the color
     */
    quint8 opacityU8() const;
    qreal opacityF() const;

    /// Convenient function for converting from a QColor
    void fromQColor(const QColor& c);

    /**
     * @return the buffer associated with this color object to be used with the
     *         transformation object created by the color space of this KoColor
     *         or to copy to a different buffer from the same color space
     */
    quint8 * data() {
        return m_data;
    }

    /**
     * @return the buffer associated with this color object to be used with the
     *         transformation object created by the color space of this KoColor
     *         or to copy to a different buffer from the same color space
     */
    const quint8 * data() const {
        return m_data;
    }


    /**
     * Channelwise subtracts \p value from *this and stores the result in *this
     *
     * Throws a safe assert if the colorspaces of the two colors are different
     */
    void subtract(const KoColor &value);

    /**
     * Channelwise subtracts \p value from a copy of *this and returns the result
     *
     * Throws a safe assert if the colorspaces of the two colors are different
     */
    KoColor subtracted(const KoColor &value) const;

    /**
     * Channelwise adds \p value to *this and stores the result in *this
     *
     * Throws a safe assert if the colorspaces of the two colors are different
     */
    void add(const KoColor &value);

    /**
     * Channelwise adds \p value to a copy of *this and returns the result
     *
     * Throws a safe assert if the colorspaces of the two colors are different
     */
    KoColor added(const KoColor &value) const;

    /**
     * Serialize this color following Create's swatch color specification available
     * at https://web.archive.org/web/20110826002520/http://create.freedesktop.org/wiki/Swatches_-_colour_file_format/Draft
     *
     * This function doesn't create the \<color /\> element but rather the \<CMYK /\>,
     * \<sRGB /\>, \<RGB /\> ... elements. It is assumed that colorElt is the \<color /\>
     * element.
     *
     * @param colorElt root element for the serialization, it is assumed that this
     *                 element is \<color /\>
     * @param doc is the document containing colorElt
     */
    void toXML(QDomDocument& doc, QDomElement& colorElt) const;

    /**
     * Unserialize a color following Create's swatch color specification available
     * at https://web.archive.org/web/20110826002520/http://create.freedesktop.org/wiki/Swatches_-_colour_file_format/Draft
     *
     * @param elt the element to unserialize (\<CMYK /\>, \<sRGB /\>, \<RGB /\>)
     * @param channelDepthId the bit depth is unspecified by the spec, this allow to select
     *                   a preferred bit depth for creating the KoColor object (if that
     *                   bit depth isn't available, this function will randomly select
     *                   an other bit depth)
     * @return the unserialize color, or an empty color object if the function failed
     *         to unserialize the color
     */
    static KoColor fromXML(const QDomElement& elt, const QString & channelDepthId);

    /**
     * Unserialize a color following Create's swatch color specification available
     * at https://web.archive.org/web/20110826002520/http://create.freedesktop.org/wiki/Swatches_-_colour_file_format/Draft
     *
     * @param elt the element to unserialize (\<CMYK /\>, \<sRGB /\>, \<RGB /\>)
     * @param channelDepthId the bit depth is unspecified by the spec, this allow to select
     *                   a preferred bit depth for creating the KoColor object (if that
     *                   bit depth isn't available, this function will randomly select
     *                   an other bit depth)
     * @param ok If a an error occurs, *ok is set to false; otherwise it's set to true
     * @return the unserialize color, or an empty color object if the function failed
     *         to unserialize the color
     */
    static KoColor fromXML(const QDomElement& elt, const QString & channelDepthId, bool* ok);


    /**
     * @brief toXML creates a string with XML that represents the current color. The XML
     * is extended with a "channeldepth" attribute so we can restore the color to the same
     * channel depth.
     * @return a valid XML document in a string
     */
    QString toXML() const;

    /**
     * @brief fromXML restores a KoColor from a string saved with toXML(). If the
     * string does not contain the "channeldepth" attribute, 16 bit integer is assumed.
     * @param xml a valid XML document
     * @return a new KoColor object
     */
    static KoColor fromXML(const QString &xml);

    /**
     * @brief toSVG11
     * @param csList list of colorspaces, this will add the colorspace name, so it may be embedded.
     * @return a color definiton string with both a srgb hexcode fallback as well as a icc-color definition.
     */
    QString toSVG11(QMap<QString, const KoColorSpace *> *csList);

    /**
     * @brief fromSVG11
     * Parses a color attribute value and returns a KoColor. SVG defines the colorprofiles elsewhere
     * in the file, so this function expects you to first figure out the profiles and which colorspaces
     * these match to, and it will then use those colorspaces to generate the kocolor. If it cannot find
     * the appropriate colorspace, it will return the color fallback. If that doesn't work, an empty KoColor.
     * This function ignores url() values;
     *
     * https://www.w3.org/TR/SVG11/types.html#DataTypeColor for hex, rgb() and colornames
     * https://www.w3.org/TR/SVG11/types.html#DataTypeICCColor for icc-color()
     * https://www.w3.org/TR/SVG11/painting.html#SpecifyingPaint
     *
     * @param value the content of the svg color value
     * @param csList list of KoColorSpaces that were found inside the svg file, with the reference names.
     * @param current the current color. If
     * @return a KoColor as parsed from the value string.
     */
    static KoColor fromSVG11(const QString value, QMap<QString, const KoColorSpace*> csList, KoColor current = KoColor());

    /**
     * @brief toQString create a user-visible string of the channel names and the channel values
     * @param color the color to create the string from
     * @return a string that can be used to display the values of this color to the user.
     */
    static QString toQString(const KoColor &color);

#ifndef NODEBUG
    /// use qDebug calls to print internal info
    void dump() const;
#endif

private:
    inline void assertPermanentColorspace() {
#ifndef NODEBUG
        if (m_colorSpace) {
            Q_ASSERT(*m_colorSpace == *KoColorSpaceRegistry::instance()->permanentColorspace(m_colorSpace));
        }
#endif
    }

    const KoColorSpace *m_colorSpace;
    quint8 m_data[MAX_PIXEL_SIZE];
    quint8 m_size;
};

Q_DECLARE_METATYPE(KoColor)

KRITAPIGMENT_EXPORT QDebug operator<<(QDebug dbg, const KoColor &color);


#endif
