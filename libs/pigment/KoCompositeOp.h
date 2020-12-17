/*
 * SPDX-FileCopyrightText: 2005 Adrian Page <adrian@pagenet.plus.com>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef KOCOMPOSITEOP_H
#define KOCOMPOSITEOP_H

#include <QString>
#include <QList>
#include <QMultiMap>
#include <QBitArray>

#include <boost/optional.hpp>

#include "kritapigment_export.h"

class KoColorSpace;

class KoColorSpace;

/**
 * Base for colorspace-specific blending modes.
 */
class KRITAPIGMENT_EXPORT KoCompositeOp
{
public:
    static QString categoryColor();

    static QString categoryArithmetic();
    static QString categoryBinary();
    static QString categoryModulo();
    static QString categoryNegative();
    static QString categoryLight();
    static QString categoryDark();
    static QString categoryHSY();
    static QString categoryHSI();
    static QString categoryHSL();
    static QString categoryHSV();
    static QString categoryMix();
    static QString categoryMisc();    
    static QString categoryQuadratic();

    struct KRITAPIGMENT_EXPORT ParameterInfo
    {
        ParameterInfo();
        ParameterInfo(const ParameterInfo &rhs);
        ParameterInfo& operator=(const ParameterInfo &rhs);

        quint8*       dstRowStart {0};
        qint32        dstRowStride {0};
        const quint8* srcRowStart {0};
        qint32        srcRowStride {0};
        const quint8* maskRowStart {0};
        qint32        maskRowStride {0};
        qint32        rows {0};
        qint32        cols {0};
        float         opacity {0.0};
        float         flow {0.0};
        float         _lastOpacityData {0.0};
        float*        lastOpacity {0};
        QBitArray     channelFlags;

        void setOpacityAndAverage(float _opacity, float _averageOpacity);

        void updateOpacityAndAverage(float value);
    private:
        inline void copy(const ParameterInfo &rhs);
    };

public:

    /**
     * @param cs a pointer to the color space that can be used with this composite op
     * @param id the identifier for this composite op (not user visible)
     * @param description a user visible string describing this composite operation
     * @param category the name of the category where to put that composite op when displayed
     * @param userVisible define whether or not that composite op should be visible in a user
     *                    interface
     */
    KoCompositeOp(const KoColorSpace * cs, const QString& id, const QString& description, const QString & category = KoCompositeOp::categoryMisc());
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
     * @return the category associated with the composite op
     */
    QString category() const;

    // WARNING: A derived class needs to overwrite at least one
    //          of the following virtual methods or a call to
    //          composite(...) will lead to an endless recursion/stack overflow

    /**
     * @param dstRowStart pointer to the start of the byte array we will composite the source on
     * @param dstRowStride length of the rows of the block of destination pixels in bytes
     * @param srcRowStart pointer to the start of the byte array we will mix with dest
     * @param srcRowStride length of the rows of the block of src in bytes
     * pixels (may be different from the rowstride of the dst pixels,
     * in which case the smaller value is used). If srcRowStride is null
     * it is assumed that the source is a constant color.
     * @param maskRowStart start of the byte mask that determines whether and if so, then how much of src is used for blending
     * @param maskRowStride length of the mask scanlines in bytes
     * @param rows number of scanlines to blend
     * @param numColumns length of the row of pixels in pixels
     * @param opacity transparency with which to blend
     * @param channelFlags a bit array that determines which channels should be processed (channels are in the order of the channels in the colorspace)
     */
    virtual void composite(quint8 *dstRowStart, qint32 dstRowStride,
                            const quint8 *srcRowStart, qint32 srcRowStride,
                            const quint8 *maskRowStart, qint32 maskRowStride,
                            qint32 rows, qint32 numColumns,
                            quint8 opacity, const QBitArray& channelFlags=QBitArray()) const;

    /**
    * Same as previous, but uses a parameter structure
    */
    virtual void composite(const ParameterInfo& params) const;

private:
    KoCompositeOp();
    struct Private;
    Private* const d;
};

#endif // KOCOMPOSITEOP_H
