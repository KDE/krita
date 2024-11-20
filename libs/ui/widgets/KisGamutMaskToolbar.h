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
    ~KisGamutMaskToolbar();
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
    void updateMaskState(bool maskEnabled, bool internalChange);

    struct Private;
    const QScopedPointer<Private> m_d;
    QScopedPointer<Ui_wdgGamutMaskToolbar> m_ui;
};

#endif // KISGAMUTMASKTOOLBAR_H
