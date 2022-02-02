/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_DLG_MISSING_COLOR_PROFILE_H
#define KIS_DLG_MISSING_COLOR_PROFILE_H

#include <QButtonGroup>

#include "KoDialog.h"
#include "kis_clipboard.h"

#include "ui_wdgMissingColorProfile.h"

class KisDlgMissingColorProfile : public KoDialog
{
    Q_OBJECT
public:
    KisDlgMissingColorProfile(QWidget *parent);

    KisClipboard::PasteBehaviour source() const;
    bool remember() const;

private Q_SLOTS:
    void onInputChanged();

private:
    Ui_WdgMissingColorProfile wdg;
    QButtonGroup colorProfiles;
};

#endif // KIS_DLG_PASTE_FORMAT_H
