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


#include <QDebug>

KisColSelNgWidget::KisColSelNgWidget(QWidget *parent) :
    QWidget(parent), m_verticalColorPatchesLayout(0), m_horizontalColorPatchesLayout(0)
{
    m_bigWidgetsParent = new QWidget(this);
//    m_bigWidgetsParent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
//    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    m_barWidget = new KisColSelNgBar(this);
    m_colorSelectorWidget = new KisColSelNgColorSelector(m_bigWidgetsParent);
    m_myPaintShadeWidget = new KisColSelNgMyPaintShadeSelector(m_bigWidgetsParent);
    m_minimalShadeWidget = new KisColSelNgShadeSelector(m_bigWidgetsParent);
    m_lastColorsWidget = new KisColSelNgColorPatches(this);
    m_commonColorsWidget = new KisColSelNgCommonColors(this);

    m_myPaintShadeWidget->hide();
    m_minimalShadeWidget->hide();

    //default settings
    //remember to also change the default in the ui

    //shade selector
    m_shadeWidget = m_myPaintShadeWidget;   //0, if it shouldn't be shown
    m_shadeSelectorHideable = true;

    //color patches
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


    QVBoxLayout* bwpLayout = new QVBoxLayout(m_bigWidgetsParent);
    bwpLayout->setSpacing(0);
    bwpLayout->addWidget(m_colorSelectorWidget);
    bwpLayout->addWidget(m_myPaintShadeWidget);
    bwpLayout->addWidget(m_minimalShadeWidget);

    QHBoxLayout* horzLayout = new QHBoxLayout();
    horzLayout->setSpacing(0);
    horzLayout->addWidget(m_bigWidgetsParent);


    m_standardBarLayout = new QVBoxLayout();
    m_standardBarLayout->setSpacing(0);
    m_standardBarLayout->addWidget(m_barWidget);
    horzLayout->addLayout(m_standardBarLayout);

    m_verticalColorPatchesLayout = new QHBoxLayout();
    m_verticalColorPatchesLayout->setSpacing(2);
    m_standardBarLayout->addLayout(m_verticalColorPatchesLayout);

    m_horizontalColorPatchesLayout = new QVBoxLayout(this);
    m_horizontalColorPatchesLayout->setSpacing(2);
    m_horizontalColorPatchesLayout->addLayout(horzLayout);

    updateLayout();

    connect(m_barWidget, SIGNAL(openSettings()), this, SLOT(openSettings()));
}

void KisColSelNgWidget::setCanvas(KoCanvasBase *canvas)
{
    m_commonColorsWidget->setCanvas(canvas);
}

void KisColSelNgWidget::openSettings()
{
    qDebug()<<"minimumHeight()="<<minimumHeight()<<"   \n"<<"sizeHint()"<<sizeHint().height();
    
    KisColSelNgSettings settings;
    if(settings.exec()==QDialog::Accepted) {
        //shade selectors
        int shadeSelector = settings.ui->shadeSelectorType->currentIndex();
        if(shadeSelector==0)
            m_shadeWidget = m_myPaintShadeWidget;
        else if(shadeSelector==1)
            m_shadeWidget = m_minimalShadeWidget;
        else m_shadeWidget = 0;

        m_shadeSelectorHideable = settings.ui->shadeSelectorHideable->isChecked();

        //color patches
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

void KisColSelNgWidget::resizeEvent(QResizeEvent* e)
{
    int height = m_bigWidgetsParent->height()-m_colorSelectorWidget->minimumHeight();

    if(m_shadeWidget!=0) {
        height -= m_shadeWidget->minimumHeight();
        if(height<0 && m_shadeSelectorHideable)
            m_shadeWidget->hide();
//        else if (height<0 && m_shadeSelectorHideable==false)
//            setMinimumHeight(this->height()-height);
        else
            m_shadeWidget->show();
    }

    if(e!=0)
        QWidget::resizeEvent(e);
}

void KisColSelNgWidget::updateLayout()
{
    //bar (color picker and settings)
    if((m_commonColorsDirection==KisColSelNgColorPatches::Horizontal || m_commonColorsShow==false)
       && (m_lastColorsDirection==KisColSelNgColorPatches::Horizontal || m_lastColorsShow==false)) {
        m_standardBarLayout->removeWidget(m_barWidget);
        m_barWidget->setParent(m_bigWidgetsParent);
        m_barWidget->setMaximumWidth(QWIDGETSIZE_MAX);
        m_barWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        Q_ASSERT(dynamic_cast<QVBoxLayout*>(m_bigWidgetsParent->layout()));
        dynamic_cast<QVBoxLayout*>(m_bigWidgetsParent->layout())->insertWidget(0, m_barWidget);
    }
    else {
        Q_ASSERT(dynamic_cast<QVBoxLayout*>(m_bigWidgetsParent->layout()));
        dynamic_cast<QVBoxLayout*>(m_bigWidgetsParent->layout())->removeWidget(m_barWidget);
        m_barWidget->setParent(this);
        m_barWidget->setMaximumWidth(m_barWidget->minimumWidth());
        m_barWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        m_standardBarLayout->insertWidget(0, m_barWidget);
    }

    //shade selector
    m_myPaintShadeWidget->hide();
    m_minimalShadeWidget->hide();
    if(m_shadeWidget!=0)
        m_shadeWidget->show();

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

    if(m_commonColorsShow==false)
        m_commonColorsWidget->hide();
    else
        m_commonColorsWidget->show();

    if(m_lastColorsShow && m_lastColorsDirection==KisColSelNgColorPatches::Vertical)
        m_verticalColorPatchesLayout->addWidget(m_lastColorsWidget);

    if(m_commonColorsShow && m_commonColorsDirection==KisColSelNgColorPatches::Vertical)
        m_verticalColorPatchesLayout->addWidget(m_commonColorsWidget);

    if(m_lastColorsShow && m_lastColorsDirection==KisColSelNgColorPatches::Horizontal)
        m_horizontalColorPatchesLayout->addWidget(m_lastColorsWidget);

    if(m_commonColorsShow && m_commonColorsDirection==KisColSelNgColorPatches::Horizontal)
        m_horizontalColorPatchesLayout->addWidget(m_commonColorsWidget);

    updateGeometry();
}
