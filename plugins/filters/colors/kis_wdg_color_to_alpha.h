/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_WDG_COLOR_TO_ALPHA_H_
#define _KIS_WDG_COLOR_TO_ALPHA_H_

#include <kis_config_widget.h>

class KoColor;
class Ui_WdgColorToAlphaBase;


class KisWdgColorToAlpha : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgColorToAlpha(QWidget * parent);
    ~KisWdgColorToAlpha() override;
    inline const Ui_WdgColorToAlphaBase* widget() const {
        return m_widget;
    }

    void setView(KisViewManager *view) override;

    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;

protected:

    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;

private Q_SLOTS:
    void slotFgColorChanged(const KoColor &color);
    void slotColorSelectorChanged(const KoColor &color);
    void slotCustomColorSelected(const KoColor &color);

private:
    Ui_WdgColorToAlphaBase* m_widget;
    KisViewManager *m_view;
};

#endif
