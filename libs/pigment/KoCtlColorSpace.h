/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KO_CTL_COLOR_SPACE_H_
#define _KO_CTL_COLOR_SPACE_H_

#include <KoColorSpace.h>
#include "pigment_export.h"

class KoCtlColorProfile;

class PIGMENTCMS_EXPORT KoCtlColorSpace : public KoColorSpace {
    public:
        /**
         * This class is use when creating color space that are defined using the Color Transformation Language.
         */
        KoCtlColorSpace(const QString &id, const QString &name, KoMixColorsOp* mixColorsOp, const KoColorSpace* fallBack, const KoCtlColorProfile* profile);
        ~KoCtlColorSpace();
        virtual bool profileIsCompatible(const KoColorProfile* profile) const;
        virtual bool hasHighDynamicRange() const;
        virtual const KoColorProfile * profile() const;
        virtual KoColorProfile * profile();
        virtual KoColorTransformation *createBrightnessContrastAdjustment(const quint16 *transferValues) const;
        virtual KoColorTransformation *createDesaturateAdjustment() const;
        virtual KoColorTransformation *createPerChannelAdjustment(const quint16 * const* transferValues) const;
        virtual KoColorTransformation *createDarkenAdjustment(qint32 shade, bool compensate, double compensation) const;
        virtual KoColorTransformation *createInvertTransformation() const;
        virtual quint8 difference(const quint8* src1, const quint8* src2) const;
        virtual void fromQColor(const QColor& color, quint8 *dst, const KoColorProfile * profile = 0) const;
        virtual void toQColor(const quint8 *src, QColor *c, const KoColorProfile * profile = 0) const;
        virtual quint8 intensity8(const quint8 * src) const;
        virtual KoID mathToolboxId() const;
        virtual void colorToXML( const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const;
        virtual void colorFromXML( quint8* pixel, const QDomElement& elt) const;
    private:
        struct Private;
        Private* const d;
};

class PIGMENTCMS_EXPORT KoCtlColorSpaceFactory : public KoColorSpaceFactory {
    public:
        virtual ~KoCtlColorSpaceFactory() {}
        virtual bool profileIsCompatible(const KoColorProfile* profile) const;
        QList<KoColorConversionTransformationFactory*> colorConversionLinks() const;
};

#endif
