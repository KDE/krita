/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_FILTER_CONFIGURATION_H_
#define _KIS_FILTER_CONFIGURATION_H_

#include <QMetaType>

#include "kis_properties_configuration.h"

#include "kis_types.h"
#include "kritaimage_export.h"


class KoResource;
typedef QSharedPointer<KoResource> KoResourceSP;

class KisResourcesInterface;
typedef QSharedPointer<KisResourcesInterface> KisResourcesInterfaceSP;

/**
 * KisFilterConfiguration does inherit neither KisShared or QSharedData
 * so sometimes there might be problem with broken QSharedPointer counters.
 * This macro activates debugging routines for such stuff.
 *
 * In the future, please either port the entire KisNodeFilterInterface
 * into KisFilterConfigurationSP or derive filter configuration
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
    KisFilterConfiguration(const QString & name, qint32 version, KisResourcesInterfaceSP resourcesInterface);

    /**
     * @return an exact copy of the filter configuration. Resources interface is
     * becomes shared between two configuration objects.
     */
    virtual KisFilterConfigurationSP clone() const;

protected:
    /**
     * Deep copy the filter configFile
     */
    KisFilterConfiguration(const KisFilterConfiguration & rhs);
public:
    ~KisFilterConfiguration() override;

public:

    /**
     * This function is use to convert from legacy XML as used in .kra file.
     */
    virtual void fromLegacyXML(const QDomElement&);

    using KisPropertiesConfiguration::fromXML;
    using KisPropertiesConfiguration::toXML;

    void fromXML(const QDomElement&) override;
    void toXML(QDomDocument&, QDomElement&) const override;

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

    /**
     * @return resource interface that is used by KisFilterConfiguration object for
     * loading linked resources
     */
    KisResourcesInterfaceSP resourcesInterface() const;

    /**
     * Set resource interface that will be used by KisFilterConfiguration object for
     * loading linked resources
     */
    void setResourcesInterface(KisResourcesInterfaceSP resourcesInterface);

    /**
     * \see KisRequiredResourcesOperators::createLocalResourcesSnapshot
     */
    void createLocalResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface = nullptr);

    /**
     * \see KisRequiredResourcesOperators::hasLocalResourcesSnapshot
     */
    bool hasLocalResourcesSnapshot() const;

    /**
     * \see KisRequiredResourcesOperators::cloneWithResourcesSnapshot
     */
    KisFilterConfigurationSP cloneWithResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface = nullptr) const;

    /**
     * Loads all the required resources either from \p globalResourcesInterface or
     * from embedded data. The filter first tries to fetch the required resource
     * from the global source, and only if it fails, tries to load it from the
     * embedded data. One can check if the loaded resource is embedded by checking
     * its resourceId().
     *
     * The set of resources returned is basically: linkedResources() + embeddedResources()
     */
    QList<KoResourceSP> requiredResources(KisResourcesInterfaceSP globalResourcesInterface) const;

    /**
     * @return all the resources that are needed but (*this) filter and
     * are not embedded into it. The resources are fetched from
     * \p globalResourcesInterface. If fetching of some resources is failed,
     * then (*this) filter is invalid.
     */
    virtual QList<KoResourceSP> linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const;

    /**
     * @return all the resources that were embedded into (*this) filter.
     * If the resources were already added to the global database, then they
     * are fetched from \p globalResourcesInterface to save time/memory.
     */
    virtual QList<KoResourceSP> embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const;

    virtual bool compareTo(const KisPropertiesConfiguration *rhs) const override;

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
