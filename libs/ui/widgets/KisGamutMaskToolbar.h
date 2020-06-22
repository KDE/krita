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

#ifndef KISGAMUTMASKTOOLBAR_H
#define KISGAMUTMASKTOOLBAR_H

#include <QWidget>
#include <QIcon>

#include <resources/KoGamutMask.h>
#include "kritaui_export.h"

#include "ui_wdgGamutMaskToolbar.h"

class KisCanvasResourceProvider;

class KRITAUI_EXPORT KisGamutMaskToolbar : public QWidget
{
    Q_OBJECT
public:
    KisGamutMaskToolbar(QWidget* parent = nullptr);
    void connectMaskSignals(KisCanvasResourceProvider* resourceProvider);

Q_SIGNALS:
    void sigGamutMaskToggle(bool state);
    void sigGamutMaskChanged(KoGamutMaskSP);
    void sigGamutMaskDeactivated();

public Q_SLOTS:
    void slotGamutMaskSet(KoGamutMaskSP mask);
    void slotGamutMaskUnset();
    void slotGamutMaskDeactivate();

private Q_SLOTS:
    void slotGamutMaskToggle(bool state);
    void slotGamutMaskRotate(int angle);

private:
    Ui_wdgGamutMaskToolbar* m_ui;
    KoGamutMaskSP m_selectedMask;

    QIcon m_iconMaskOff;
    QIcon m_iconMaskOn;

    QString m_textNoMask;
    QString m_textMaskDisabled;

    bool m_selfUpdate;
};

#endif // KISGAMUTMASKTOOLBAR_H
