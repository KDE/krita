/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_FILTER_CONFIGURATION_H_
#define _KIS_FILTER_CONFIGURATION_H_

#include <QMetaType>

#include "kis_properties_configuration.h"

#include "kis_types.h"
#include "krita_export.h"

/**
 * KisFilterConfiguration does inherit neither KisShared or QSharedData
 * so sometimes there might be problem with broken QSharedPointer counters.
 * This macro activates debugging routines for such stuff.
 *
 * In the future, please either port the entire KisNodeFilterInterface
 * into KisSafeFilterConfigurationSP or derive filter configuration
 * interface from QSharedData to handle these cases.
 */
#define SANITY_CHECK_FILTER_CONFIGURATION_OWNER

/**
 * A KisFilterConfiguration is the serializable representation of
 * the filter parameters. Filters can subclass this class to implement
 * direct accessors to properties, but properties not in the map will
 * not be serialized.
 *
 * XXX: Use KoProperties here!
 */
class KRITAIMAGE_EXPORT KisFilterConfiguration : public KisPropertiesConfiguration
{

public:

    /**
     * Create a new filter config.
     */
    KisFilterConfiguration(const QString & name, qint32 version);

protected:
    /**
     * Deep copy the filter configFile
     */
    KisFilterConfiguration(const KisFilterConfiguration & rhs);
public:
    virtual ~KisFilterConfiguration();

public:

    /**
     * This function is use to convert from legacy XML as used in .kra file.
     */
    virtual void fromLegacyXML(const QDomElement&);

    using KisPropertiesConfiguration::fromXML;
    using KisPropertiesConfiguration::toXML;

    virtual void fromXML(const QDomElement&);
    virtual void toXML(QDomDocument&, QDomElement&) const;

    /**
     * Get the unique, language independent name of the filter.
     */
    const QString & name() const;

    /**
     * Get the version of the filter that has created this config
     */
    qint32 version() const;

    /**
     * Check if that configuration is compatible with this paint device.
     * The default implementation always return true.
     */
    virtual bool isCompatible(const KisPaintDeviceSP) const;


    /**
     * @return an array with each colorspace channel a true/false bit
     * that indicates whether the channel should be filtered or left
     * alone. It is up to the filter to decide whether channels that
     * are to be left alone are copied to the dest file or not.
     */
    QBitArray channelFlags() const;

    /**
     * Set the channel flags. An empty array is allowed; that means
     * that all channels are to be filtered. Filters can optimize on
     * that. The array must be in the order of the pixel layout.
     */
    void setChannelFlags(QBitArray channelFlags);

    /**
     * These functions exist solely to allow plugins to reimplement them as
     * needed, while allowing consumers to implement support for them without
     * linking directly to the plugin. In particular, the filter management
     * in Sketch requires this.
     */
    virtual void setCurve(const KisCubicCurve &curve);
    virtual const KisCubicCurve& curve() const;
    virtual void setCurves(QList<KisCubicCurve> &curves);
    virtual const QList<KisCubicCurve>& curves() const;

#ifdef SANITY_CHECK_FILTER_CONFIGURATION_OWNER
private:
    friend class KisNodeFilterInterface;
    int sanityRefUsageCounter();
    int sanityDerefUsageCounter();

#endif /* SANITY_CHECK_FILTER_CONFIGURATION_OWNER */

protected:
    void setVersion(qint32 version);
private:
    struct Private;
    Private* const d;
};

Q_DECLARE_METATYPE(KisFilterConfiguration*)

#endif // _KIS_FILTER_CONFIGURATION_H_
