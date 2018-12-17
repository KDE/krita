/*
 *  Copyright (c) 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
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

#include <QObject>
#include <QWidget>
#include <QDial>

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

Q_SIGNALS:
    void valueChanged(int);
    void configChanged();
    void globalAngleChanged(int);

private Q_SLOTS:
    void slotDialAngleChanged(int value);
    void slotIntAngleChanged(int value);
    void slotGlobalLightToggled();

private:
    void emitChangeSignals();

    Ui_WdgKisLayerStyleAngleSelector* ui;

    // BUG: 372169
    // Adobe's dial widget differs from QDial by 90 degrees,
    // therefore we need to apply this magic constant
    // to this widget's QDial for consistency between
    // the settings dialogs and on-canvas effects.
    // Wrapping is handled by QDial itself.
    const static int m_dialValueShift = 90;

    bool m_enableGlobalLight;
};

#endif // KISLAYERSTYLEANGLESELECTOR_H
