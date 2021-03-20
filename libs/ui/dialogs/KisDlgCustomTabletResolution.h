/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
