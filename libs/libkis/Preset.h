
/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef LIBKIS_PRESET_H
#define LIBKIS_PRESET_H

#include "kritalibkis_export.h"
#include "libkis.h"
#include "Resource.h"
#include <kis_types.h>
#include <kis_paintop_preset.h>

/**
 * @brief The Preset class
 * Preset is a resource object that stores brush preset data.
 *
 * An example for printing the current brush preset and all its settings:
 *
 * @code
from krita import *

view = Krita.instance().activeWindow().activeView()
preset = Preset(view.currentBrushPreset())

print ( preset.toXML() )

 * @endcode
 */

class KRITALIBKIS_EXPORT Preset : public QObject
{
public:
    Preset(Resource *resource);
    ~Preset() override;

    /**
     * @brief toXML
     * convert the preset settings into a preset formatted xml.
     * @return the xml in a string.
     */
    QString toXML() const;

    /**
     * @brief fromXML
     * convert the preset settings into a preset formatted xml.
     * @param xml valid xml preset string.
     */
    void fromXML(const QString &xml);

private:
    struct Private;
    Private *const d;

    /**
     * @brief paintOpPreset
     * @return gives a KisPaintOpPreset object back
     */
    KisPaintOpPresetSP paintOpPreset();

};

#endif // LIBKIS_PRESET_H
