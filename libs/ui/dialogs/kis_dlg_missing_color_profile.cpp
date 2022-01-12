/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_dlg_missing_color_profile.h"

#include <QPushButton>

KisDlgMissingColorProfile::KisDlgMissingColorProfile(QWidget *parent)
    : KoDialog(parent)
    , wdg()
    , colorProfiles()
{
    setObjectName("KisDlgMissingColorProfile");

    setWindowTitle(i18nc("@title:window", "Missing Color Profile"));

    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    button(KoDialog::Ok)->setEnabled(false);
    auto *page = new QWidget(this);
    wdg.setupUi(page);
    setMainWidget(page);

    colorProfiles.addButton(wdg.btnAsMonitor, KisClipboard::PASTE_ASSUME_MONITOR);
    colorProfiles.addButton(wdg.btnAsWeb, KisClipboard::PASTE_ASSUME_WEB);
    connect(&colorProfiles,
            qOverload<int>(&QButtonGroup::buttonClicked),
            this,
            &KisDlgMissingColorProfile::onInputChanged);
}

KisClipboard::PasteBehaviour KisDlgMissingColorProfile::source() const
{
    return static_cast<KisClipboard::PasteBehaviour>(colorProfiles.checkedId());
}

bool KisDlgMissingColorProfile::remember() const
{
    return wdg.dontPrompt->isChecked();
}

void KisDlgMissingColorProfile::onInputChanged()
{
    bool isValid = colorProfiles.checkedId() >= 0;
    button(KoDialog::Ok)->setEnabled(isValid);
}
