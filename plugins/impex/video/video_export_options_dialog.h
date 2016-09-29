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

#ifndef VIDEO_EXPORT_OPTIONS_DIALOG_H
#define VIDEO_EXPORT_OPTIONS_DIALOG_H

#include <kis_config_widget.h>
#include <kis_properties_configuration.h>

#include "video_saver.h"

#include <QScopedPointer>

namespace Ui {
class VideoExportOptionsDialog;
}

class VideoExportOptionsDialog : public KisConfigWidget
{
    Q_OBJECT

public:
    enum CodecIndex {
        CODEC_H264 = 0,
        CODEC_THEORA
    };

public:
    explicit VideoExportOptionsDialog(QWidget *parent = 0);
    ~VideoExportOptionsDialog();

    void setCodec(CodecIndex index);

    QStringList customUserOptions() const;

    void setConfiguration(const KisPropertiesConfigurationSP  config);
    KisPropertiesConfigurationSP configuration() const;

private Q_SLOTS:
    void slotCustomLineToggled(bool value);
    void slotSaveCustomLine();
    void slotResetCustomLine();

private:
    Ui::VideoExportOptionsDialog *ui;

private:
    QStringList generateCustomLine() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // VIDEO_EXPORT_OPTIONS_DIALOG_H
