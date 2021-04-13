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

class WdgAnimationRenderer : public QWidget, public Ui::WdgAnimaterionRenderer
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

    void frameRateChanged(int framerate);

protected Q_SLOTS:

    void slotButtonClicked(int button) override;
    void slotDialogAccepted();


private: 
    void initializeRenderSettings(const KisDocument &doc, const KisAnimationRenderingOptions &lastUsedOptions);

    static QString defaultVideoFileName(KisDocument *doc, const QString &mimeType);

    static void getDefaultVideoEncoderOptions(const QString &mimeType,
                                              KisPropertiesConfigurationSP cfg,
                                              QString *customFFMpegOptionsString,
                                              bool *forceHDRVideo);

    static void filterSequenceMimeTypes(QStringList &mimeTypes);
    static QStringList makeVideoMimeTypesList();
    static bool imageMimeSupportsHDR(QString &hdr);

    static KisPropertiesConfigurationSP loadLastConfiguration(QString configurationID);
    static void saveLastUsedConfiguration(QString configurationID, KisPropertiesConfigurationSP config);

private:

    static QString findFFMpeg(const QString &customLocation);

    KisImageSP m_image;
    KisDocument *m_doc;
    WdgAnimationRenderer *m_page {0};

    QString m_customFFMpegOptionsString;
    bool m_wantsRenderWithHDR = false;
};

#endif // DLG_ANIMATIONRENDERERIMAGE
