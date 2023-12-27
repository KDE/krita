/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_BRUSH_OPTION_H
#define KIS_BRUSH_OPTION_H

#include "kis_paintop_option.h"
#include "kis_brush_option.h"
#include <kritapaintop_export.h>
#include "kis_brush.h"
#include <KisBrushOptionWidgetFlags.h>
#include <lager/reader.hpp>
#include <KisBrushModel.h>

class KisBrushSelectionWidget;

/**
 * The brush option allows the user to select a particular brush
 * footprint for suitable paintops
 */
class PAINTOP_EXPORT KisBrushOptionWidget : public KisPaintOpOption
{
    Q_OBJECT
public:

    KisBrushOptionWidget(KisBrushOptionWidgetFlags flags);

    ~KisBrushOptionWidget() override;

    /**
     * @return the currently selected brush
     */
    KisBrushSP brush() const;

    void setImage(KisImageWSP image) override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void hideOptions(const QStringList &options);

    lager::reader<bool> lightnessModeEnabled() const;
    lager::reader<qreal> effectiveBrushSize() const;
    lager::reader<KisBrushModel::BrushData> bakedBrushData() const;

protected:
    OptionalLodLimitationsReader lodLimitationsReader() const override;

private:

    KisBrushSelectionWidget * m_brushSelectionWidget;

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
