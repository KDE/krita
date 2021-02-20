/*
 * SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_WDG_ASCCDL_H
#define KIS_WDG_ASCCDL_H
#include <kis_config_widget.h>
#include <QWidget>
#include "ui_wdg_asccdl.h"
#include <KisVisualRectangleSelectorShape.h>
#include <KisVisualEllipticalSelectorShape.h>

class Ui_WdgASCCDL;

/**
 * @brief The KisASCCDLConfigWidget class
 * this handles the configuration widget for the slope offset power filter.
 *
 * Future improvements:
 * 1. Have the cs that the widgets gets when being created be actually the image cs.
 * 2. Have the shape be force to a HSV wheel with a slider.
 * 3. Make it easier to select power higher than 1.0 (it is possible, but cumbersome)
 * 4. make it easier to access ocio from filters.
 * 5. Implement saturation whenever we can figure out what the formula is for that.
 * 6. Implement a way to retrieve and store xml data according to the asc-cdl spec...
 *
 * The main problem for 5 and 6 is that I am unable to find the actual asc-cdl spec.
 */

class KisASCCDLConfigWidget : public KisConfigWidget
{

    Q_OBJECT

public:
    KisASCCDLConfigWidget(QWidget * parent, const KoColorSpace *cs);
    ~KisASCCDLConfigWidget() override;

    KisPropertiesConfigurationSP  configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    Ui_WdgASCCDL *m_page;
    const KoColorSpace *m_cs;
public Q_SLOTS:
    void slopeColorChanged(const KoColor &c);
    void offsetColorChanged(const KoColor &c);
    void powerColorChanged(const KoColor &c);
};

#endif //KIS_WDG_ASCCDL_H
