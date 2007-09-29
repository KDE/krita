/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KO_COLOR_CONVERSION_LINK_H_
#define _KO_COLOR_CONVERSION_LINK_H_

class QString;
class KoColorSpace;
class KoColorConversionTransformation;

#include <KoColorConversionTransformation.h>

#include <pigment_export.h>

/**
 * Factory to create a color transformation between two colorsapces.
 */
class PIGMENT_EXPORT KoColorConversionTransformationFactory {
    public:
        /**
         * Create a color conversion tranformation factory, that can create
         * KoColorConversionTrasnformation object between a source colorspace
         * and a destination colorspace.
         *
         * @param _srcModelId id for the source color model
         * @param _srcDepthId id for the source depth 
         * @param _dstModelId id for the destination color model
         * @param _dstDepthId id for the destination depth
         */
        KoColorConversionTransformationFactory(QString _srcModelId, QString _srcDepthId, QString _dstModelId, QString _dstDepthId);
        virtual ~KoColorConversionTransformationFactory();
        /**
         * Creates a color transformation between the source color space and the destination
         * color space.
         * 
         * @param srcColorSpace source color space
         * @param dstColorSpace destination color space
         */
        virtual KoColorConversionTransformation* createColorTransformation(KoColorSpace* srcColorSpace, KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) =0;
        /**
         * @return true if this factory creates a color conversion transformation which
         * conserve color information (typical color transformation that lose that information
         * is anything to grayscale).
         */
        virtual bool conserveColorInformation() const =0;
        /**
         * @return true if this factory creates a color conversion transformation which
         * conserve the dynamic range of the color.
         */
        virtual bool conserveDynamicRange() const = 0;
        /**
         * @return indicates how much the information on depth is lost when using a color
         * conversion transformation created by this factory (for instance between RGB8bit and R
         * RGB16bit, this function should return 0, while between RGB16bit and RGB8bit the function
         * return 8). If one of the color space has different depths for each of its channel, then
         * this function should return the worse case.
         */
        virtual int depthDecrease() const = 0;
    public:
        /**
         * @return the id of the source color model
         */
        QString srcColorModelId() const;
        /**
         * @return the id of the source color depth
         */
        QString srcColorDepthId() const;
        /**
         * @return the id of the destination color model
         */
        QString dstColorModelId() const;
        /**
         * @return the id of the destination color depth
         */
        QString dstColorDepthId() const;
    protected:
        /**
         * @param srcCS source color space
         * @return true if the color space given as argument can be used as a source colorspace
         */
        bool canBeSource(KoColorSpace* srcCS);
        /**
         * @param dstCS destination color space
         * @return true if the color space given as argument can be used as a destination colorspace
         */
        bool canBeDestination(KoColorSpace* dstCS);
    private:
        struct Private;
        Private* const d;
};

#endif
