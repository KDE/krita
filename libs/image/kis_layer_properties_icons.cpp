/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_layer_properties_icons.h"

#include <QMap>

#include <QGlobalStatic>
Q_GLOBAL_STATIC(KisLayerPropertiesIcons, s_instance)

#include <kis_icon_utils.h>


const KoID KisLayerPropertiesIcons::locked("locked", i18n("Locked"));
const KoID KisLayerPropertiesIcons::visible("visible", i18n("Visible"));
const KoID KisLayerPropertiesIcons::layerStyle("layer-style", i18n("Layer Style"));
const KoID KisLayerPropertiesIcons::inheritAlpha("inherit-alpha", i18n("Inherit Alpha"));
const KoID KisLayerPropertiesIcons::alphaLocked("alpha-locked", i18n("Alpha Locked"));
const KoID KisLayerPropertiesIcons::onionSkins("onion-skins", i18n("Onion Skins"));
const KoID KisLayerPropertiesIcons::passThrough("pass-through", i18n("Pass Through"));
const KoID KisLayerPropertiesIcons::selectionActive("selection-active", i18n("Active"));
const KoID KisLayerPropertiesIcons::colorLabelIndex("color-label", i18n("Color Label"));


struct IconsPair {
    IconsPair() {}
    IconsPair(const QIcon &_on, const QIcon &_off) : on(_on), off(_off) {}

    QIcon on;
    QIcon off;

    const QIcon& getIcon(bool state) {
        return state ? on : off;
    }
};

struct KisLayerPropertiesIcons::Private
{
    QMap<QString, IconsPair> icons;
};

KisLayerPropertiesIcons::KisLayerPropertiesIcons()
    : m_d(new Private)
{
    updateIcons();
}

KisLayerPropertiesIcons::~KisLayerPropertiesIcons()
{
}

KisLayerPropertiesIcons *KisLayerPropertiesIcons::instance()
{
    return s_instance;
}

void KisLayerPropertiesIcons::updateIcons()
{
    m_d->icons.clear();
    m_d->icons.insert(locked.id(), IconsPair(KisIconUtils::loadIcon("layer-locked"), KisIconUtils::loadIcon("layer-unlocked")));
    m_d->icons.insert(visible.id(), IconsPair(KisIconUtils::loadIcon("visible"), KisIconUtils::loadIcon("novisible")));
    m_d->icons.insert(layerStyle.id(), IconsPair(KisIconUtils::loadIcon("layer-style-enabled"), KisIconUtils::loadIcon("layer-style-disabled")));
    m_d->icons.insert(inheritAlpha.id(), IconsPair(KisIconUtils::loadIcon("transparency-disabled"), KisIconUtils::loadIcon("transparency-enabled")));
    m_d->icons.insert(alphaLocked.id(), IconsPair(KisIconUtils::loadIcon("transparency-locked"), KisIconUtils::loadIcon("transparency-unlocked")));
    m_d->icons.insert(onionSkins.id(), IconsPair(KisIconUtils::loadIcon("onionOn"), KisIconUtils::loadIcon("onionOff")));
    m_d->icons.insert(passThrough.id(), IconsPair(KisIconUtils::loadIcon("passthrough-enabled"), KisIconUtils::loadIcon("passthrough-disabled")));
    m_d->icons.insert(selectionActive.id(), IconsPair(KisIconUtils::loadIcon("local_selection_active"), KisIconUtils::loadIcon("local_selection_inactive")));
}

KisBaseNode::Property KisLayerPropertiesIcons::getProperty(const KoID &id, bool state)
{
    const IconsPair &pair = instance()->m_d->icons[id.id()];
    return KisBaseNode::Property(id,
                                 pair.on, pair.off, state);
}

KisBaseNode::Property KisLayerPropertiesIcons::getProperty(const KoID &id, bool state,
                                                                       bool isInStasis, bool stateInStasis)
{
    const IconsPair &pair = instance()->m_d->icons[id.id()];
    return KisBaseNode::Property(id,
                                 pair.on, pair.off, state,
                                 isInStasis, stateInStasis);
}
