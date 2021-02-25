/*
 *  SPDX-FileCopyrightText: 2020 Wolthera van Hövell tot Westerflier
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLGHEIFIMPORT_H
#define DLGHEIFIMPORT_H

#include <KoDialog.h>

namespace Ui {
class DlgHeifImport;
}

class DlgHeifImport : public KoDialog
{
    Q_OBJECT

public:
    explicit DlgHeifImport(bool applyootf, float gamma, float brightness, QWidget *parent = nullptr);
    bool applyOOTF();
    float gamma();
    float nominalPeakBrightness();
private Q_SLOTS:
    void toggleHLGOptions(bool toggle);
private:
    Ui::DlgHeifImport *ui;

};

#endif // DLGHEIFIMPORT_H
