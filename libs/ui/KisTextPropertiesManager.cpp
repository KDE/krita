/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisTextPropertiesManager.h"
#include <KoSelectedShapesProxy.h>
#include <kis_canvas_resource_provider.h>
#include <KoSvgTextPropertyData.h>
#include <KoSelection.h>
#include <KoSvgTextShape.h>
#include <KoSvgTextPropertiesInterface.h>

#include <KisView.h>
#include <kis_canvas2.h>
#include <kis_signal_auto_connection.h>

#include <QPointer>

struct KisTextPropertiesManager::Private {
    KisCanvasResourceProvider *provider {nullptr};
    KoSvgTextPropertiesInterface *interface {nullptr};

    KoSvgTextPropertyData lastSetTextData;

    KisSignalAutoConnectionsStore interfaceConnectionStore;
    KisSignalAutoConnectionsStore providerConnectionStore;
};

KisTextPropertiesManager::KisTextPropertiesManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

KisTextPropertiesManager::~KisTextPropertiesManager()
{
}

void KisTextPropertiesManager::setCanvasResourceProvider(KisCanvasResourceProvider *provider)
{
    d->providerConnectionStore.clear();
    d->provider = provider;
    if (d->provider) {
        d->providerConnectionStore.addUniqueConnection(d->provider, SIGNAL(sigTextPropertiesChanged()), this, SLOT(slotTextPropertiesChanged()));
    }
}

void KisTextPropertiesManager::setTextPropertiesInterface(KoSvgTextPropertiesInterface *interface)
{
    d->interfaceConnectionStore.clear();
    d->interface = interface;
    if (d->interface) {
        d->interfaceConnectionStore.addUniqueConnection(d->interface, SIGNAL(textSelectionChanged()), this, SLOT(slotInterfaceSelectionChanged()));
        slotInterfaceSelectionChanged();
    }
}

KoSvgTextPropertyData textDataProperties(QList<KoSvgTextProperties> props, QSet<KoSvgTextProperties::PropertyId> propIds) {
    KoSvgTextPropertyData textData;
    textData.commonProperties = props.first();

    for (auto it = props.begin(); it != props.end(); it++) {
        for (int i = 0; i < propIds.size(); i++) {
            KoSvgTextProperties::PropertyId p = propIds.values().at(i);
            if (it->hasProperty(p)) {
                if (textData.commonProperties.property(p) != it->property(p)) {
                    textData.commonProperties.removeProperty(p);
                    textData.tristate.insert(p);
                }
            } else {
                textData.commonProperties.removeProperty(p);
                textData.tristate.insert(p);
            }
        }
    }
    return textData;
}

void KisTextPropertiesManager::slotInterfaceSelectionChanged() {
    if (!d->interface || !d->provider) return;

    QList<KoSvgTextProperties> props = d->interface->getSelectedProperties();
    if (props.isEmpty()) return;

    QSet<KoSvgTextProperties::PropertyId> propIds;

    for (auto it = props.begin(); it != props.end(); it++) {
        KoSvgTextProperties p = *it;
        for (int i = 0; i < p.properties().size(); i++) {
            propIds.insert(p.properties().at(i));
        }
    }

    KoSvgTextPropertyData textData = textDataProperties(props, propIds);
    textData.inheritedProperties = d->interface->getInheritedProperties();
    textData.spanSelection = d->interface->spanSelection();

    d->lastSetTextData = textData;
    d->provider->setTextPropertyData(textData);

}

void KisTextPropertiesManager::slotTextPropertiesChanged()
{
    if (!d->interface || !d->provider) return;
    if (d->lastSetTextData == d->provider->textPropertyData()) return;

    KoSvgTextPropertyData textData = d->provider->textPropertyData();

    KoSvgTextProperties newProps = textData.commonProperties.ownProperties(d->lastSetTextData.commonProperties);
    QSet<KoSvgTextProperties::PropertyId> removeProperties;

    QList<KoSvgTextProperties::PropertyId> oldPropIds = d->lastSetTextData.commonProperties.properties();
    oldPropIds.append(d->lastSetTextData.tristate.values());
    for (auto it = oldPropIds.begin(); it != oldPropIds.end(); it++) {
        KoSvgTextProperties::PropertyId p = *it;
        if (!textData.commonProperties.hasProperty(p) && !textData.tristate.contains(p)) {
            removeProperties.insert(p);
        }
    }

    if (newProps.isEmpty() && removeProperties.isEmpty()) return;
    d->interface->setPropertiesOnSelected(newProps, removeProperties);
}
