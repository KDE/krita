/*
 *  SPDX-FileCopyrightText: 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_WDG_OPTIONS_HEIGHTMAP_H_
#define _KIS_WDG_OPTIONS_HEIGHTMAP_H_

#include <kis_config_widget.h>
#include "ui_kis_wdg_options_heightmap.h"

class KisWdgOptionsHeightmap : public KisConfigWidget, public Ui::WdgOptionsHeightMap
{
    Q_OBJECT

public:
    explicit KisWdgOptionsHeightmap(QWidget *parent);
    explicit KisWdgOptionsHeightmap(QWidget *parent, bool export_mode);

Q_SIGNALS:
    void statusUpdated(bool status);

protected:
    void showEvent(QShowEvent *event) override;
    void setConfiguration(const KisPropertiesConfigurationSP  cfg) override;
    KisPropertiesConfigurationSP configuration() const override;

private Q_SLOTS:
    void guessDimensions();
    void widthChanged(int i);
    void heightChanged(int i);

private:
    void updateStatus();
    bool m_exportMode;
};

#endif // _KIS_WDG_OPTIONS_HEIGHTMAP_H_
