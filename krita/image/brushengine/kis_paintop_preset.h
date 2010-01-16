/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#ifndef KIS_PAINTOP_PRESET_H
#define KIS_PAINTOP_PRESET_H

#include "KoResource.h"
#include "KoID.h"

#include "kis_types.h"
#include "kis_shared.h"
#include "krita_export.h"

class QImage;

/**
 * A KisPaintOpPreset contains a particular set of settings
 * associated with a paintop, like brush, paintopsettings.
 */
class KRITAIMAGE_EXPORT KisPaintOpPreset : public KoResource, public KisShared
{
public:

    KisPaintOpPreset();

    KisPaintOpPreset(const QString& filename);

    ~KisPaintOpPreset();

    KisPaintOpPreset* clone() const;

    /// set the id of the paintop plugin
    void setPaintOp(const KoID & paintOp);

    /// return the id of the paintop plugin
    KoID paintOp() const;

    /// replace the current settings object with the specified settings
    void setSettings(KisPaintOpSettingsSP settings);

    /// return the settings that define this paintop preset
    KisPaintOpSettingsSP settings() const;

    bool load();

    bool save();
    
    void toXML(QDomDocument& doc, QDomElement& elt) const;

    void fromXML(const QDomElement& elt);


    QImage image() const;

    bool removable() const {
        return true;
    }

    QString defaultFileExtension() const {
        return "kpp";
    }

    void updateImage();
    
    QImage generatePreviewImage(int width, int height);

private:

    struct Private;
    Private * const m_d;
};

Q_DECLARE_METATYPE(KisPaintOpPresetSP)

#endif
