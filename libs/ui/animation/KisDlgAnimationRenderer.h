/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLG_ANIMATIONRENDERERIMAGE
#define DLG_ANIMATIONRENDERERIMAGE

#include <KoDialog.h>
#include <kis_properties_configuration.h>

#include "ui_wdg_animationrenderer.h"

#include <kis_types.h>

#ifdef Q_OS_ANDROID
#include <QVariantMap>
#endif

#include "kritaui_export.h"

class KisDocument;
class KisImportExportFilter;
class KisConfigWidget;
class QHBoxLayout;
class KisAnimationVideoSaver;
class KisAnimationRenderingOptions;

class WdgAnimationRenderer : public QWidget, public Ui::WdgAnimationRenderer
{
    Q_OBJECT

public:
    WdgAnimationRenderer(QWidget *parent)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

class KRITAUI_EXPORT KisDlgAnimationRenderer: public KoDialog
{

    Q_OBJECT

public:

    KisDlgAnimationRenderer(KisDocument *doc, QWidget *parent = 0);
    ~KisDlgAnimationRenderer() override;

    KisAnimationRenderingOptions getEncoderOptions() const;

private Q_SLOTS:
#ifndef Q_OS_ANDROID
    void selectRenderType(int i);
#endif
    void selectRenderOptions();
    /**
     * @brief sequenceMimeTypeSelected
     * calls the dialog for the export widget.
     */
    void sequenceMimeTypeOptionsClicked();

    void slotLockAspectRatioDimensionsWidth(int width);
    void slotLockAspectRatioDimensionsHeight(int height);

    void slotExportTypeChanged();
    void slotRenderTypeChanged();
#ifndef Q_OS_ANDROID
    void setFFmpegPath(const QString& path);
#endif

    void slotCheckWarnings();

protected Q_SLOTS:
#ifndef Q_OS_ANDROID
    void slotButtonClicked(int button) override;
#endif
    void slotDialogAccepted();


private: 
#ifndef Q_OS_ANDROID
    enum FFmpegValidationResult {
        VALID = 1,
        INVALID = 0,
        NOT_A_BINARY = -1,
        COMPRESSED_FORMAT = -2
    };
#endif

    void initializeRenderSettings(const KisDocument &doc, const KisAnimationRenderingOptions &lastUsedOptions);

    void updateWarnings();

#ifndef Q_OS_ANDROID
    FFmpegValidationResult validateFFmpeg(const QString &ffmpegPath);

    static QString defaultVideoFileName(KisDocument *doc, const QString &mimeType);

    static void getDefaultVideoEncoderOptions(const QString &mimeType,
                                              KisPropertiesConfigurationSP cfg,
                                              const QStringList &availableEncoders,
                                              QString *customFFMpegOptionsString,
                                              bool *forceHDRVideo);
#endif

    static void filterSequenceMimeTypes(QStringList &mimeTypes);

#ifndef Q_OS_ANDROID
    static QStringList makeVideoMimeTypesList();
    QStringList filterMimeTypeListByAvailableEncoders(const QStringList &mimeTypes);
#endif
    static bool imageMimeSupportsHDR(QString &hdr);

#ifdef Q_OS_ANDROID
    static QVariantMap loadVideoFormatPreferences();
    static void saveVideoFormatPreferences(const QVariantMap &value);
#endif

    static KisPropertiesConfigurationSP loadLastConfiguration(QString configurationID);
    static void saveLastUsedConfiguration(QString configurationID, KisPropertiesConfigurationSP config);

    static bool looksLikeGif(const QString &videoType);
    static bool supportsAudio(const QString &videoType);

private:
    KisImageSP m_image;
    KisDocument *m_doc;

#ifdef Q_OS_ANDROID
    QString m_videoFileName;
    QVariantMap m_videoFormatPreferences;
#else
    QString m_customFFMpegOptionsString;
    QString ffmpegVersion = "None";

    QStringList ffmpegCodecs = QStringList(); // List of all supported output formats.
    QMap<QString, QStringList> ffmpegEncoderTypes; // Maps supported output format to available list of encoder(s)

    bool m_wantsRenderWithHDR = false;
#endif

    WdgAnimationRenderer *m_page {0};
};

#endif // DLG_ANIMATIONRENDERERIMAGE
