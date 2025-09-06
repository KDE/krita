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


struct KisTextPropertiesManager::Private {
    KisCanvasResourceProvider *provider {nullptr};
    KoSvgTextPropertiesInterface *interface {nullptr};

    KoSvgTextPropertyData lastSetTextData;
    KoSvgTextPropertyData lastSetCharacterData;

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
        d->providerConnectionStore.addUniqueConnection(d->provider, SIGNAL(sigCharacterPropertiesChanged()), this, SLOT(slotCharacterPropertiesChanged()));
    }
}

void KisTextPropertiesManager::setTextPropertiesInterface(KoSvgTextPropertiesInterface *interface)
{
    d->interfaceConnectionStore.clear();
    d->interface = interface;
    if (d->interface) {
        d->interfaceConnectionStore.addUniqueConnection(d->interface, SIGNAL(textSelectionChanged()), this, SLOT(slotInterfaceSelectionChanged()));
        d->interfaceConnectionStore.addUniqueConnection(d->interface, SIGNAL(textCharacterSelectionChanged()), this, SLOT(slotCharacterInterfaceSelectionChanged()));
        slotInterfaceSelectionChanged();
        slotCharacterInterfaceSelectionChanged();
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

QSet<KoSvgTextProperties::PropertyId> getTristate(QList<KoSvgTextProperties> props) {
    QSet<KoSvgTextProperties::PropertyId> propIds;

    for (auto it = props.begin(); it != props.end(); it++) {
        KoSvgTextProperties p = *it;
        for (int i = 0; i < p.properties().size(); i++) {
            propIds.insert(p.properties().at(i));
        }
    }
    return propIds;
}

void KisTextPropertiesManager::slotInterfaceSelectionChanged() {
    if (!d->interface || !d->provider) return;

    QList<KoSvgTextProperties> props = d->interface->getSelectedProperties();
    if (props.isEmpty()) return;

    QSet<KoSvgTextProperties::PropertyId> propIds = getTristate(props);

    KoSvgTextPropertyData textData = textDataProperties(props, propIds);
    textData.inheritedProperties = KoSvgTextProperties();
    textData.enabled = true;

    d->lastSetTextData = textData;
    d->provider->setTextPropertyData(textData);

}

void KisTextPropertiesManager::slotCharacterInterfaceSelectionChanged()
{
    if (!d->interface || !d->provider) return;
    KoSvgTextPropertyData charData;
    if (d->interface->characterPropertiesEnabled()) {
        QList<KoSvgTextProperties> charProps = d->interface->getCharacterProperties();
        if (!charProps.isEmpty()) {
            QSet<KoSvgTextProperties::PropertyId> charPropIds = getTristate(charProps);
            charData = textDataProperties(charProps, charPropIds);
        }
        charData.inheritedProperties = d->interface->getInheritedProperties();
        charData.spanSelection = d->interface->spanSelection();
        charData.enabled = true;
    }
    d->lastSetCharacterData = charData;
    d->provider->setCharacterPropertyData(charData);
}

QSet<KoSvgTextProperties::PropertyId> removedProps(KoSvgTextPropertyData textData, KoSvgTextPropertyData oldProps) {
    QSet<KoSvgTextProperties::PropertyId> removeProperties;
    QList<KoSvgTextProperties::PropertyId> oldPropIds = oldProps.commonProperties.properties();
    oldPropIds.append(oldProps.tristate.values());
    for (auto it = oldPropIds.begin(); it != oldPropIds.end(); it++) {
        KoSvgTextProperties::PropertyId p = *it;
        if (!textData.commonProperties.hasProperty(p) && !textData.tristate.contains(p)) {
            removeProperties.insert(p);
        }
    }
    return removeProperties;
}

void KisTextPropertiesManager::slotTextPropertiesChanged()
{
    if (!d->interface || !d->provider) return;
    if (d->lastSetTextData == d->provider->textPropertyData()) return;

    KoSvgTextPropertyData textData = d->provider->textPropertyData();

    KoSvgTextProperties newProps = textData.commonProperties.ownProperties(d->lastSetTextData.commonProperties);
    QSet<KoSvgTextProperties::PropertyId> removeProperties = removedProps(textData, d->lastSetTextData);

    if (newProps.isEmpty() && removeProperties.isEmpty()) return;
    d->interface->setPropertiesOnSelected(newProps, removeProperties);
}

void KisTextPropertiesManager::slotCharacterPropertiesChanged()
{
    if (!d->interface || !d->provider) return;
    if (!d->interface->characterPropertiesEnabled()) return;
    KoSvgTextPropertyData textData = d->provider->characterTextPropertyData();
    if (textData == d->lastSetCharacterData) return;

    KoSvgTextProperties newProps = textData.commonProperties.ownProperties(d->lastSetCharacterData.commonProperties);
    QSet<KoSvgTextProperties::PropertyId> removeProperties = removedProps(textData, d->lastSetCharacterData);

    if (newProps.isEmpty() && removeProperties.isEmpty()) return;
    d->interface->setCharacterPropertiesOnSelected(newProps, removeProperties);
}
