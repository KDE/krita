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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_video_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QMessageBox>

#include <kpluginfactory.h>
#include <QFileInfo>
#include <QApplication>

#include <kis_debug.h>
#include <KisImportExportManager.h>
#include <KoColorSpaceConstants.h>
#include <KoDialog.h>
#include "KisPart.h"
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_config.h>

#include "video_saver.h"
#include "video_export_options_dialog.h"




K_PLUGIN_FACTORY_WITH_JSON(KisVideoExportFactory, "krita_video_export.json", registerPlugin<KisVideoExport>();)

KisVideoExport::KisVideoExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisVideoExport::~KisVideoExport()
{
}

KisImportExportFilter::ConversionStatus KisVideoExport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP configuration)
{
    QString ffmpegPath = configuration->getString("ffmpeg_path");
    if (ffmpegPath.isEmpty()) {
        KisConfig cfg(true);
        ffmpegPath = cfg.customFFMpegPath();
        if (ffmpegPath.isEmpty()) {
            const QString warningMessage =
                    i18n("Could not find \'ffmpeg\' binary. Saving to video formats is impossible.");

            if (!batchMode()) {
                QMessageBox::critical(KisPart::instance()->currentMainwindow(),
                                      i18n("Video Export Error"),
                                      warningMessage);
            }
            else {
                qWarning() << warningMessage;
            }
            document->setErrorMessage(warningMessage);
            return KisImportExportFilter::UsageError;
        }
    }

    VideoSaver videoSaver(document, ffmpegPath, batchMode());


    KisImageBuilder_Result res = videoSaver.encode(filename(), configuration);

    if (res == KisImageBuilder_RESULT_OK) {
        return KisImportExportFilter::OK;
    }
    else if (res == KisImageBuilder_RESULT_CANCEL) {
        return KisImportExportFilter::ProgressCancelled;
    }
    else {
        document->setErrorMessage(i18n("FFMpeg failed to convert the image sequence. Check the logfile in your output directory for more information."));
    }

    return KisImportExportFilter::InternalError;
}

KisPropertiesConfigurationSP KisVideoExport::defaultConfiguration(const QByteArray &from, const QByteArray &to) const
{
    Q_UNUSED(from);
    Q_ASSERT(!to.isEmpty());

    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());

    cfg->setProperty("h264PresetIndex", 5);
    cfg->setProperty("h264ConstantRateFactor", 23);
    // This was 4, 'high422', but Windows media player cannot play this profile, so
    // lets default to 'baseline' instead.
    cfg->setProperty("h264ProfileIndex", 0);
    cfg->setProperty("h264TuneIndex", 1);
    cfg->setProperty("TheoraBitrate", 5000);
    cfg->setProperty("CustomLineValue", "");

    if (to == "video/ogg") {
        cfg->setProperty("CodecIndex", VideoExportOptionsDialog::CODEC_THEORA);
    }
    else if (to == "video/x-matroska" || to == "video/mp4") {
        cfg->setProperty("CodecIndex", VideoExportOptionsDialog::CODEC_H264);
    }
    cfg->setProperty("mimetype", to);

    return cfg;
}

KisConfigWidget *KisVideoExport::createConfigurationWidget(QWidget *parent, const QByteArray &from, const QByteArray &to) const
{
    Q_UNUSED(from);
    KisConfigWidget *w = 0;
    if (to != "image/gif") {
        w = new VideoExportOptionsDialog(parent);
    }
    return w;
}

#include "kis_video_export.moc"
