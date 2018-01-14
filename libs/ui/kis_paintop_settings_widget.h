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
#ifndef KIS_PAINTOP_SETTINGS_WIDGET_H
#define KIS_PAINTOP_SETTINGS_WIDGET_H

#include <kritaui_export.h>
#include "kis_paintop_option.h"
#include <brushengine/kis_paintop_config_widget.h>

class KisPropertiesConfiguration;
class KisPaintOpConfigWidget;
class KisPaintopLodLimitations;

/**
 * A common widget for enabling/disabling and determining
 * the effect of tablet pressure, tilt and rotation and
 * other paintop settings.
 */
class KRITAUI_EXPORT KisPaintOpSettingsWidget : public KisPaintOpConfigWidget
{
    Q_OBJECT

public:

    KisPaintOpSettingsWidget(QWidget * parent = 0);

    ~KisPaintOpSettingsWidget() override;

    void addPaintOpOption(KisPaintOpOption * option, const QString &label);
    void addPaintOpOption(KisPaintOpOption * option, const QString &label, KisPaintOpOption::PaintopCategory category);

    /// Reimplemented
    void setConfiguration(const KisPropertiesConfigurationSP  config) override;

    /// Reimplemented
    void writeConfiguration(KisPropertiesConfigurationSP config) const override;

    KisPaintopLodLimitations lodLimitations() const override;

    ///Reimplemented, sets image on option widgets
    void setImage(KisImageWSP image) override;

    ///Reimplemented, sets node on option widgets
    void setNode(KisNodeWSP node) override;

private Q_SLOTS:

    void changePage(const QModelIndex&);
    void lockProperties(const QModelIndex& index);
    void slotLockPropertiesDrop();
    void slotLockPropertiesSave();
    void slotEntryChecked(const QModelIndex &index);

protected:
    virtual void notifyPageChanged();

private:
    
    struct Private;
    Private* const m_d;
    bool m_saveLockedOption;

};

#endif
