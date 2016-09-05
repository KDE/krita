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
#include <kis_types.h>

class KisDocument;
class KisImportExportFilter;
class KisConfigWidget;
class QHBoxLayout;

class WdgAnimaterionRenderer : public QWidget, public Ui::WdgAnimaterionRenderer
{
    Q_OBJECT

public:
    WdgAnimaterionRenderer(QWidget *parent)
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
    ~DlgAnimationRenderer();

    KisPropertiesConfigurationSP getSequenceConfiguration() const;
    void setSequenceConfiguration(KisPropertiesConfigurationSP  cfg);

    KisPropertiesConfigurationSP getFrameExportConfiguration() const;

    bool renderToVideo() const;

    KisPropertiesConfigurationSP getVideoConfiguration() const;
    void setVideoConfiguration(KisPropertiesConfigurationSP cfg);

    KisPropertiesConfigurationSP getEncoderConfiguration() const;
    void setEncoderConfiguration(KisPropertiesConfigurationSP cfg);

    QSharedPointer<KisImportExportFilter> encoderFilter() const;

private Q_SLOTS:

    void selectRenderType();
    void toggleSequenceType(bool toggle);
    void sequenceMimeTypeSelected();
    void ffmpegLocationChanged(const QString&);

protected Q_SLOTS:

    void slotButtonClicked(int button);

private:

    KisImageWSP m_image;
    WdgAnimaterionRenderer *m_page {0};
    QList<QSharedPointer<KisImportExportFilter>> m_renderFilters;
    KisConfigWidget *m_encoderConfigWidget {0};
    KisConfigWidget *m_frameExportConfigWidget {0};
    QString m_defaultFileName;
};

#endif // DLG_ANIMATIONRENDERERIMAGE
