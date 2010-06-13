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

#include "kis_colselng_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <KoCanvasBase.h>

#include "kis_colselng_my_paint_shade_selector.h"

#include "kis_colselng_bar.h"
#include "kis_colselng_my_paint_shade_selector.h"
#include "kis_colselng_color_selector.h"
#include "kis_colselng_color_patches.h"
#include "kis_colselng_shade_selector.h"
#include "kis_colselng_common_colors.h"
#include "kis_colselng_settings.h"

#include "ui_kis_colselng_settings.h"

KisColSelNgWidget::KisColSelNgWidget(QWidget *parent) :
    QWidget(parent), m_bigWidgetsLayout(0), m_horizontalColorPatchesLayout(0), m_verticalColorPathcesLayout(0)
{
    setMinimumHeight(50);
    m_barWidget = new KisColSelNgBar(this);
    m_colorSelectorWidget = new KisColSelNgColorSelector(this);
    m_myPaintShadeWidget = new KisColSelNgMyPaintShadeSelector(this);
    m_shadeSelectionWidget = new KisColSelNgShadeSelector(this);
    m_lastColorsWidget = new KisColSelNgColorPatches(this);
    m_commonColorsWidget = new KisColSelNgCommonColors(this);


    m_lastColorsShow=true;
    m_lastColorsDirection=KisColSelNgColorPatches::Vertical;
    m_lastColorsScrolling=true;
    m_lastColorsColCount=2;
    m_lastColorsRowCount=3;

    m_commonColorsShow=true;
    m_commonColorsDirection=KisColSelNgColorPatches::Horizontal;
    m_commonColorsScrolling=false;
    m_commonColorsColCount=2;
    m_commonColorsRowCount=3;


    m_bigWidgetsLayout = new QVBoxLayout();
    m_bigWidgetsLayout->addWidget(m_barWidget);
    m_bigWidgetsLayout->addWidget(m_colorSelectorWidget);
    m_bigWidgetsLayout->addWidget(m_myPaintShadeWidget);
    m_bigWidgetsLayout->addWidget(m_shadeSelectionWidget);

    m_horizontalColorPatchesLayout = new QHBoxLayout();
    m_horizontalColorPatchesLayout->addLayout(m_bigWidgetsLayout);

    m_verticalColorPathcesLayout = new QVBoxLayout(this);
    m_verticalColorPathcesLayout->addLayout(m_horizontalColorPatchesLayout);

    updateLayout();

    m_verticalColorPathcesLayout->addWidget(m_commonColorsWidget);

    connect(m_barWidget, SIGNAL(openSettings()), this, SLOT(openSettings()));
}

void KisColSelNgWidget::setCanvas(KoCanvasBase *canvas)
{
    m_commonColorsWidget->setCanvas(canvas);
}

void KisColSelNgWidget::openSettings()
{
    KisColSelNgSettings settings;
    if(settings.exec()==QDialog::Accepted) {
        m_lastColorsShow = settings.ui->lastUsedColorsShow->isChecked();
        m_lastColorsDirection = settings.ui->lastUsedColorsAlignVertical->isChecked()?KisColSelNgColorPatches::Vertical:KisColSelNgColorPatches::Horizontal;
        m_lastColorsScrolling = settings.ui->lastUsedColorsAllowScrolling->isChecked();
        m_lastColorsColCount = settings.ui->lastUsedColorsNumCols->value();
        m_lastColorsRowCount = settings.ui->lastUsedColorsNumRows->value();

        m_commonColorsShow = settings.ui->commonColorsShow->isChecked();
        m_commonColorsDirection = settings.ui->commonColorsAlignVertical->isChecked()?KisColSelNgColorPatches::Vertical:KisColSelNgColorPatches::Horizontal;
        m_commonColorsScrolling = settings.ui->commonColorsAllowScrolling->isChecked();
        m_commonColorsColCount = settings.ui->commonColorsNumCols->value();
        m_commonColorsRowCount = settings.ui->commonColorsNumRows->value();

        updateLayout();
    }
}

void KisColSelNgWidget::updateLayout()
{
    m_lastColorsWidget->setPatchLayout(m_lastColorsDirection, m_lastColorsScrolling, m_lastColorsRowCount, m_lastColorsColCount);
    m_commonColorsWidget->setPatchLayout(m_commonColorsDirection, m_commonColorsScrolling, m_commonColorsRowCount, m_commonColorsColCount);


    m_horizontalColorPatchesLayout->removeWidget(m_lastColorsWidget);
    m_horizontalColorPatchesLayout->removeWidget(m_commonColorsWidget);
    m_verticalColorPathcesLayout->removeWidget(m_lastColorsWidget);
    m_verticalColorPathcesLayout->removeWidget(m_commonColorsWidget);

    if(m_lastColorsShow==false)
        m_lastColorsWidget->hide();
    else
        m_lastColorsWidget->show();

    if(m_commonColorsShow==false)
        m_commonColorsWidget->hide();
    else
        m_commonColorsWidget->show();

    if(m_lastColorsShow && m_lastColorsDirection==KisColSelNgColorPatches::Vertical)
        m_horizontalColorPatchesLayout->addWidget(m_lastColorsWidget);

    if(m_commonColorsShow && m_commonColorsDirection==KisColSelNgColorPatches::Vertical)
        m_horizontalColorPatchesLayout->addWidget(m_commonColorsWidget);

    if(m_lastColorsShow && m_lastColorsDirection==KisColSelNgColorPatches::Horizontal)
        m_verticalColorPathcesLayout->addWidget(m_lastColorsWidget);

    if(m_commonColorsShow && m_commonColorsDirection==KisColSelNgColorPatches::Horizontal)
        m_verticalColorPathcesLayout->addWidget(m_commonColorsWidget);
}
