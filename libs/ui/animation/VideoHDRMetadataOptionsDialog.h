/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef VIDEOHDRMETADATAOPTIONSDIALOG_H
#define VIDEOHDRMETADATAOPTIONSDIALOG_H

#include <KoDialog.h>

#include "kis_types.h"

struct KisHDRMetadataOptions;


namespace Ui {
class VideoHDRMetadataOptionsDialog;
} // namespace Ui

class VideoHDRMetadataOptionsDialog : public KoDialog
{
    Q_OBJECT

public:
    explicit VideoHDRMetadataOptionsDialog(QWidget *parent = nullptr);
    ~VideoHDRMetadataOptionsDialog() override;

    void setHDRMetadataOptions(const KisHDRMetadataOptions &options);
    KisHDRMetadataOptions hdrMetadataOptions() const;

private Q_SLOTS:
    void slotPredefinedDisplayIdChanged();

private:
    Ui::VideoHDRMetadataOptionsDialog *ui;
};

#endif // VIDEOHDRMETADATAOPTIONSDIALOG_H
