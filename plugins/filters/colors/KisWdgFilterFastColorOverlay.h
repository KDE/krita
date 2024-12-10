/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISWDGFILTERFASTCOLOROVERLAY_H
#define KISWDGFILTERFASTCOLOROVERLAY_H

#include <kis_config_widget.h>

class Ui_WdgFilterFastColorOverlay;

class KisWdgFilterFastColorOverlay : public KisConfigWidget
{
    Q_OBJECT

public:
    explicit KisWdgFilterFastColorOverlay(QWidget *parent);
    ~KisWdgFilterFastColorOverlay();

    void setView(KisViewManager *view) override;

    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;

private:
    QScopedPointer<Ui_WdgFilterFastColorOverlay> m_widget;
    KisViewManager *m_view;
};

#endif // KISWDGFILTERFASTCOLOROVERLAY_H
