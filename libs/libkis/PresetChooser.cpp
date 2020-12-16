/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "PresetChooser.h"

#include <KoResource.h>
#include <kis_config.h>
#include "Resource.h"

PresetChooser::PresetChooser(QWidget *parent)
    : KisPresetChooser(parent)
{
    connect(this, SIGNAL(resourceSelected(KoResourceSP )), SLOT(slotResourceSelected(KoResourceSP )));
    connect(this, SIGNAL(resourceClicked(KoResourceSP )), SLOT(slotResourceClicked(KoResourceSP )));
    showTaggingBar(true);
}


void PresetChooser::setCurrentPreset(Resource *resource)
{
    KoResourceSP r = resource->resource();
    setCurrentResource(r);
}

Resource *PresetChooser::currentPreset() const
{
    KoResourceSP r = currentResource();
    if (r) {
        return new Resource(r->resourceId(), "paintoppreset", r->name(), r->filename(), r->image());
    }
    return 0;
}

void PresetChooser::slotResourceSelected(KoResourceSP r)
{
    emit presetSelected(Resource(r->resourceId(), "paintoppreset", r->name(), r->filename(), r->image()));
}

void PresetChooser::slotResourceClicked(KoResourceSP r)
{
    emit presetClicked(Resource(r->resourceId(), "paintoppreset", r->name(), r->filename(), r->image()));
}
