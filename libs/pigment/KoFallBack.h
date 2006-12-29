/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KO_FALLBACK_H_
#define _KO_FALLBACK_H_

#include <KoColorTransformation.h>
#include <KoColorSpace.h>

class KoRGB16FallbackColorTransformation : public KoColorTransformation {
  public:
    KoRGB16FallbackColorTransformation(const KoColorSpace* cs, const KoColorSpace* fallBackCS, KoColorTransformation* transfo)
      : m_buff(0), m_buffSize(0), m_colorSpace(cs), m_fallBackColorSpace(fallBackCS) , m_colorTransformation(transfo)
    {
    }
    virtual ~KoRGB16FallbackColorTransformation()
    {
      if(m_buff) delete[] m_buff;
      delete m_colorTransformation;
    }
  public:
    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
    {
      if( m_buffSize < nPixels)
      { // Expand the buffer if needed
        m_buffSize = nPixels;
        if(m_buff) delete[] m_buff;
        m_buff = new quint8[ m_buffSize * m_fallBackColorSpace->pixelSize() ];
      }
      m_colorSpace->toRgbA16(src, m_buff, nPixels);
      m_colorTransformation->transform(m_buff, m_buff, nPixels);
      m_colorSpace->fromRgbA16(m_buff, dst, nPixels);
    }
  private:
    mutable quint8* m_buff;
    mutable qint32 m_buffSize;
    const KoColorSpace* m_colorSpace;
    const KoColorSpace* m_fallBackColorSpace;
    KoColorTransformation* m_colorTransformation;
};


class KoRGB16Fallback {
  public:
    static inline void toLabA16(const KoColorSpace* cs, const KoColorSpace* fallBackCS, const quint8 * src, quint8 * dst, QByteArray& buf, const quint32 nPixels)
    {
        int lenght = nPixels * fallBackCS->pixelSize();
        if(lenght > buf.size())
        {
            buf.resize(lenght);
        }
        cs->toRgbA16( src, (quint8*)buf.data(), nPixels);
        fallBackCS->toLabA16( (quint8*)buf.data(), dst, nPixels);
    }

    static inline void fromLabA16(const KoColorSpace* cs, const KoColorSpace* fallBackCS, const quint8 * src, quint8 * dst, QByteArray& buf, const quint32 nPixels)
    {
        int lenght = nPixels * fallBackCS->pixelSize();
        if(lenght > buf.size())
        {
            buf.resize(lenght);
        }
        fallBackCS->toLabA16(src, (quint8*)buf.data(), nPixels);
        cs->toRgbA16( (quint8*)buf.data(), dst, nPixels);
    }
    static inline void fromRgbA16(const KoColorSpace* cs, const KoColorSpace* fallBackCS, const quint8 * src, quint8 * dst, QByteArray& buf, const quint32 nPixels)
    {
        Q_UNUSED(cs);
        Q_UNUSED(fallBackCS);
        Q_UNUSED(src);
        Q_UNUSED(dst);
        Q_UNUSED(buf);
        Q_UNUSED(nPixels);
        kError() << "THIS FUNCTION SHOULDN'T BE EXECUTED YOU NEED TO REIMPLEMENT fromRgbA16 IN YOUR COLORSPACE"  << endl;
    }
    static inline  void toRgbA16(const KoColorSpace* cs, const KoColorSpace* fallBackCS, const quint8 * src, quint8 * dst, QByteArray& buf, const quint32 nPixels)
    {
        Q_UNUSED(cs);
        Q_UNUSED(fallBackCS);
        Q_UNUSED(src);
        Q_UNUSED(dst);
        Q_UNUSED(buf);
        Q_UNUSED(nPixels);
        kError() << "THIS FUNCTION SHOULDN'T BE CALLED YOU NEED TO REIMPLEMENT toRgbA16 IN YOUR COLORSPACE"  << endl;
    }
    static inline KoColorSpace* createColorSpace()
    {
      return KoColorSpaceRegistry::instance()->rgb16();
    }
    static inline KoColorTransformation* createTransformation(const KoColorSpace* cs, const KoColorSpace* fallBackCS, KoColorTransformation* c)
    {
      return new KoRGB16FallbackColorTransformation(cs, fallBackCS, c);
    }
};

#endif
