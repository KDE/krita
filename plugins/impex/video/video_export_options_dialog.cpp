/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "video_export_options_dialog.h"
#include "ui_video_export_options_dialog.h"

#include <KoID.h>


struct VideoExportOptionsDialog::Private
{
    Private() {
        presets << KoID("ultrafast", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "ultrafast"));
        presets << KoID("superfast", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "superfast"));
        presets << KoID("veryfast", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "veryfast"));
        presets << KoID("faster", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "faster"));
        presets << KoID("fast", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "fast"));
        presets << KoID("medium", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "medium"));
        presets << KoID("slow", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "slow"));
        presets << KoID("slower", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "slower"));
        presets << KoID("veryslow", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "veryslow"));
        presets << KoID("placebo", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "placebo"));

        defaultPreset = 5;
        defaultBitrate = 5000;
        defaultConstantRateFactor = 23;
    }

    QVector<KoID> presets;
    int defaultPreset;
    int defaultBitrate;
    int defaultConstantRateFactor;
};


VideoExportOptionsDialog::VideoExportOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VideoExportOptionsDialog),
    m_d(new Private)
{
    ui->setupUi(this);

    ui->intConstantRateFactor->setRange(0, 51);
    ui->intConstantRateFactor->setValue(m_d->defaultConstantRateFactor);

    Q_FOREACH (const KoID &preset, m_d->presets) {
        ui->cmbPreset->insertItem(ui->cmbPreset->count(), preset.name());
    }
    ui->cmbPreset->setCurrentIndex(m_d->defaultPreset);

    ui->intBitrate->setRange(10, 50000);
    ui->intBitrate->setValue(5000);
    ui->intBitrate->setSuffix(i18nc("kilo-bits-per-second, video bitrate suffix", "kbps"));

    connect(ui->cmbCodec, SIGNAL(currentIndexChanged(int)),
            ui->stackedWidget, SLOT(setCurrentIndex(int)));

    ui->cmbCodec->setCurrentIndex(0);
    ui->cmbCodec->setEnabled(false);

    setModal(true);
}

VideoExportOptionsDialog::~VideoExportOptionsDialog()
{
    delete ui;
}

void VideoExportOptionsDialog::setCodec(CodecIndex index)
{
    ui->cmbCodec->setCurrentIndex(int(index));
}

VideoSaver::AdditionalOptions VideoExportOptionsDialog::getOptions() const
{
    VideoSaver::AdditionalOptions options;

    if (ui->cmbCodec->currentIndex() == int(CODEC_H264)) {
        options["crf"] = QString::number(ui->intConstantRateFactor->value());

        const int presetIndex = ui->cmbPreset->currentIndex();
        options["preset"] = m_d->presets[presetIndex].id();

    } else if (ui->cmbCodec->currentIndex() == int(CODEC_THEORA)) {
        const qint64 bitRate = ui->intBitrate->value() * 1024;
        options["bit_rate"] = bitRate;
    }

    return options;
}
