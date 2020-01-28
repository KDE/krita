/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2006-09-13
 * @brief  LibRaw settings widgets
 *
 * @author Copyright (C) 2006-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2006-2011 by Marcel Wiesweg
 *         <a href="mailto:marcel dot wiesweg at gmx dot de">marcel dot wiesweg at gmx dot de</a>
 * @author Copyright (C) 2007-2008 by Guillaume Castagnino
 *         <a href="mailto:casta at xwing dot info">casta at xwing dot info</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DCRAW_SETTINGS_WIDGET_H
#define DCRAW_SETTINGS_WIDGET_H

// Qt includes

#include <QtCore/QString>

// KDE includes

#include <kconfiggroup.h>

// Local includes


#include "rawdecodingsettings.h"
#include "rexpanderbox.h"
#include "rwidgetutils.h"

namespace KDcrawIface
{

class  DcrawSettingsWidget : public RExpanderBox
{
    Q_OBJECT

public:

    enum AdvancedSettingsOptions
    {
        SIXTEENBITS      = 0x00000001,
        COLORSPACE       = 0x00000002,
        POSTPROCESSING   = 0x00000004,
        BLACKWHITEPOINTS = 0x00000008
    };

    enum SettingsTabs
    {
        DEMOSAICING = 0,
        WHITEBALANCE,
        CORRECTIONS,
        COLORMANAGEMENT
    };

public:

    /**
     * @param advSettings the default value is COLORSPACE
     */
    explicit DcrawSettingsWidget(QWidget* const parent, int advSettings = COLORSPACE);
    ~DcrawSettingsWidget() override;

    RFileSelector* inputProfileUrlEdit()  const;
    RFileSelector* outputProfileUrlEdit() const;

    void setup(int advSettings);

    void setEnabledBrightnessSettings(bool b);
    bool brightnessSettingsIsEnabled() const;

    void updateMinimumWidth();

    void resetToDefault();

    void setSettings(const RawDecodingSettings& settings);
    RawDecodingSettings settings() const;

    void readSettings(KConfigGroup& group) override;
    void writeSettings(KConfigGroup& group) override;

Q_SIGNALS:

    void signalSixteenBitsImageToggled(bool);
    void signalSettingsChanged();

private Q_SLOTS:

    void slotWhiteBalanceToggled(int);
    void slotsixteenBitsImageToggled(bool);
    void slotUnclipColorActivated(int);
    void slotNoiseReductionChanged(int);
    void slotCACorrectionToggled(bool);
    void slotExposureCorrectionToggled(bool);
    void slotAutoCAToggled(bool);
    void slotInputColorSpaceChanged(int);
    void slotOutputColorSpaceChanged(int);
    void slotRAWQualityChanged(int);
    void slotExpoCorrectionShiftChanged(double);

private:

    class Private;
    Private* const d;
};

} // NameSpace KDcrawIface

#endif /* DCRAW_SETTINGS_WIDGET_H */
