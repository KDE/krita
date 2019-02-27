/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef DLG_ANIMATIONRENDERERIMAGE
#define DLG_ANIMATIONRENDERERIMAGE

#include <KoDialog.h>
#include <kis_properties_configuration.h>

#include "ui_wdg_animationrenderer.h"

#include <QSharedPointer>
#include <QScopedPointer>
#include <kis_types.h>

class KisDocument;
class KisImportExportFilter;
class KisConfigWidget;
class QHBoxLayout;
class VideoSaver;
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

class DlgAnimationRenderer: public KoDialog
{

    Q_OBJECT

public:

    DlgAnimationRenderer(KisDocument *doc, QWidget *parent = 0);
    ~DlgAnimationRenderer() override;

    KisAnimationRenderingOptions getEncoderOptions() const;

private Q_SLOTS:

    void selectRenderType(int i);
    void selectRenderOptions();
    /**
     * @brief sequenceMimeTypeSelected
     * calls the dialog for the export widget.
     */
    void sequenceMimeTypeSelected();

    void slotLockAspectRatioDimensionsWidth(int width);
    void slotLockAspectRatioDimensionsHeight(int height);

    void slotExportTypeChanged();


protected Q_SLOTS:

    void slotButtonClicked(int button) override;
    void slotDialogAccepted();

private:
    void loadAnimationOptions(const KisAnimationRenderingOptions &options);

    static QString defaultVideoFileName(KisDocument *doc, const QString &mimeType);

    static void getDefaultVideoEncoderOptions(const QString &mimeType,
                                              KisPropertiesConfigurationSP cfg,
                                              QString *customFFMpegOptionsString,
                                              bool *forceHDRVideo);

private:

    static QString findFFMpeg(const QString &customLocation);

    KisImageSP m_image;
    KisDocument *m_doc;
    WdgAnimationRenderer *m_page {0};
    KisPropertiesConfigurationSP m_frameExportConfig;

    QString m_customFFMpegOptionsString;
    bool m_forceHDRVideo = false;
};

#endif // DLG_ANIMATIONRENDERERIMAGE
