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

#include <QSharedPointer>
#include <QScopedPointer>
#include <kis_types.h>


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

    void selectRenderType(int i);
    void selectRenderOptions();
    /**
     * @brief sequenceMimeTypeSelected
     * calls the dialog for the export widget.
     */
    void sequenceMimeTypeOptionsClicked();

    void slotLockAspectRatioDimensionsWidth(int width);
    void slotLockAspectRatioDimensionsHeight(int height);

    void slotExportTypeChanged();
    void setFFmpegPath(const QString& path);

    void frameRateChanged(int framerate);

protected Q_SLOTS:

    void slotButtonClicked(int button) override;
    void slotDialogAccepted();


private: 
    void initializeRenderSettings(const KisDocument &doc, const KisAnimationRenderingOptions &lastUsedOptions);
    void ffmpegWarningCheck();
    bool validateFFmpeg(bool warn = false);

    static QString defaultVideoFileName(KisDocument *doc, const QString &mimeType);

    static void getDefaultVideoEncoderOptions(const QString &mimeType,
                                              KisPropertiesConfigurationSP cfg,
                                              const QStringList &availableEncoders,
                                              QString *customFFMpegOptionsString,
                                              bool *forceHDRVideo);

    static void filterSequenceMimeTypes(QStringList &mimeTypes);
    static QStringList makeVideoMimeTypesList();
    QStringList filterMimeTypeListByAvailableEncoders(const QStringList &mimeTypes);
    static bool imageMimeSupportsHDR(QString &hdr);

    static KisPropertiesConfigurationSP loadLastConfiguration(QString configurationID);
    static void saveLastUsedConfiguration(QString configurationID, KisPropertiesConfigurationSP config);

private:
    KisImageSP m_image;
    KisDocument *m_doc;

    QString m_customFFMpegOptionsString;
    QString ffmpegVersion = "None";

    QStringList ffmpegCodecs = QStringList(); // List of all supported output formats.
    QMap<QString, QStringList> ffmpegEncoderTypes; // Maps supported output format to available list of encoder(s)

    bool m_wantsRenderWithHDR = false;

    WdgAnimationRenderer *m_page {0};
};

#endif // DLG_ANIMATIONRENDERERIMAGE
