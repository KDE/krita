/*
 *  SPDX-FileCopyrightText: 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
    void slotGamutMaskRotate(qreal angle);

private:
    QScopedPointer<Ui_wdgGamutMaskToolbar> m_ui;
    KoGamutMaskSP m_selectedMask;

    QIcon m_iconMaskOff;
    QIcon m_iconMaskOn;

    QString m_textNoMask;
    QString m_textMaskDisabled;

    bool m_selfUpdate;
};

#endif // KISGAMUTMASKTOOLBAR_H
