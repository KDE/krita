/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "PresetChooser.h"

#include <KoResource.h>
#include <kis_config.h>
#include "Resource.h"

PresetChooser::PresetChooser(QWidget *parent)
    : KisPresetChooser(parent)
{
    connect(this, SIGNAL(resourceSelected(KoResource*)), SLOT(slotResourceSelected(KoResource*)));
    connect(this, SIGNAL(resourceClicked(KoResource*)), SLOT(slotResourceClicked(KoResource*)));
    showTaggingBar(true);
}


void PresetChooser::setCurrentPreset(Resource *resource)
{
    KoResource *r = resource->resource();
    setCurrentResource(r);
}

Resource *PresetChooser::currentPreset() const
{
    KoResource *r = currentResource();
    return new Resource(r);
}

void PresetChooser::slotResourceSelected(KoResource *resource)
{
    Resource *r = new Resource(resource);
    emit presetSelected(r);
}

void PresetChooser::slotResourceClicked(KoResource *resource)
{
    Resource *r = new Resource(resource);
    emit presetClicked(r);
}
