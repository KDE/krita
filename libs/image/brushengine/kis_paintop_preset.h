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

#include <resources/KoResource.h>
#include "KoID.h"

#include "kis_types.h"
#include "kis_shared.h"
#include "kritaimage_export.h"
#include <brushengine/kis_uniform_paintop_property.h>

class KisPaintopSettingsUpdateProxy;
class KisPaintOpConfigWidget;

/**
 * A KisPaintOpPreset contains a particular set of settings
 * associated with a paintop, like brush, paintopsettings.
 * A new property in this class is to make it dirty. That means the
 * user can now temporarily save any tweaks in the Preset throughout
 * the session. The Dirty Preset setting/unsetting is handled by KisPaintOpPresetSettings
 */
class KRITAIMAGE_EXPORT KisPaintOpPreset : public KoResource, public KisShared
{
public:

    KisPaintOpPreset();

    KisPaintOpPreset(const QString& filename);

    ~KisPaintOpPreset() override;

    KisPaintOpPresetSP clone() const;

    /// set the id of the paintop plugin
    void setPaintOp(const KoID & paintOp);

    /// return the id of the paintop plugin
    KoID paintOp() const;

    /// replace the current settings object with the specified settings
    void setSettings(KisPaintOpSettingsSP settings);
    void setOriginalSettings(KisPaintOpSettingsSP originalSettings);

    /// return the settings that define this paintop preset
    KisPaintOpSettingsSP settings() const;
    KisPaintOpSettingsSP originalSettings() const;

    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;

    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;

    void toXML(QDomDocument& doc, QDomElement& elt) const;

    void fromXML(const QDomElement& elt);

    bool removable() const {
        return true;
    }

    QString defaultFileExtension() const override {
        return ".kpp";
    }

    /// Mark the preset as modified but not saved
    void setDirty(bool value);

    /// @return true if the preset has been modified, but not saved
    bool isDirty() const;

    /**
     * Never use manual save/restore calls to
     * isPresetDirty()/setPresetDirty()! They will lead to
     * hard-to-tack-down bugs when the dirty state will not be
     * restored on jumps like 'return', 'break' or exception.
     */
    class KRITAIMAGE_EXPORT DirtyStateSaver {
    public:
        DirtyStateSaver(KisPaintOpPreset *preset)
            : m_preset(preset), m_isDirty(preset->isDirty())
        {
        }

        ~DirtyStateSaver() {
            m_preset->setDirty(m_isDirty);
        }

    private:
        KisPaintOpPreset *m_preset;
        bool m_isDirty;
    };

    /**
     * @brief The UpdatedPostponer class
     * @see KisPaintopSettingsUpdateProxy::postponeSettingsChanges()
     */
    class KRITAIMAGE_EXPORT UpdatedPostponer{
    public:
        UpdatedPostponer(KisPaintOpPreset *preset);

        ~UpdatedPostponer();

    private:
        KisPaintopSettingsUpdateProxy *m_updateProxy;
    };

    void setOptionsWidget(KisPaintOpConfigWidget *widget);

    KisPaintopSettingsUpdateProxy* updateProxy() const;
    KisPaintopSettingsUpdateProxy* updateProxyNoCreate() const;

    QList<KisUniformPaintOpPropertySP> uniformProperties();

    /**
     * @return true if this preset demands a secondary masked brush running
     *         alongside it
     */
    bool hasMaskingPreset() const;

    /**
     * @return a newly created preset of the masked brush that should be run
     *         alongside the current brush
     */
    KisPaintOpPresetSP createMaskingPreset() const;


private:

    struct Private;
    Private * const m_d;
};

Q_DECLARE_METATYPE(KisPaintOpPresetSP)

#endif
