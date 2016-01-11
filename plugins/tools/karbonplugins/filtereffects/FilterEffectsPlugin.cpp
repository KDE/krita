/* This file is part of the KDE project
* Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "FilterEffectsPlugin.h"
#include "BlurEffectFactory.h"
#include "OffsetEffectFactory.h"
#include "MergeEffectFactory.h"
#include "ColorMatrixEffectFactory.h"
#include "FloodEffectFactory.h"
#include "CompositeEffectFactory.h"
#include "BlendEffectFactory.h"
#include "ComponentTransferEffectFactory.h"
#include "ImageEffectFactory.h"
#include "MorphologyEffectFactory.h"
#include "ConvolveMatrixEffectFactory.h"

#include "KoFilterEffectRegistry.h"

#include <kpluginfactory.h>

K_PLUGIN_FACTORY_WITH_JSON(FilterEffectsPluginFacory, "karbon_filtereffects.json", registerPlugin<FilterEffectsPlugin>();)

FilterEffectsPlugin::FilterEffectsPlugin(QObject *parent, const QList<QVariant> &)
    : QObject(parent)
{
    KoFilterEffectRegistry::instance()->add(new BlurEffectFactory());
    KoFilterEffectRegistry::instance()->add(new OffsetEffectFactory());
    KoFilterEffectRegistry::instance()->add(new MergeEffectFactory());
    KoFilterEffectRegistry::instance()->add(new ColorMatrixEffectFactory());
    KoFilterEffectRegistry::instance()->add(new FloodEffectFactory());
    KoFilterEffectRegistry::instance()->add(new CompositeEffectFactory());
    KoFilterEffectRegistry::instance()->add(new BlendEffectFactory());
    KoFilterEffectRegistry::instance()->add(new ComponentTransferEffectFactory());
    KoFilterEffectRegistry::instance()->add(new ImageEffectFactory());
    KoFilterEffectRegistry::instance()->add(new MorphologyEffectFactory());
    KoFilterEffectRegistry::instance()->add(new ConvolveMatrixEffectFactory());
}
#include <FilterEffectsPlugin.moc>
