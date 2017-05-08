/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
