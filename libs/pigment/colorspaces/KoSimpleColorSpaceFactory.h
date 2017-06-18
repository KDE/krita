/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2005-2006 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2004,2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#ifndef KOSIMPLECOLORSPACEFACTORY_H_
#define KOSIMPLECOLORSPACEFACTORY_H_

#include "KoColorConversionTransformationFactory.h"
#include <colorprofiles/KoDummyColorProfile.h>

#include <KoColorModelStandardIds.h>

class KoSimpleColorSpaceFactory : public KoColorSpaceFactory
{

public:

    KoSimpleColorSpaceFactory(const QString& id,
                              const QString& name,
                              bool userVisible,
                              const KoID& colorModelId,
                              const KoID& colorDepthId,
                              int referenceDepth = -1,
                              int crossingCost = 1)
        : m_id(id)
        , m_name(name)
        , m_userVisible(userVisible)
        , m_colorModelId(colorModelId)
        , m_colorDepthId(colorDepthId)
        , m_referenceDepth(referenceDepth)
        , m_crossingCost(crossingCost)
    {
        if (m_referenceDepth >= 0) {
            // noop, already initialized!
        } else if (colorDepthId == Integer8BitsColorDepthID) {
            m_referenceDepth = 1 * 8;
        } else if (colorDepthId == Integer16BitsColorDepthID) {
            m_referenceDepth = 2 * 8;
        } else if (colorDepthId == Float16BitsColorDepthID) {
            m_referenceDepth = 2 * 8;
        } else if (colorDepthId == Float32BitsColorDepthID) {
            m_referenceDepth = 4 * 8;
        } else if (colorDepthId == Float64BitsColorDepthID) {
            m_referenceDepth = 8 * 8;
        }
    }


    QString id() const override {
        return m_id;
    }

    QString name() const override {
        return m_name;
    }

    bool userVisible() const override {
        return m_userVisible;
    }

    KoID colorModelId() const override {
        return m_colorModelId;
    }

    KoID colorDepthId() const override {
        return m_colorDepthId;
    }

    bool profileIsCompatible(const KoColorProfile* profile) const override {
        return dynamic_cast<const KoDummyColorProfile*>(profile);
    }

    QString colorSpaceEngine() const override {
        return "simple";
    }

    bool isHdr() const override {
        return false;
    }

    int referenceDepth() const override {
        return m_referenceDepth;
    }

    int crossingCost() const override {
        return m_crossingCost;
    }

    QList<KoColorConversionTransformationFactory*> colorConversionLinks() const override {
        return QList<KoColorConversionTransformationFactory*>();
    }

    QString defaultProfile() const override {
        return QString("default");
    }
protected:
    KoColorProfile* createColorProfile(const QByteArray& /*rawData*/) const override {
        return 0;
    }
private:

    QString m_id;
    QString m_name;
    bool    m_userVisible;
    KoID    m_colorModelId;
    KoID    m_colorDepthId;
    int     m_referenceDepth;
    int m_crossingCost;

};

#endif
