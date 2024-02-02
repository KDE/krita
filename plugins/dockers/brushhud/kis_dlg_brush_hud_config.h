/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DLG_BRUSH_HUD_CONFIG_H
#define KIS_DLG_BRUSH_HUD_CONFIG_H

#include <QScopedPointer>
#include <QDialog>

#include "kis_types.h"


namespace Ui {
class KisDlgConfigureBrushHud;
}

class KisDlgConfigureBrushHud : public QDialog
{
    Q_OBJECT

public:
    explicit KisDlgConfigureBrushHud(KisPaintOpPresetSP preset, QWidget *parent = 0);
    ~KisDlgConfigureBrushHud() override;

private Q_SLOTS:
    void slotConfigAccepted();

    void slotMoveRight();
    void slotMoveLeft();
    void slotMoveUp();
    void slotMoveDown();

private:
    Ui::KisDlgConfigureBrushHud *ui;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_DLG_BRUSH_HUD_CONFIG_H
