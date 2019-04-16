/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISDLGCUSTOMTABLETRESOLUTION_H
#define KISDLGCUSTOMTABLETRESOLUTION_H

#include <QDialog>
#include "kritaui_export.h"

namespace Ui {
class KisDlgCustomTabletResolution;
}

class KRITAUI_EXPORT KisDlgCustomTabletResolution : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        USE_WINTAB = 0,
        USE_VIRTUAL_SCREEN,
        USE_CUSTOM
    };

    void accept();

public:
    explicit KisDlgCustomTabletResolution(QWidget *parent = 0);
    ~KisDlgCustomTabletResolution();

    static QRect calcNativeScreenRect();
    static Mode getTabletMode(QRect *customRect);

    static void applyConfiguration(Mode mode, const QRect &customRect);

private:
    Ui::KisDlgCustomTabletResolution *ui;
};

#endif // KISDLGCUSTOMTABLETRESOLUTION_H
