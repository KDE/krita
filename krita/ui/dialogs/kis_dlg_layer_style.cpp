/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_dlg_layer_style.h"

#include <QWidget>
#include <QStackedWidget>
#include <QTreeWidget>

#include "kis_config.h"

KisDlgLayerStyle::KisDlgLayerStyle(KisPSDLayerStyle *layerStyle, QWidget *parent)
    : KDialog(parent)
    , m_layerStyle(layerStyle)
{
    setCaption(i18n("Layer Styles"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    QWidget *page = new QWidget(this);
    wdgLayerStyles.setupUi(page);
    setMainWidget(page);

    m_stylesSelector = new StylesSelector(this);
    wdgLayerStyles.stylesStack->addWidget(m_stylesSelector);
    m_blendingOptions = new BlendingOptions(this);
    wdgLayerStyles.stylesStack->addWidget(m_blendingOptions);
    m_dropShadow = new DropShadow(this);
    wdgLayerStyles.stylesStack->addWidget(m_dropShadow);
    m_innerShadow = new InnerShadow(this);
    wdgLayerStyles.stylesStack->addWidget(m_innerShadow);
    m_outerGlow = new OuterGlow(this);
    wdgLayerStyles.stylesStack->addWidget(m_outerGlow);
    m_innerGlow = new InnerGlow(this);
    wdgLayerStyles.stylesStack->addWidget(m_innerGlow);
    m_bevelAndEmboss = new BevelAndEmboss(this);
    wdgLayerStyles.stylesStack->addWidget(m_bevelAndEmboss);
    m_contour = new Contour(this);
    wdgLayerStyles.stylesStack->addWidget(m_contour);
    m_texture = new Texture(this);
    wdgLayerStyles.stylesStack->addWidget(m_texture);
    m_satin = new Satin(this);
    wdgLayerStyles.stylesStack->addWidget(m_satin);
    m_colorOverlay = new ColorOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(m_colorOverlay);
    m_gradientOverlay = new GradientOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(m_gradientOverlay);
    m_patternOverlay = new PatternOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(m_patternOverlay);
    m_stroke = new Stroke(this);
    wdgLayerStyles.stylesStack->addWidget(m_stroke);

    KisConfig cfg;
    wdgLayerStyles.stylesStack->setCurrentIndex(cfg.readEntry("KisDlgLayerStyle::current", 1));
    wdgLayerStyles.lstStyleSelector->setCurrentRow(cfg.readEntry("KisDlgLayerStyle::current", 1));

    connect(wdgLayerStyles.lstStyleSelector,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
             this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

}

void KisDlgLayerStyle::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current) {
        current = previous;
    }
    wdgLayerStyles.stylesStack->setCurrentIndex(wdgLayerStyles.lstStyleSelector->row(current));
}



BevelAndEmboss::BevelAndEmboss(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


BlendingOptions::BlendingOptions(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


ColorOverlay::ColorOverlay(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


Contour::Contour(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


DropShadow::DropShadow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


GradientOverlay::GradientOverlay(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


InnerGlow::InnerGlow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


InnerShadow::InnerShadow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


PatternOverlay::PatternOverlay(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


Satin::Satin(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


Stroke::Stroke(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


StylesSelector::StylesSelector(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


Texture::Texture(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


OuterGlow::OuterGlow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}
