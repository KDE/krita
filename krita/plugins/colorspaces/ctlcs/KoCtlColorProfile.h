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

#ifndef _KO_CTL_COLOR_PROFILE_H_
#define _KO_CTL_COLOR_PROFILE_H_

#include <KoColorProfile.h>
#include "pigment_export.h"

class QDomElement;
class KoColorConversionTransformationFactory;
class KoColorSpace;

namespace GTLCore
{
class PixelDescription;
};

namespace OpenCTL
{
class Program;
};

/**
 * This class is a color profile for a color space which is managed by CTL programs.
 */
class PIGMENTCMS_EXPORT KoCtlColorProfile : public KoColorProfile
{
public:
    KoCtlColorProfile(QString fileName);
    KoCtlColorProfile(const KoCtlColorProfile&);
    virtual ~KoCtlColorProfile();
    virtual KoColorProfile* clone() const;
    virtual bool valid() const;
    virtual bool isSuitableForOutput() const;
    virtual bool isSuitableForPrinting() const;
    virtual bool isSuitableForDisplay() const;
    virtual bool operator==(const KoColorProfile&) const;
    virtual bool load();
    /**
     * @return the color model that can be used by this profile
     */
    QString colorModel() const;
    /**
     * @return the color depth that can be used by this profile
     */
    QString colorDepth() const;
public:

    /**
     * This function create a color conversion program between two color spaces.
     */
    OpenCTL::Program* createColorConversionProgram(const KoColorSpace* csSrc, const KoColorSpace* csDst) const;

    /**
     * This function will create the list of color conversion transformation factories that
     * are available using this profile.
     *
     * XXX: who will deleted these factories? memcheck says they are leaked.
     */
    QList<KoColorConversionTransformationFactory*> createColorConversionTransformationFactories() const;

    virtual QVariant property(const QString& _name) const;
    virtual void setProperty(const QString& _name, const QVariant& _variant);
private:
    /** decode the \<transformations\> tag.
     */
    void decodeTransformations(QDomElement&);
    /** decode the \<conversions\> tag.
     */
    void decodeConversions(QDomElement&);
private:
    struct Private;
    Private* const d;
};

#endif
