/*
 *  Copyright (c) 2008-2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KOCTLCOLORSPACEFACTORY_H_
#define _KOCTLCOLORSPACEFACTORY_H_

#include <KoColorSpace.h>

class KoCtlColorSpaceInfo;

class KoCtlColorSpaceFactory : public KoColorSpaceFactory
{
public:
    KoCtlColorSpaceFactory(KoCtlColorSpaceInfo*);
    virtual ~KoCtlColorSpaceFactory();
    virtual QString id() const ;
    virtual QString name() const;
    virtual bool userVisible() const;
    virtual KoID colorModelId() const;
    virtual KoID colorDepthId() const;
    virtual KoColorSpace *createColorSpace(const KoColorProfile *) const;
    virtual QString colorSpaceEngine() const;
    virtual bool isHdr() const;
    virtual int referenceDepth() const;
    virtual QString defaultProfile() const;
    virtual bool profileIsCompatible(const KoColorProfile* profile) const;
    // XXX: are these factories ever deleted? memcheck says not.
    QList<KoColorConversionTransformationFactory*> colorConversionLinks() const;
private:
    KoCtlColorSpaceInfo* m_info;
};

#endif
