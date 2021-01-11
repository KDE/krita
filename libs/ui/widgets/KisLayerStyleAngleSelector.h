/*
 *  Copyright (c) 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISLAYERSTYLEANGLESELECTOR_H
#define KISLAYERSTYLEANGLESELECTOR_H

#include <QWidget>

#include <ui_wdgKisLayerStyleAngleSelector.h>

class KisLayerStyleAngleSelector : public QWidget
{
    Q_OBJECT

public:
    KisLayerStyleAngleSelector(QWidget* parent);

    int value();
    void setValue(int value);

    void enableGlobalLight(bool enable);

    bool useGlobalLight();
    void setUseGlobalLight(bool state);

    KisAngleSelector* angleSelector();

Q_SIGNALS:
    void valueChanged(int);
    void configChanged();
    void globalAngleChanged(int);

private Q_SLOTS:
    void slotAngleSelectorAngleChanged(qreal value);
    void slotGlobalLightToggled();

private:
    void emitChangeSignals();

    Ui_WdgKisLayerStyleAngleSelector* ui;
    
    bool m_enableGlobalLight;
};

#endif // KISLAYERSTYLEANGLESELECTOR_H
