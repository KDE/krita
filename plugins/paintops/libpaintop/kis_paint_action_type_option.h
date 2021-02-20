/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_PAINT_ACTION_TYPE_OPTION_H
#define KIS_PAINT_ACTION_TYPE_OPTION_H

#include <kis_paintop_option.h>
#include <kritapaintop_export.h>

class KisPaintActionWidget;

enum enumPaintActionType {
    UNSUPPORTED,
    BUILDUP,
    WASH,
    FRINGED // not used yet
};

/**
 * Allows the user to choose between two types of paint action:
 * * incremental (going over the same spot in one stroke makes it darker)
 * * indirect (like photoshop and gimp)
 */
class PAINTOP_EXPORT KisPaintActionTypeOption : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisPaintActionTypeOption();

    ~KisPaintActionTypeOption() override;

    enumPaintActionType paintActionType() const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;

    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

public Q_SLOTS:
    void slotForceWashMode(bool value);

private:
    void updateControlsAvailability(bool value);

private:

    KisPaintActionWidget * m_optionWidget;

};

#endif
