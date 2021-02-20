/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef VIDEOHDRMETADATAOPTIONSDIALOG_H
#define VIDEOHDRMETADATAOPTIONSDIALOG_H

#include <QDialog>

#include "kis_types.h"

struct KisHDRMetadataOptions;


namespace Ui {
class VideoHDRMetadataOptionsDialog;
}

class VideoHDRMetadataOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoHDRMetadataOptionsDialog(QWidget *parent = nullptr);
    ~VideoHDRMetadataOptionsDialog();

    void setHDRMetadataOptions(const KisHDRMetadataOptions &options);
    KisHDRMetadataOptions hdrMetadataOptions() const;

private Q_SLOTS:
    void slotPredefinedDisplayIdChanged();

private:
    Ui::VideoHDRMetadataOptionsDialog *ui;
};

#endif // VIDEOHDRMETADATAOPTIONSDIALOG_H
