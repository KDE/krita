/*
 * Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
