/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_AIRBRUSH_OPTION_H
#define KIS_AIRBRUSH_OPTION_H

#include <kis_paintop_option.h>
#include <kritapaintop_export.h>

/**
 * Allows the user to activate airbrushing of the brush mask (brush is painted at the same position over and over)
 * Rate is set in milliseconds.
 */
class PAINTOP_EXPORT KisAirbrushOptionWidget : public KisPaintOpOption
{
    Q_OBJECT

public:
    KisAirbrushOptionWidget(bool enabled = true, bool canIgnoreSpacing = true);
    ~KisAirbrushOptionWidget() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    /**
     * Returns the airbrushing interval, in milliseconds. This value should be ignored if the
     * KisAirbrushOption is not checked according to isChecked().
     */
    qreal airbrushInterval() const;

    /**
     * Returns true if regular distance-based spacing should be ignored and overridden by time-based
     * spacing. This value should be ignored if the KisAirbrushOption is not checked according to
     * isChecked().
     */
    bool ignoreSpacing() const;

private Q_SLOTS:
    void slotIntervalChanged();
    void slotIgnoreSpacingChanged();

private:
    // Reads the airbrush interval from the GUI.
    void updateInterval();
    // Reads the "ignore spacing" setting from the GUI.
    void updateIgnoreSpacing();

    struct Private;
    Private *const m_d;
};

class PAINTOP_EXPORT KisAirbrushOptionProperties : public KisPaintopPropertiesBase
{
public:
    bool enabled {false};
    qreal airbrushInterval {1000.0 / 20.0};
    bool ignoreSpacing {false};
protected:
    void readOptionSettingImpl(const KisPropertiesConfiguration *settings) override;
    void writeOptionSettingImpl(KisPropertiesConfiguration *settings) const override;
};


#endif
