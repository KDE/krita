/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter.h"

#include <kis_debug.h>
#include <QDomDocument>
#include <QString>

#include "filter/kis_filter_registry.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "KoID.h"
#include "kis_types.h"
#include <KisRequiredResourcesOperators.h>

#include "kis_config_widget.h"

struct Q_DECL_HIDDEN KisFilterConfiguration::Private {
    QString name;
    qint32 version;
    QBitArray channelFlags;
    KisCubicCurve curve;
    QList< KisCubicCurve > curves;
    KisResourcesInterfaceSP resourcesInterface = 0;

    Private(const QString & _name, qint32 _version, KisResourcesInterfaceSP _resourcesInterface)
        : name(_name),
          version(_version),
          resourcesInterface(_resourcesInterface)
    {
    }

    Private(const Private &rhs)
        : name(rhs.name),
          version(rhs.version),
          channelFlags(rhs.channelFlags),
          curve(rhs.curve),
          curves(rhs.curves),
          resourcesInterface(rhs.resourcesInterface)
    {
    }

#ifdef SANITY_CHECK_FILTER_CONFIGURATION_OWNER
    QAtomicInt sanityUsageCounter;
#endif /* SANITY_CHECK_FILTER_CONFIGURATION_OWNER */
};

KisFilterConfiguration::KisFilterConfiguration(const QString & name, qint32 version, KisResourcesInterfaceSP resourcesInterface)
        : d(new Private(name, version, resourcesInterface))
{
}

KisFilterConfigurationSP KisFilterConfiguration::clone() const
{
    return new KisFilterConfiguration(*this);
}

KisFilterConfiguration::KisFilterConfiguration(const KisFilterConfiguration & rhs)
        : KisPropertiesConfiguration(rhs)
        , d(new Private(*rhs.d))
{
}

KisFilterConfiguration::~KisFilterConfiguration()
{
    delete d;
}

void KisFilterConfiguration::fromLegacyXML(const QDomElement& e)
{
    clearProperties();
    d->name = e.attribute("name");
    d->version = e.attribute("version").toInt();

    QDomNode n = e.firstChild();


    while (!n.isNull()) {
        // We don't nest elements in filter configuration. For now...
        QDomElement e = n.toElement();
        QString name;
        QString type;
        QString value;

        if (!e.isNull()) {
            if (e.tagName() == "property") {
                name = e.attribute("name");
                type = e.attribute("type");
                value = e.text();
                // XXX Convert the variant pro-actively to the right type?
                setProperty(name, QVariant(value));
            }
        }
        n = n.nextSibling();
    }
}

void KisFilterConfiguration::fromXML(const QDomElement& elt)
{
    d->version = elt.attribute("version").toInt();
    KisPropertiesConfiguration::fromXML(elt);
}

void KisFilterConfiguration::toXML(QDomDocument& doc, QDomElement& elt) const
{
    elt.setAttribute("version", d->version);
    KisPropertiesConfiguration::toXML(doc, elt);
}


const QString & KisFilterConfiguration::name() const
{
    return d->name;
}

qint32 KisFilterConfiguration::version() const
{
    return d->version;
}

void KisFilterConfiguration::setVersion(qint32 version)
{
    d->version = version;
}

const KisCubicCurve& KisFilterConfiguration::curve() const
{
    return d->curve;
}

void KisFilterConfiguration::setCurve(const KisCubicCurve& curve)
{
    d->curve = curve;
}

const QList< KisCubicCurve >& KisFilterConfiguration::curves() const
{
    return d->curves;
}

KisResourcesInterfaceSP KisFilterConfiguration::resourcesInterface() const
{
    return d->resourcesInterface;
}

void KisFilterConfiguration::setResourcesInterface(KisResourcesInterfaceSP resourcesInterface)
{
    d->resourcesInterface = resourcesInterface;
}

void KisFilterConfiguration::createLocalResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface)
{
    KisRequiredResourcesOperators::createLocalResourcesSnapshot(this, globalResourcesInterface);
}

bool KisFilterConfiguration::hasLocalResourcesSnapshot() const
{
    return KisRequiredResourcesOperators::hasLocalResourcesSnapshot(this);
}

KisFilterConfigurationSP KisFilterConfiguration::cloneWithResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface) const
{
    return KisRequiredResourcesOperators::cloneWithResourcesSnapshot<KisFilterConfigurationSP>(this, globalResourcesInterface);
}

QList<KoResourceSP> KisFilterConfiguration::requiredResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    return linkedResources(globalResourcesInterface) + embeddedResources(globalResourcesInterface);
}

QList<KoResourceSP> KisFilterConfiguration::linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    Q_UNUSED(globalResourcesInterface);
    return {};
}

QList<KoResourceSP> KisFilterConfiguration::embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    Q_UNUSED(globalResourcesInterface);
    return {};
}

void KisFilterConfiguration::setCurves(QList< KisCubicCurve >& curves)
{
    d->curves = curves;
}

bool KisFilterConfiguration::isCompatible(const KisPaintDeviceSP) const
{
    return true;
}

bool KisFilterConfiguration::compareTo(const KisPropertiesConfiguration *rhs) const
{
    const KisFilterConfiguration *otherConfig = dynamic_cast<const KisFilterConfiguration *>(rhs);

    return otherConfig
        && KisPropertiesConfiguration::compareTo(rhs)
        && name() == otherConfig->name()
        && version() == otherConfig->version()
        && channelFlags() == otherConfig->channelFlags()
        && curve() == otherConfig->curve()
        && curves() == otherConfig->curves();
}

QBitArray KisFilterConfiguration::channelFlags() const
{
    return d->channelFlags;
}

void KisFilterConfiguration::setChannelFlags(QBitArray channelFlags)
{
    d->channelFlags = channelFlags;
}

#ifdef SANITY_CHECK_FILTER_CONFIGURATION_OWNER

int KisFilterConfiguration::sanityRefUsageCounter()
{
    return d->sanityUsageCounter.ref();
}

int KisFilterConfiguration::sanityDerefUsageCounter()
{
    return d->sanityUsageCounter.deref();
}

#endif /* SANITY_CHECK_FILTER_CONFIGURATION_OWNER */
