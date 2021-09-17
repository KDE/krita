/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LEVEL_CONFIG_WIDGET_H
#define KIS_LEVEL_CONFIG_WIDGET_H

#include <QPair>
#include <QPolygonF>

#include <kis_config_widget.h>
#include <KisLevelsCurve.h>

#include "../colorsfilters/virtual_channel_info.h"

#include "ui_KisLevelsConfigWidget.h"

class KisHistogram;
class QWidget;
class KisAutoLevelsWidget;

class KisLevelsConfigWidget : public KisConfigWidget
{
    Q_OBJECT

public:
    KisLevelsConfigWidget(QWidget * parent, KisPaintDeviceSP dev, const KoColorSpace* colorSpace);
    ~KisLevelsConfigWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;

private Q_SLOTS:
    void resetAll();
    void resetInputLevels();
    void resetOutputLevels();
    void resetAllChannels();
    void setActiveChannel(int ch);

    void slot_buttonGroupMode_buttonToggled(QAbstractButton *button, bool checked);
    void slot_comboBoxChannel_activated(int index);
    void slot_buttonGroupHistogramMode_buttonToggled(QAbstractButton *button, bool checked);
    void slot_spinBoxInputBlackPoint_valueChanged(int value);
    void slot_spinBoxInputWhitePoint_valueChanged(int value);
    void slot_spinBoxInputGamma_valueChanged(qreal value);
    void slot_spinBoxOutputBlackPoint_valueChanged(int value);
    void slot_spinBoxOutputWhitePoint_valueChanged(int value);
    void slot_sliderInputLevels_blackPointChanged(qreal value);
    void slot_sliderInputLevels_whitePointChanged(qreal value);
    void slot_sliderInputLevels_gammaChanged(qreal value);
    void slot_sliderOutputLevels_blackPointChanged(qreal value);
    void slot_sliderOutputLevels_whitePointChanged(qreal value);
    void slot_buttonAutoLevels_clicked();
    void slot_buttonAutoLevelsAllChannels_clicked();
    void slot_autoLevelsWidget_parametersChanged();
    void slot_autoLevelsWidgetAllChannels_parametersChanged();

    void computeChannelsMinMaxRanges();
    void updateWidgets();
    void updateHistograms();
    void updateHistogramViewChannels();
    void setButtonsIcons();

private:
    Ui::LevelsConfigWidget m_page;
    KisPaintDeviceSP m_dev;
    const KoColorSpace *m_colorSpace;
    QVector<VirtualChannelInfo> m_virtualChannels;
    QVector<QPair<int, int>> m_virtualChannelsMinMaxRanges;
    QPair<int, int> m_lightnessMinMaxRanges;
    int m_activeChannel;
    int m_activeChannelMin, m_activeChannelMax;
    QVector<KisLevelsCurve> m_levelsCurves;
    KisLevelsCurve m_lightnessLevelsCurve;
    KisLevelsCurve *m_activeLevelsCurve;
    QScopedPointer<KisHistogram> m_channelsHistogram;
    QScopedPointer<KisHistogram> m_lightnessHistogram;
    KisAutoLevelsWidget *m_autoLevelsWidget;

    bool event(QEvent *e) override;
};

#endif
