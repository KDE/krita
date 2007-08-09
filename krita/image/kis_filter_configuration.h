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

#include "kis_properties_configuration.h"

#include <QBitArray>

#include "kis_types.h"
#include "krita_export.h"


/**
 * A KisFilterConfiguration is the serializable representation of
 * the filter parameters. Filters can subclass this class to implement
 * direct accessors to properties, but properties not in the map will
 * not be serialized.
 *
 * XXX: Use KoProperties here!
 */
class KRITAIMAGE_EXPORT KisFilterConfiguration : public KisPropertiesConfiguration {

public:

    /**
     * Create a new filter config.
     */
    KisFilterConfiguration(const QString & name, qint32 version);

    /**
     * Deep copy the filter configFile
     */
    KisFilterConfiguration(const KisFilterConfiguration & rhs);

    virtual ~KisFilterConfiguration();

public:

    virtual void fromLegacyXML(QString);
    virtual void fromLegacyXML(const QDomElement&);
    virtual QString toLegacyXML() const;
    virtual void toLegacyXML(QDomDocument&, QDomElement&) const;

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
     * The default implementation allways return true.
     */
    virtual bool isCompatible(const KisPaintDeviceSP) const;


    /**
     * @return an array with each colorspace channel a true/false bit
     * that indicates whether the channel should be filtered or left
     * alone. It is up to the filter to decide whether channels that
     * are to be left alone are copied to the dest file or not.
     */
    QBitArray channelFlags();

    /**
     * Set the channel flags. An empty array is allowed; that means
     * that all channels are to be filtered. Filters can optimize on
     * that.
     */
    void setChannelFlags(QBitArray channelFlags);

private:
    struct Private;
    Private* const d;
};

class KRITAIMAGE_EXPORT KisFilterConfigurationFactory : public KisSerializableConfigurationFactory {
    public:
        KisFilterConfigurationFactory(const QString & name, qint32 version);
        virtual ~KisFilterConfigurationFactory();
        virtual KisSerializableConfiguration* create(const QDomElement& e);
    private:
        struct Private;
        Private* const d;
};

#endif // _KIS_FILTER_CONFIGURATION_H_
