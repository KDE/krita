/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_dlg_paste_format.h"

#include <QPushButton>

KisDlgPasteFormat::KisDlgPasteFormat(QWidget *parent)
    : KoDialog(parent)
    , wdg(new Ui_WdgPasteFormat)
    , pasteSources()
{
    setObjectName("KisDlgPasteFormat");

    setWindowTitle(i18nc("@title:window", "Multiple Paste Sources Detected"));

    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    button(KoDialog::Ok)->setEnabled(false);
    auto *page = new QWidget(this);
    wdg->setupUi(page);
    setMainWidget(page);

    pasteSources.addButton(wdg->btnDownload, KisClipboard::PASTE_FORMAT_DOWNLOAD);
    pasteSources.addButton(wdg->btnUseLocal, KisClipboard::PASTE_FORMAT_LOCAL);
    pasteSources.addButton(wdg->btnUseBitmap, KisClipboard::PASTE_FORMAT_CLIP);
    connect(&pasteSources, qOverload<int>(&QButtonGroup::buttonClicked), this, &KisDlgPasteFormat::onInputChanged);

    wdg->btnDownload->setEnabled(false);
    wdg->btnUseLocal->setEnabled(false);
    wdg->btnUseBitmap->setEnabled(false);
}

KisClipboard::PasteFormatBehaviour KisDlgPasteFormat::source() const
{
    return static_cast<KisClipboard::PasteFormatBehaviour>(pasteSources.checkedId());
}

void KisDlgPasteFormat::onInputChanged()
{
    bool isValid = pasteSources.checkedId() >= 0;
    button(KoDialog::Ok)->setEnabled(isValid);
}

void KisDlgPasteFormat::setSourceAvailable(KisClipboard::PasteFormatBehaviour id, bool value)
{
    if (pasteSources.button(id)) {
        pasteSources.button(id)->setEnabled(value);
    }
}

bool KisDlgPasteFormat::remember() const
{
    return wdg->chkRemember->isChecked();
}
