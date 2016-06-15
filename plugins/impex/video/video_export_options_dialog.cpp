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

#include <ksharedconfig.h>
#include <kconfiggroup.h>


struct VideoExportOptionsDialog::Private
{
    Private() {
        presets << KoID("ultrafast", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "ultrafast"));
        presets << KoID("superfast", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "superfast"));
        presets << KoID("veryfast", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "veryfast"));
        presets << KoID("faster", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "faster"));
        presets << KoID("fast", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "fast"));
        presets << KoID("medium", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "medium"));
        presets << KoID("slow", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "slow"));
        presets << KoID("slower", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "slower"));
        presets << KoID("veryslow", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "veryslow"));
        presets << KoID("placebo", i18nc("h264 preset name, check simplescreenrecorder for standard translations", "placebo"));

        profiles << KoID("baseline", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "baseline"));
        profiles << KoID("main", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "main"));
        profiles << KoID("high", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "high"));
        profiles << KoID("high10", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "high10"));
        profiles << KoID("high422", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "high422"));
        profiles << KoID("high444", i18nc("h264 profile name, check simplescreenrecorder for standard translations", "high444"));

        tunes << KoID("film", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "film"));
        tunes << KoID("animation", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "animation"));
        tunes << KoID("grain", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "grain"));
        tunes << KoID("stillimage", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "stillimage"));
        tunes << KoID("psnr", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "psnr"));
        tunes << KoID("ssim", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "ssim"));
        tunes << KoID("fastdecode", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "fastdecode"));
        tunes << KoID("zerolatency", i18nc("h264 tune option name, check simplescreenrecorder for standard translations", "zerolatency"));


        KConfigGroup cfg = KSharedConfig::openConfig()->group("VideoExportPlugin");

        defaultPreset = cfg.readEntry("h264PresetIndex", 5);
        defaultConstantRateFactor = cfg.readEntry("h264ConstantRateFactor", 23);
        defaultProfile = cfg.readEntry("h264ProfileIndex", 4);
        defaultTune = cfg.readEntry("h264TuneIndex", 1);
        defaultBitrate = cfg.readEntry("TheoraBitrate", 5000);
        defaultCustomLine = cfg.readEntry("CustomLineValue", "");
    }

    void updateDefaultCustomLine() {
        KConfigGroup cfg = KSharedConfig::openConfig()->group("VideoExportPlugin");
        defaultCustomLine = cfg.readEntry("CustomLineValue", "");
    }

    QVector<KoID> presets;
    int defaultPreset;
    int defaultBitrate;
    int defaultConstantRateFactor;

    QVector<KoID> profiles;
    int defaultProfile;

    QVector<KoID> tunes;
    int defaultTune;

    QString defaultCustomLine;
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

    Q_FOREACH (const KoID &profile, m_d->profiles) {
        ui->cmbProfile->insertItem(ui->cmbProfile->count(), profile.name());
    }
    ui->cmbProfile->setCurrentIndex(m_d->defaultProfile);

    Q_FOREACH (const KoID &tune, m_d->tunes) {
        ui->cmbTune->insertItem(ui->cmbTune->count(), tune.name());
    }
    ui->cmbTune->setCurrentIndex(m_d->defaultTune);

    ui->intBitrate->setRange(10, 50000);
    ui->intBitrate->setValue(5000);
    ui->intBitrate->setSuffix(i18nc("kilo-bits-per-second, video bitrate suffix", "kbps"));

    connect(ui->cmbCodec, SIGNAL(currentIndexChanged(int)),
            ui->stackedWidget, SLOT(setCurrentIndex(int)));

    ui->cmbCodec->setCurrentIndex(0);
    ui->cmbCodec->setEnabled(false);

    // TODO: temporarily hidden! Some combinations of 'tune' and
    //       'profile' options make ffmpeg generate empty file.
    //       We should not let the user shoot into his own foot!
    ui->cmbTune->setVisible(false);
    ui->lblTune->setVisible(false);

    setModal(true);
    connect(this, SIGNAL(accepted()), SLOT(slotAccepted()));

    ui->chkCustomLine->setChecked(!m_d->defaultCustomLine.isEmpty());
    slotCustomLineToggled(!m_d->defaultCustomLine.isEmpty());
    connect(ui->chkCustomLine, SIGNAL(toggled(bool)), SLOT(slotCustomLineToggled(bool)));

    connect(ui->txtCustomLine, SIGNAL(editingFinished()), SLOT(slotSaveCustomLine()));
    connect(ui->btnResetCustomLine, SIGNAL(clicked()), SLOT(slotResetCustomLine()));
}

VideoExportOptionsDialog::~VideoExportOptionsDialog()
{
    delete ui;
}

void VideoExportOptionsDialog::slotAccepted()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("VideoExportPlugin");

    cfg.writeEntry("h264PresetIndex", ui->cmbPreset->currentIndex());
    cfg.writeEntry("h264ConstantRateFactor", ui->intConstantRateFactor->value());
    cfg.writeEntry("h264ProfileIndex", ui->cmbProfile->currentIndex());
    cfg.writeEntry("h264TuneIndex", ui->cmbTune->currentIndex());
    cfg.writeEntry("TheoraBitrate", ui->intBitrate->value());
    slotSaveCustomLine();
}

void VideoExportOptionsDialog::slotCustomLineToggled(bool value)
{
    m_d->updateDefaultCustomLine();
    QString customLine = m_d->defaultCustomLine;

    if (customLine.isEmpty() && value) {
        customLine = generateCustomLine().join(" ");
    } else if (!value) {
        customLine = "";
    }

    ui->txtCustomLine->setText(customLine);

    ui->stackedWidget->setEnabled(!value);
    ui->txtCustomLine->setEnabled(value);
    ui->btnResetCustomLine->setEnabled(value);
}

void VideoExportOptionsDialog::slotSaveCustomLine()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("VideoExportPlugin");
    cfg.writeEntry("CustomLineValue", ui->txtCustomLine->text());
}

void VideoExportOptionsDialog::slotResetCustomLine()
{
    ui->txtCustomLine->setText(generateCustomLine().join(" "));
    slotSaveCustomLine();
}

void VideoExportOptionsDialog::setCodec(CodecIndex index)
{
    ui->cmbCodec->setCurrentIndex(int(index));
}

QStringList VideoExportOptionsDialog::customUserOptions() const
{
    return ui->chkCustomLine->isChecked() ?
        ui->txtCustomLine->text().split(" ", QString::SkipEmptyParts) :
        generateCustomLine();
}

QStringList VideoExportOptionsDialog::generateCustomLine() const
{
    QStringList options;

    if (ui->cmbCodec->currentIndex() == int(CODEC_H264)) {
        options << "-crf" << QString::number(ui->intConstantRateFactor->value());

        const int presetIndex = ui->cmbPreset->currentIndex();
        options << "-preset" << m_d->presets[presetIndex].id();

        const int profileIndex = ui->cmbProfile->currentIndex();
        options << "-profile" << m_d->profiles[profileIndex].id();

        if (m_d->profiles[profileIndex].id() == "high422") {
            options << "-pix_fmt" << "yuv422p";
        } else if (m_d->profiles[profileIndex].id() == "high444") {
            options << "-pix_fmt" << "yuv444p";
        } else {
            options << "-pix_fmt" << "yuv420p";
        }

        // Disabled! see the comment in c-tor!
        //const int tuneIndex = ui->cmbTune->currentIndex();
        //options << "-tune" << m_d->tunes[tuneIndex].id();

    } else if (ui->cmbCodec->currentIndex() == int(CODEC_THEORA)) {
        options << "-b" << QString::number(ui->intBitrate->value()) + "k";
    }

    return options;
}
