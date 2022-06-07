/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "Preset.h"
#include <QDomDocument>

struct Preset::Private {
    KisPaintOpPresetSP preset {0};
};

Preset::Preset(Resource *resource): d(new Private()) {
    d->preset = resource->resource().dynamicCast<KisPaintOpPreset>();
}

Preset::~Preset()
{
    delete d;
}


QString Preset::toXML() const
{
    QDomDocument xmlDoc;
    QDomElement root = xmlDoc.createElement("Preset");

    d->preset->toXML(xmlDoc,root);

    xmlDoc.appendChild(root);

    return xmlDoc.toString();
}

void Preset::fromXML(const QString &xml)
{
    QDomDocument xmlDoc;

    if (!xmlDoc.setContent(xml)) {
        qWarning() << "XML string format is invalid!";
        return;
    }

    d->preset->fromXML(xmlDoc.documentElement(),d->preset->resourcesInterface());
    d->preset->setDirty(true);
}
