/*
 *  SPDX-FileCopyrightText: 2018 Iván Santa María <ghevan@gmail.com>
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KO_INVERT_COLOR_TRANSFORMATION_H
#define KO_INVERT_COLOR_TRANSFORMATION_H

#include "KoColorTransformation.h"

#include "KoColorSpace.h"
#include "KoColorSpaceMaths.h"

#include "KoColorModelStandardIds.h"

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif
class KoInvertColorTransformationT : public KoColorTransformation {

public:

    KoInvertColorTransformationT(const KoColorSpace* cs)
        : m_colorSpace(cs)
        , m_psize(cs->pixelSize())
        , m_chanCount(cs->channelCount())
    {
        // Only invert COLOR channels
        QList<KoChannelInfo *> channels = cs->channels();
        for(quint8 i = 0; i < m_chanCount; i++){
            if(channels.at(i)->channelType() == KoChannelInfo::COLOR)
                m_channels.append(i);
        }
    }

    template<typename T>
    void transformI(const quint8 *src, quint8 *dst, qint32 nPixels) const {
        T *m_rgba = (T*)(src);
        T *m_dst = (T*)(dst);

        while (nPixels--) {
            for(quint8 i : m_channels){
                m_dst[i] = KoColorSpaceMaths<T>::invert(m_rgba[i]);
            }
            m_rgba += m_chanCount;
            m_dst += m_chanCount;
        }

    }

    void transformGen(const quint8 *src, quint8 *dst, qint32 nPixels) const {
        quint16 m_rgba[4];
        while (nPixels--) {
            m_colorSpace->toRgbA16(src, reinterpret_cast<quint8 *>(m_rgba), 1);
            m_rgba[0] = KoColorSpaceMathsTraits<quint16>::max - m_rgba[0];
            m_rgba[1] = KoColorSpaceMathsTraits<quint16>::max - m_rgba[1];
            m_rgba[2] = KoColorSpaceMathsTraits<quint16>::max - m_rgba[2];
            m_colorSpace->fromRgbA16(reinterpret_cast<quint8 *>(m_rgba), dst, 1);
            src += m_psize;
            dst += m_psize;
        }
    }

    // Once CMYK and LAB 32 float are normalized, this inverts will invert properly
//    template<typename T>
//    void transformC(const quint8 *src, quint8 *dst, qint32 nPixels) const {
//        QVector<float> normChan(m_chanCount);

//        float *m_rgba;
//        float *m_dst = (float*)(dst);
//        while (nPixels--) {
//            m_colorSpace->normalisedChannelsValue(src, normChan);
//            for(quint8 i : m_channels){
//                normChan[i] = KoColorSpaceMaths<float>::invert(normChan[i]);
//            }
//            m_colorSpace->fromNormalisedChannelsValue(dst,normChan);
//            //m_rgba += m_psize;
//            src += m_psize;
//            dst += m_psize;
//        }
//    }

protected:
    QList<quint8> m_channels;
private:
    const KoColorSpace* m_colorSpace;
    quint32 m_psize;
    quint32 m_chanCount;
};

class KoU8InvertColorTransformer : public KoInvertColorTransformationT {
public:
    KoU8InvertColorTransformer(const KoColorSpace* cs)
       : KoInvertColorTransformationT(cs)
    {
    };

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override {
        transformI<quint8>(src,dst,nPixels);
    }
};

class KoU16InvertColorTransformer : public KoInvertColorTransformationT {
public:
    KoU16InvertColorTransformer(const KoColorSpace* cs)
       : KoInvertColorTransformationT(cs)
    {
    };

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override {
        transformI<quint16>(src,dst,nPixels);
    }
};

#ifdef HAVE_OPENEXR
class KoF16InvertColorTransformer : public KoInvertColorTransformationT {
public:
    KoF16InvertColorTransformer(const KoColorSpace* cs)
       : KoInvertColorTransformationT(cs)
    {
    };

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override {
        transformI<half>(src,dst,nPixels);
    }
};
#endif

class KoF32InvertColorTransformer : public KoInvertColorTransformationT {
public:
    KoF32InvertColorTransformer(const KoColorSpace* cs)
       : KoInvertColorTransformationT(cs)
    {
    };

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override {
        transformI<float>(src,dst,nPixels);
    }
};

class KoF32GenInvertColorTransformer : public KoInvertColorTransformationT {
public:
    KoF32GenInvertColorTransformer(const KoColorSpace* cs)
       : KoInvertColorTransformationT(cs)
    {
    };

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override {
        transformGen(src,dst,nPixels);
    }
};


class KoInvertColorTransformation
{
public:
  static KoColorTransformation* getTransformator(const KoColorSpace *cs)
  {
      KoID id = cs->colorDepthId();
      KoID modelId = cs->colorModelId();
      if (id == Integer8BitsColorDepthID) {
          return new KoU8InvertColorTransformer(cs);
      } else if (id == Integer16BitsColorDepthID) {
          return new KoU16InvertColorTransformer(cs);
#ifdef HAVE_OPENEXR
      } else if (id == Float16BitsColorDepthID) {
          return new KoF16InvertColorTransformer(cs);
#endif
      } else {
          if(modelId == LABAColorModelID || modelId == CMYKAColorModelID){
              return new KoF32GenInvertColorTransformer(cs);
          }
          return new KoF32InvertColorTransformer(cs);
      }
  }
};

#endif
