/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KOCOMPOSITEOP_H
#define KOCOMPOSITEOP_H

#include <QtCore>
#include <klocale.h>

#include "pigment_export.h"

class KoColorSpace;

const QString COMPOSITE_OVER = "normal";
const QString COMPOSITE_ERASE = "erase";
const QString COMPOSITE_COPY = "copy";
const QString COMPOSITE_ALPHA_DARKEN = "alphadarken";
const QString COMPOSITE_IN = "in";
const QString COMPOSITE_OUT = "out";
const QString COMPOSITE_ATOP = "atop";
const QString COMPOSITE_XOR = "xor";
const QString COMPOSITE_PLUS = "plus";
const QString COMPOSITE_MINUS = "minus";
const QString COMPOSITE_ADD = "add";
const QString COMPOSITE_SUBTRACT = "subtract";
const QString COMPOSITE_INVERSED_SUBTRACT = "inversed_subtract";
const QString COMPOSITE_DIFF = "diff";
const QString COMPOSITE_MULT = "multiply";
const QString COMPOSITE_DIVIDE = "divide";
const QString COMPOSITE_DODGE = "dodge";
const QString COMPOSITE_BURN = "burn";
const QString COMPOSITE_BUMPMAP = "bumpmap";
const QString COMPOSITE_CLEAR = "clear";
const QString COMPOSITE_DISSOLVE = "dissolve";
const QString COMPOSITE_DISPLACE = "displace";
const QString COMPOSITE_NO = "nocomposition";
const QString COMPOSITE_DARKEN = "darken";
const QString COMPOSITE_LIGHTEN = "lighten";
const QString COMPOSITE_HUE = "hue";
const QString COMPOSITE_SATURATION = "saturation";
const QString COMPOSITE_VALUE = "value";
const QString COMPOSITE_COLOR = "color";
const QString COMPOSITE_COLORIZE = "colorize";
const QString COMPOSITE_LUMINIZE = "luminize";
const QString COMPOSITE_SCREEN = "screen";
const QString COMPOSITE_OVERLAY = "overlay";
const QString COMPOSITE_COPY_RED = "copy_red";
const QString COMPOSITE_COPY_GREEN = "copy_green";
const QString COMPOSITE_COPY_BLUE = "copy_blue";
const QString COMPOSITE_COPY_OPACITY = "copy_opacity";
const QString COMPOSITE_HARD_LIGHT = "hard_light";
const QString COMPOSITE_SOFT_LIGHT = "soft_light";
const QString COMPOSITE_EXCLUSION = "exclusion"; // XXX: not implemented anywhere yet
const QString COMPOSITE_INVERTED_DIVIDE = "inverted_divide"; // XXX: not implemented anywhere yet
const QString COMPOSITE_VIVID_LIGHT = "vivid light"; // XXX: not implemented anywhere yet
const QString COMPOSITE_LINEAR_LIGHT = "linear light"; // XXX: not implemented anywhere yet
const QString COMPOSITE_PIN_LIGHT = "pin light"; // XXX: not implemented anywhere yet
const QString COMPOSITE_HARD_MIX = "hard mix"; // XXX: not implemented anywhere yet
const QString COMPOSITE_PASS_THROUGH = "pass through"; // XXX: not implemented anywhere yet

const QString COMPOSITE_UNDEF = "underfined";

/**
 * Base for colorspace-specific blending modes.
 */
class PIGMENTCMS_EXPORT KoCompositeOp
{
public:
    static QString categoryMix();
    static QString categoryLight();
    static QString categoryArithmetic();
    static QString categoryColor();
    static QString categoryMisc();
public:

    /**
     * @param cs a pointer to the color space that can be used with this composite op
     * @param id the identifier for this composite op (not user visible)
     * @param description an user visible string describing this composite operation
     * @param category the name of the category where to put that composite op when displayed
     * @param userVisible define whether or not that composite op should be visible in an user
     *                    interface
     */
    KoCompositeOp(const KoColorSpace * cs, const QString& id, const QString& description, const QString & categoryMisc, const bool userVisible = true);
    virtual ~KoCompositeOp();

    /**
     * @return the identifier of this composite op
     */
    QString id() const;
    /**
     * @return the user visible string for this composite op
     */
    QString description() const;
    /**
     * @return the color space that can use and own this composite op
     */
    const KoColorSpace * colorSpace() const;
    /**
     * @return whether this composite op should be visible in the user interface
     */
    bool userVisible() const;
    /**
     * @return the category associated with the composite op
     */
    QString category() const;

    /**
      @param dstRowStart pointer to the start of the byte array we will composite the source on
      @param dstRowStride length of the rows of the block of destination pixels in bytes
      @param srcRowStart pointer to the start of the byte array we will mix with dest
      @param srcRowStride length of the rows of the block of src in bytes
       pixels (may be different from the rowstride of the dst pixels,
       in which case the smaller value is used). If srcRowStride is null
       it is assumed that the source is a constant color.
      @param maskRowStart start of the byte mask that determines whether and if so, then how much of src is used for blending
      @param maskRowStride length of the mask scanlines in bytes
      @param rows number of scanlines to blend
      @param numColumns length of the row of pixels in pixels
      @param opacity transparency with which to blend
    */
    virtual void composite(quint8 *dstRowStart, qint32 dstRowStride,
                           const quint8 *srcRowStart, qint32 srcRowStride,
                           const quint8 *maskRowStart, qint32 maskRowStride,
                           qint32 rows, qint32 numColumns,
                           quint8 opacity) const;

    /**
     * Same as previous, but with a bit array that determines which channels
     * (channels are in the order of the channels in the colorspace) should
     * be excluded (false) or included (true) in the bitBlt.
     *
     * For instance:
     * if this is COMPOSITY_COPY
     * composite(..., [1, 0]) for grayscale is the same as old COMPOSITE_COPY_GRAY --
     * only the gray channel is copied.
     */
    virtual void composite(quint8 *dstRowStart, qint32 dstRowStride,
                           const quint8 *srcRowStart, qint32 srcRowStride,
                           const quint8 *maskRowStart, qint32 maskRowStride,
                           qint32 rows, qint32 numColumns,
                           quint8 opacity,
                           const QBitArray & channelFlags) const = 0;
private:
    KoCompositeOp();
    struct Private;
    Private* const d;
};

#endif // KOCOMPOSITEOP_H
