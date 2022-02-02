/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_DLG_PASTE_FORMAT_H
#define KIS_DLG_PASTE_FORMAT_H

#include <QButtonGroup>

#include "KoDialog.h"
#include "kis_clipboard.h"

#include "ui_wdgPasteFormat.h"

class KisDlgPasteFormat : public KoDialog
{
    Q_OBJECT
public:
    KisDlgPasteFormat(QWidget *parent = nullptr);

    KisClipboard::PasteFormatBehaviour source() const;

    void setSourceAvailable(KisClipboard::PasteFormatBehaviour id, bool value);

    bool remember() const;

private Q_SLOTS:
    void onInputChanged();

private:
    Ui_WdgPasteFormat *wdg;
    QButtonGroup pasteSources;
};

#endif // KIS_DLG_PASTE_FORMAT_H
