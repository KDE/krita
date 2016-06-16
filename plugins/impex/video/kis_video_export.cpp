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

#include <KisImportExportManager.h>
#include <KisFilterChain.h>
#include <KoColorSpaceConstants.h>

#include "KisPart.h"
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>

#include "video_saver.h"
#include "video_export_options_dialog.h"

#include "kis_cursor_override_hijacker.h"


K_PLUGIN_FACTORY_WITH_JSON(KisVideoExportFactory, "krita_video_export.json", registerPlugin<KisVideoExport>();)

KisVideoExport::KisVideoExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisVideoExport::~KisVideoExport()
{
}

KisImportExportFilter::ConversionStatus KisVideoExport::convert(const QByteArray& from, const QByteArray& to)
{
    Q_UNUSED(to);

    if (from != "application/x-krita")
        return KisImportExportFilter::NotImplemented;

    KisDocument* input = inputDocument();
    QString filename = outputFile();

    if (!input)
        return KisImportExportFilter::NoDocumentCreated;

    if (filename.isEmpty()) return KisImportExportFilter::FileNotFound;

    bool askForOptions = false;

    const QFileInfo fileInfo(filename);
    const QString suffix = fileInfo.suffix().toLower();

    VideoExportOptionsDialog::CodecIndex codecIndex =
        VideoExportOptionsDialog::CODEC_H264;

    if (suffix == "mkv" || suffix == "mp4") {
        codecIndex = VideoExportOptionsDialog::CODEC_H264;
        askForOptions = true;
    } else if (suffix == "ogv") {
        codecIndex = VideoExportOptionsDialog::CODEC_THEORA;
        askForOptions = true;
    }

    QStringList additionalOptionsList;

    askForOptions &=
        !qApp->applicationName().toLower().contains("test") &
        !getBatchMode();

    if (askForOptions) {
        KisCursorOverrideHijacker badGuy;

        VideoExportOptionsDialog dlg;
        dlg.setCodec(codecIndex);

        if (dlg.exec() == QDialog::Accepted) {
            additionalOptionsList = dlg.customUserOptions();
        } else {
            return KisImportExportFilter::UserCancelled;
        }
    }

    VideoSaver kpc(input, getBatchMode());

    if (!kpc.hasFFMpeg()) {
        const QString warningMessage =
            i18n("Could not find \'ffmpeg\' binary. Saving to video formats is impossible.");

        if (askForOptions) {
            QMessageBox::critical(KisPart::instance()->currentMainwindow(),
                                  i18n("Video Export Error"),
                                  warningMessage);
        } else {
            qWarning() << "WARNING:" << warningMessage;
        }

        return KisImportExportFilter::UsageError;
    }

    KisImageBuilder_Result res = kpc.encode(filename, additionalOptionsList);

    if (res == KisImageBuilder_RESULT_OK) {
        return KisImportExportFilter::OK;
    } else if (res == KisImageBuilder_RESULT_CANCEL) {
        return KisImportExportFilter::ProgressCancelled;
    }

    return KisImportExportFilter::InternalError;
}

#include "kis_video_export.moc"
