/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_FILTER_OPTION_H
#define KIS_FILTER_OPTION_H

#include "kis_paintop_option.h"
#include <kis_types.h>
#include <kritapaintop_export.h>

class QGridLayout;
class KoID;
class KisConfigWidget;
class KisFilterConfiguration;
class KisFilterOptionWidget;
class KisPaintopLodLimitations;


const QString FILTER_ID = "Filter/id";
const QString FILTER_SMUDGE_MODE = "Filter/smudgeMode";
const QString FILTER_CONFIGURATION = "Filter/configuration";

/**
 * The filter option allows the user to select a particular filter
 * that can be applied by the paintop to the brush footprint or the
 * original paint device data.
 */
class PAINTOP_EXPORT KisFilterOption : public KisPaintOpOption
{
    Q_OBJECT

public:

    KisFilterOption();
    ~KisFilterOption() override;

    /**
     * Return the currently selected filter
     */
    const KisFilterSP filter() const;

    /**
     * Return the currently selected filter configuration
     */
    KisFilterConfigurationSP filterConfig() const;

    bool smudgeMode() const;

    /**
     * XXX
     */
    void setNode(KisNodeWSP node) override;

    /**
     * XXX
     */
    void setImage(KisImageWSP image) override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;

    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void lodLimitations(KisPaintopLodLimitations *l) const override;

private Q_SLOTS:

    void setCurrentFilter(const KoID&);

    void updateFilterConfigWidget();

private:

    QGridLayout *m_layout {0};
    KisFilterOptionWidget *m_filterOptionWidget {0};
    KisFilterSP m_currentFilter;
    KisConfigWidget *m_currentFilterConfigWidget {0};
    KisPaintDeviceSP m_paintDevice;
    KisImageSP m_image;
};

#endif
