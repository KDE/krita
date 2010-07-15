/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_color_selector_ng_docker_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "kis_canvas2.h"

#include "kis_color_patches.h"
#include "kis_common_colors.h"
#include "kis_color_selector_settings.h"
#include "kis_color_selector_container.h"

#include "ui_wdg_color_selector_settings.h"
#include "kis_color_space_selector.h"
#include "kis_preference_set_registry.h"
#include "kis_color_selector_settings.h"

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>

#include <KDebug>

KisColorSelectorNgDockerWidget::KisColorSelectorNgDockerWidget(QWidget *parent) :
    QWidget(parent), m_verticalColorPatchesLayout(0), m_horizontalColorPatchesLayout(0), m_canvas(0)
{
    setAutoFillBackground(true);

    m_colorSelectorContainer = new KisColorSelectorContainer(this);
    m_lastColorsWidget = new KisColorPatches(this);
    m_commonColorsWidget = new KisCommonColors(this);

    //default settings
    //remember to also change the default in the ui file

    //shade selector

    //layout
    m_verticalColorPatchesLayout = new QHBoxLayout();
    m_verticalColorPatchesLayout->setSpacing(0);
    m_verticalColorPatchesLayout->setMargin(0);
    m_verticalColorPatchesLayout->addWidget(m_colorSelectorContainer);

    m_horizontalColorPatchesLayout = new QVBoxLayout(this);
    m_horizontalColorPatchesLayout->setSpacing(0);
    m_horizontalColorPatchesLayout->setMargin(0);
    m_horizontalColorPatchesLayout->addLayout(m_verticalColorPatchesLayout);

    updateLayout();

    connect(m_colorSelectorContainer, SIGNAL(openSettings()), this, SLOT(openSettings()));

    //emit settingsChanged() if the settings are changed in krita preferences
    KisPreferenceSetRegistry *preferenceSetRegistry = KisPreferenceSetRegistry::instance();
    KisColorSelectorSettings* settings = dynamic_cast<KisColorSelectorSettings*>(preferenceSetRegistry->get("extendedColorSelectorSettings"));
    Q_ASSERT(settings);

    connect(settings, SIGNAL(settingsChanged()), this,                     SIGNAL(settingsChanged()));
    connect(this,     SIGNAL(settingsChanged()), this,                     SLOT(updateLayout()));
    connect(this,     SIGNAL(settingsChanged()), m_commonColorsWidget,     SLOT(updateSettings()));
    connect(this,     SIGNAL(settingsChanged()), m_lastColorsWidget,       SLOT(updateSettings()));
    connect(this,     SIGNAL(settingsChanged()), m_colorSelectorContainer, SIGNAL(settingsChanged()));

    emit settingsChanged();
}

void KisColorSelectorNgDockerWidget::setCanvas(KisCanvas2 *canvas)
{
    Q_ASSERT(canvas);
    m_commonColorsWidget->setCanvas(canvas);
    m_colorSelectorContainer->setCanvas(canvas);
    m_canvas = canvas;
}

void KisColorSelectorNgDockerWidget::openSettings()
{
    Q_ASSERT(m_canvas);

    KisColorSelectorSettingsDialog settings;
    if(settings.exec()==QDialog::Accepted) {
        emit settingsChanged();
    }
}


void KisColorSelectorNgDockerWidget::updateLayout()
{
    KConfigGroup cfg = KGlobal::config()->group("extendedColorSelector");


    //color patches
    bool m_lastColorsShow = cfg.readEntry("lastUsedColorsShow", true);
    KisColorPatches::Direction m_lastColorsDirection;
    if(cfg.readEntry("lastUsedColorsAlignment", false))
        m_lastColorsDirection=KisColorPatches::Vertical;
    else
        m_lastColorsDirection=KisColorPatches::Horizontal;

    bool m_commonColorsShow = cfg.readEntry("commonColorsShow", true);
    KisColorPatches::Direction m_commonColorsDirection;
    if(cfg.readEntry("commonColorsAlignment", false))
        m_commonColorsDirection=KisColorPatches::Vertical;
    else
        m_commonColorsDirection=KisColorPatches::Horizontal;


    m_verticalColorPatchesLayout->removeWidget(m_lastColorsWidget);
    m_verticalColorPatchesLayout->removeWidget(m_commonColorsWidget);
    m_horizontalColorPatchesLayout->removeWidget(m_lastColorsWidget);
    m_horizontalColorPatchesLayout->removeWidget(m_commonColorsWidget);

    if(m_lastColorsShow==false)
        m_lastColorsWidget->hide();
    else
        m_lastColorsWidget->show();

    if(m_commonColorsShow==false) {
        m_commonColorsWidget->hide();
    }
    else {
        m_commonColorsWidget->show();
    }

    if(m_lastColorsShow && m_lastColorsDirection==KisColorPatches::Vertical) {
        m_verticalColorPatchesLayout->addWidget(m_lastColorsWidget);
    }

    if(m_commonColorsShow && m_commonColorsDirection==KisColorPatches::Vertical) {
        m_verticalColorPatchesLayout->addWidget(m_commonColorsWidget);
    }

    if(m_lastColorsShow && m_lastColorsDirection==KisColorPatches::Horizontal) {
        m_horizontalColorPatchesLayout->addWidget(m_lastColorsWidget);
    }

    if(m_commonColorsShow && m_commonColorsDirection==KisColorPatches::Horizontal) {
        m_horizontalColorPatchesLayout->addWidget(m_commonColorsWidget);
    }

    updateGeometry();
}
