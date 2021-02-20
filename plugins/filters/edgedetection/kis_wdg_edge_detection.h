/*
 * SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_WDG_EDGE_DETECTION_H
#define KIS_WDG_EDGE_DETECTION_H

#include <QWidget>
#include <kis_config_widget.h>
#include "ui_wdg_edge_detection.h"

class Ui_WidgetEdgeDetection;


class KisWdgEdgeDetection : public KisConfigWidget
{
    Q_OBJECT

public:
    explicit KisWdgEdgeDetection(QWidget *parent);
    ~KisWdgEdgeDetection();

    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;

private:
    Ui_WidgetEdgeDetection *ui;
    QStringList m_types;
    QStringList m_types_translatable;
    QStringList m_output;
    QStringList m_output_translatable;

private Q_SLOTS:
    void horizontalRadiusChanged(qreal r);
    void verticalRadiusChanged(qreal r);
    void aspectLockChanged(bool v);
};

#endif // KIS_WDG_EDGE_DETECTION_H
