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

#include <KoCanvasBase.h>

#include "kis_color_patches.h"
#include "kis_common_colors.h"
#include "kis_color_selector_settings.h"
#include "kis_color_selector_container.h"

#include "ui_wdg_color_selector_settings.h"
#include "kis_color_space_selector.h"

#include <KDebug>

KisColorSelectorNgDockerWidget::KisColorSelectorNgDockerWidget(QWidget *parent) :
    QWidget(parent), m_verticalColorPatchesLayout(0), m_horizontalColorPatchesLayout(0)
{
    setAutoFillBackground(true);

    m_colorSelectorContainer = new KisColorSelectorContainer(this);
    m_lastColorsWidget = new KisColorPatches(this);
    m_commonColorsWidget = new KisCommonColors(this);

    //default settings
    //remember to also change the default in the ui file

    //shade selector

    //color patches
    m_lastColorsShow=true;
    m_lastColorsDirection=KisColorPatches::Vertical;
    m_lastColorsScrolling=true;
    m_lastColorsColCount=2;
    m_lastColorsRowCount=3;

    m_commonColorsShow=true;
    m_commonColorsDirection=KisColorPatches::Horizontal;
    m_commonColorsScrolling=false;
    m_commonColorsColCount=2;
    m_commonColorsRowCount=3;

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
}

void KisColorSelectorNgDockerWidget::setCanvas(KoCanvasBase *canvas)
{
    m_commonColorsWidget->setCanvas(canvas);
    m_colorSelectorContainer->setCanvas(canvas);
}

void KisColorSelectorNgDockerWidget::openSettings()
{
    KisColorSelectorSettings settings;
    if(settings.exec()==QDialog::Accepted) {
        //general
        m_colorSelectorContainer->setShadeSelectorType(settings.ui->shadeSelectorType->currentIndex());
        m_colorSelectorContainer->setShadeSelectorHideable(settings.ui->shadeSelectorHideable->isChecked());
        m_colorSelectorContainer->setAllowHorizontalLayout(settings.ui->allowHorizontalLayout->isChecked());

        m_colorSelectorContainer->setPopupBehaviour(settings.ui->popupOnMouseOver->isChecked(), settings.ui->popupOnMouseClick->isChecked());
        m_colorSelectorContainer->setColorSpace(settings.ui->colorSpace->currentColorSpace());

        //color patches
        m_lastColorsShow = settings.ui->lastUsedColorsShow->isChecked();
        m_lastColorsDirection = settings.ui->lastUsedColorsAlignVertical->isChecked()?KisColorPatches::Vertical:KisColorPatches::Horizontal;
        m_lastColorsScrolling = settings.ui->lastUsedColorsAllowScrolling->isChecked();
        m_lastColorsColCount = settings.ui->lastUsedColorsNumCols->value();
        m_lastColorsRowCount = settings.ui->lastUsedColorsNumRows->value();

        m_commonColorsShow = settings.ui->commonColorsShow->isChecked();
        m_commonColorsDirection = settings.ui->commonColorsAlignVertical->isChecked()?KisColorPatches::Vertical:KisColorPatches::Horizontal;
        m_commonColorsScrolling = settings.ui->commonColorsAllowScrolling->isChecked();
        m_commonColorsColCount = settings.ui->commonColorsNumCols->value();
        m_commonColorsRowCount = settings.ui->commonColorsNumRows->value();

        updateLayout();
    }
}


void KisColorSelectorNgDockerWidget::updateLayout()
{
    //color patches
    m_lastColorsWidget->setPatchLayout(m_lastColorsDirection, m_lastColorsScrolling, m_lastColorsRowCount, m_lastColorsColCount);
    m_commonColorsWidget->setPatchLayout(m_commonColorsDirection, m_commonColorsScrolling, m_commonColorsRowCount, m_commonColorsColCount);

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
