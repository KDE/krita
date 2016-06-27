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

class DlgAnimaterionRenderer: public KoDialog
{

    Q_OBJECT

public:

    DlgAnimaterionRenderer(KisImageWSP image, QWidget *parent = 0);
    ~DlgAnimaterionRenderer();

    KisPropertiesConfigurationSP getSequenceConfiguration() const;
    void setSequenceConfiguration(KisPropertiesConfigurationSP  cfg);

    KisPropertiesConfigurationSP getFrameExportConfiguration() const;

    bool renderToVideo() const;

    KisPropertiesConfigurationSP getVideoConfiguration() const;
    void setVideoConfiguration(KisPropertiesConfigurationSP cfg);

    KisPropertiesConfigurationSP getencoderConfiguration() const;
    void getencoderConfiguration(KisPropertiesConfigurationSP cfg);

private Q_SLOTS:

    void selectRenderType(int renderType);
    void toggleSequenceType(bool toggle);
    void sequenceMimeTypeSelected(int index);

private:

    KisImageWSP m_image;
    WdgAnimaterionRenderer *m_page;
    QList<QSharedPointer<KisImportExportFilter>> m_filters;
    QList<QWidget> m_configWidgets;
    QHBoxLayout *m_sequenceConfigLayout;
    KisConfigWidget *m_frameExportConfigurationWidget;
};

#endif // DLG_ANIMATIONRENDERERIMAGE
