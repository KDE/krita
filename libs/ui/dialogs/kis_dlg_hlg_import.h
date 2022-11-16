/*
 *  SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DLG_HDR_IMPORT_H
#define KIS_DLG_HDR_IMPORT_H

#include <KoDialog.h>

#include "kritaui_export.h"

namespace Ui
{
class DlgHeifImport;
}

class KRITAUI_EXPORT KisDlgHLGImport : public KoDialog
{
    Q_OBJECT

public:
    explicit KisDlgHLGImport(bool applyOOTF, float gamma, float brightness, QWidget *parent = nullptr);
    bool applyOOTF();
    float gamma();
    float nominalPeakBrightness();
private Q_SLOTS:
    void toggleHLGOptions(bool toggle);

private:
    Ui::DlgHeifImport *ui;
};

#endif // KIS_DLG_HDR_IMPORT_H
