/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef LCMSRGBP2020PQCOLORSPACE_H
#define LCMSRGBP2020PQCOLORSPACE_H


#include <colorspaces/rgb_u8/RgbU8ColorSpace.h>
#include <colorspaces/rgb_u16/RgbU16ColorSpace.h>

#ifdef HAVE_OPENEXR
#include <colorspaces/rgb_f16/RgbF16ColorSpace.h>
#endif

#include <colorspaces/rgb_f32/RgbF32ColorSpace.h>

#include "KoColorConversionTransformationFactory.h"

#include <LcmsRGBP2020PQColorSpaceTransformation.h>

template <class T>
struct ColorSpaceFromFactory {
};

template<>
struct ColorSpaceFromFactory<RgbU8ColorSpaceFactory> {
  typedef RgbU8ColorSpace type;
};

template<>
struct ColorSpaceFromFactory<RgbU16ColorSpaceFactory> {
  typedef RgbU16ColorSpace type;
};

#ifdef HAVE_OPENEXR
template<>
struct ColorSpaceFromFactory<RgbF16ColorSpaceFactory> {
  typedef RgbF16ColorSpace type;
};
#endif

template<>
struct ColorSpaceFromFactory<RgbF32ColorSpaceFactory> {
  typedef RgbF32ColorSpace type;
};

/**
 *  Define a singly linked list of supported bit depth traits
 */
template<class T> struct NextTrait { using type = void; };
template<> struct NextTrait<KoBgrU8Traits> { using type = KoBgrU16Traits; };

#ifdef HAVE_OPENEXR
template<> struct NextTrait<KoBgrU16Traits> { using type = KoRgbF16Traits; };
template<> struct NextTrait<KoRgbF16Traits> { using type = KoRgbF32Traits; };
#else
template<> struct NextTrait<KoBgrU16Traits> { using type = KoRgbF32Traits; };
#endif

/**
 * Recursively add bit-depths conversions to the color space. We add only
 * **outgoing** conversions for every RGB color space. That is, every color
 * space has exactly three outgoing edges for color conversion.
 */
template<typename ParentColorSpace, typename CurrentTraits>
void addInternalConversion(QList<KoColorConversionTransformationFactory*> &list, CurrentTraits*)
{
    // general case: add a converter and recurse for the next traits
    list << new LcmsScaleRGBP2020PQTransformationFactory<ParentColorSpace, CurrentTraits>();

    using NextTraits = typename NextTrait<CurrentTraits>::type;
    addInternalConversion<ParentColorSpace>(list, static_cast<NextTraits*>(0));
}

template<typename ParentColorSpace>
void addInternalConversion(QList<KoColorConversionTransformationFactory*> &list, typename ParentColorSpace::ColorSpaceTraits*)
{
    // exception: skip adding an edge to the same bit depth

    using CurrentTraits = typename ParentColorSpace::ColorSpaceTraits;
    using NextTraits = typename NextTrait<CurrentTraits>::type;
    addInternalConversion<ParentColorSpace>(list, static_cast<NextTraits*>(0));
}

template<typename ParentColorSpace>
void addInternalConversion(QList<KoColorConversionTransformationFactory*> &, void*)
{
    // stop recursion
}

template <class BaseColorSpaceFactory>
class LcmsRGBP2020PQColorSpaceFactoryWrapper : public BaseColorSpaceFactory
{
    typedef typename ColorSpaceFromFactory<BaseColorSpaceFactory>::type RelatedColorSpaceType;

    KoColorSpace *createColorSpace(const KoColorProfile *p) const override
    {
        return new RelatedColorSpaceType(this->name(), p->clone());
    }

    QList<KoColorConversionTransformationFactory *> colorConversionLinks() const override
    {
        QList<KoColorConversionTransformationFactory *> list;

        /**
         * We explicitly disable direct conversions to/from integer color spaces, because
         * they may cause the the conversion system to choose them as an intermediate
         * color space for the conversion chain, e.g.
         * p709-g10 F32 -> p2020-g10 U16 -> Rec2020-pq U16, which is incorrect and loses
         * all the HDR data
         */
        list << new LcmsFromRGBP2020PQTransformationFactory<RelatedColorSpaceType, KoRgbF32Traits>();
        list << new LcmsToRGBP2020PQTransformationFactory<RelatedColorSpaceType, KoRgbF32Traits>();
#ifdef HAVE_OPENEXR
        list << new LcmsFromRGBP2020PQTransformationFactory<RelatedColorSpaceType, KoRgbF16Traits>();
        list << new LcmsToRGBP2020PQTransformationFactory<RelatedColorSpaceType, KoRgbF16Traits>();
#endif


        // internally, we can convert to RGB U8 if needed
        addInternalConversion<RelatedColorSpaceType>(list, static_cast<KoBgrU8Traits*>(0));

        return list;
    }
};

#endif // LCMSRGBP2020PQCOLORSPACE_H
