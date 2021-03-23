/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef VIDEO_EXPORT_OPTIONS_DIALOG_H
#define VIDEO_EXPORT_OPTIONS_DIALOG_H

#include <kis_config_widget.h>
#include <kis_properties_configuration.h>

#include "KisVideoSaver.h"

#include <QScopedPointer>

namespace Ui {
class VideoExportOptionsDialog;
}

class KisVideoExportOptionsDialog : public KisConfigWidget
{
    Q_OBJECT

public:
    enum ContainerType {
        DEFAULT,
        OGV,
        WEBM,
        GIF,
        PNG,
        WEBP
    };

    enum CodecPageIndex {
        CODEC_H264 = 0,
        CODEC_H265,
        CODEC_THEORA,
        CODEC_VP9,
        CODEC_GIF,
        CODEC_APNG,
        CODEC_WEBP
        
    };

public:
    explicit KisVideoExportOptionsDialog(ContainerType containerType, QWidget *parent = 0);
    ~KisVideoExportOptionsDialog() override;

    void setSupportsHDR(bool value);

    QStringList customUserOptions() const;
    QString customUserOptionsString() const;
    bool videoConfiguredForHDR() const;
    void setHDRConfiguration(bool value);

    void setConfiguration(const KisPropertiesConfigurationSP  config) override;
    KisPropertiesConfigurationSP configuration() const override;

    static ContainerType mimeToContainer(const QString & mimeType);

private Q_SLOTS:
    void slotCustomLineToggled(bool value);
    void slotSaveCustomLine();
    void slotResetCustomLine();

    void slotCodecSelected(int index);

    void slotH265ProfileChanged(int index);
    void slotEditHDRMetadata();
    void slotBayerFilterSelected(int index);

private:
    Ui::VideoExportOptionsDialog *ui;

private:
    QStringList generateCustomLine() const;

    QString currentCodecId() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // VIDEO_EXPORT_OPTIONS_DIALOG_H
