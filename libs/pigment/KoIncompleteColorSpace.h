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

#ifndef _KO_INCOMPLETE_COLOR_SPACE_H_
#define _KO_INCOMPLETE_COLOR_SPACE_H_

#include <KoFallBackColorTransformation.h>
#include <KoColorSpaceAbstract.h>

/**
 * Inherit this colorspace if you can't provide all the functions
 * defined in KoColorSpace.
 *
 */
template<class _CSTraits>
class KoIncompleteColorSpace : public KoColorSpaceAbstract<_CSTraits>
{

protected:

    KoIncompleteColorSpace(const QString &id,
                           const QString &name,
                           const KoColorSpace* fallBack)
            : KoColorSpaceAbstract<_CSTraits>(id, name),
            m_fallBackColorSpace( KoColorSpaceRegistry::instance()->grabColorSpace( fallBack) ) {
        m_qcolordata = new quint16[4];
        m_convertionCache.resize(m_fallBackColorSpace->pixelSize());
    }

    virtual ~KoIncompleteColorSpace() {
        delete[] m_qcolordata;
        KoColorSpaceRegistry::instance()->releaseColorSpace(m_fallBackColorSpace);
    }

public:

    virtual bool hasHighDynamicRange() const {
        return false;
    }
    virtual const KoColorProfile * profile() const {
        return 0;
    }
    virtual KoColorProfile * profile() {
        return 0;
    }

    virtual void fromQColor(const QColor& color, quint8 *dst, const KoColorProfile * profile = 0) const {
        Q_UNUSED(profile);
        m_qcolordata[3] = 0xFFFF;
        m_qcolordata[2] = KoColorSpaceMaths<quint8, quint16>::scaleToA(color.red());
        m_qcolordata[1] = KoColorSpaceMaths<quint8, quint16>::scaleToA(color.green());
        m_qcolordata[0] = KoColorSpaceMaths<quint8, quint16>::scaleToA(color.blue());
        this->fromRgbA16((const quint8*)m_qcolordata, dst, 1);
        this->setAlpha(dst, color.alpha(), 1);
    }

    virtual void toQColor(const quint8 *src, QColor *c, const KoColorProfile * profile = 0) const {
        Q_UNUSED(profile);
        this->toRgbA16(src, (quint8*)m_qcolordata, 1);
        c->setRgb(KoColorSpaceMaths<quint16, quint8>::scaleToA(m_qcolordata[2]),
                  KoColorSpaceMaths<quint16, quint8>::scaleToA(m_qcolordata[1]),
                  KoColorSpaceMaths<quint16, quint8>::scaleToA(m_qcolordata[0]));
        c->setAlpha(this->alpha(src));
    }

    virtual KoColorTransformation *createBrightnessContrastAdjustment(const quint16 *transferValues) const {
        return new KoFallBackColorTransformation(this, m_fallBackColorSpace, m_fallBackColorSpace->createBrightnessContrastAdjustment(transferValues));
    }


    virtual KoColorTransformation *createDesaturateAdjustment() const {
        return new KoFallBackColorTransformation(this, m_fallBackColorSpace, m_fallBackColorSpace->createDesaturateAdjustment());
    }

    virtual KoColorTransformation *createPerChannelAdjustment(const quint16 * const* transferValues) const {
        return new KoFallBackColorTransformation(this, m_fallBackColorSpace, m_fallBackColorSpace->createPerChannelAdjustment(transferValues));
    }

    virtual quint8 difference(const quint8* src1U8, const quint8* src2U8) const {
        const typename _CSTraits::channels_type* src1 = _CSTraits::nativeArray(src1U8);
        const typename _CSTraits::channels_type* src2 = _CSTraits::nativeArray(src2U8);
        typename _CSTraits::channels_type count = 0;
        for (uint i = 0; i < this->colorChannelCount(); i++) {
            typename _CSTraits::channels_type d = qAbs(src2[i] - src1[i]);
            if (d > count) {
                count = d;
            }
        }
        return KoColorSpaceMaths<typename _CSTraits::channels_type, quint8>::scaleToA(count);
    }

private:
    mutable quint16 * m_qcolordata; // A small buffer for conversion from and to qcolor.
    KoColorSpace* m_fallBackColorSpace;
    quint32 m_cmType;
    mutable QByteArray m_convertionCache;
};

#endif
