#ifndef LCMSRGBP2020PQCOLORSPACE_H
#define LCMSRGBP2020PQCOLORSPACE_H

#include <colorspaces/rgb_u8/RgbU8ColorSpace.h>
#include <colorspaces/rgb_u16/RgbU16ColorSpace.h>
#include <colorspaces/rgb_f16/RgbF16ColorSpace.h>
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

template<>
struct ColorSpaceFromFactory<RgbF16ColorSpaceFactory> {
  typedef RgbF16ColorSpace type;
};

template<>
struct ColorSpaceFromFactory<RgbF32ColorSpaceFactory> {
  typedef RgbF32ColorSpace type;
};


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

        list << new LcmsFromRGBP2020PQTransformationFactory<RelatedColorSpaceType>();
        list << new LcmsToRGBP2020PQTransformationFactory<RelatedColorSpaceType>();

        return list;
    }
};

#endif // LCMSRGBP2020PQCOLORSPACE_H
