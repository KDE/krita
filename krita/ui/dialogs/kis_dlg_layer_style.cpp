#include "kis_dlg_layer_style.h"

#include <QWidget>
#include <QStackedWidget>
#include <QTreeWidget>

#include "kis_config.h"

KisDlgLayerStyle::KisDlgLayerStyle(QWidget *parent) :
    KDialog(parent)
{
    setCaption(i18n("Layer Styles"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    QWidget *page = new QWidget(this);
    wdgLayerStyles.setupUi(page);
    setMainWidget(page);

    stylesSelector = new StylesSelector(this);
    wdgLayerStyles.stylesStack->addWidget(stylesSelector);
    blendingOptions = new BlendingOptions(this);
    wdgLayerStyles.stylesStack->addWidget(blendingOptions);
    dropShadow = new DropShadow(this);
    wdgLayerStyles.stylesStack->addWidget(dropShadow);
    innerShadow = new InnerShadow(this);
    wdgLayerStyles.stylesStack->addWidget(innerShadow);
    outerGlow = new OuterGlow(this);
    wdgLayerStyles.stylesStack->addWidget(outerGlow);
    innerGlow = new InnerGlow(this);
    wdgLayerStyles.stylesStack->addWidget(innerGlow);
    bevelAndEmboss = new BevelAndEmboss(this);
    wdgLayerStyles.stylesStack->addWidget(bevelAndEmboss);
    contour = new Contour(this);
    wdgLayerStyles.stylesStack->addWidget(contour);
    texture = new Texture(this);
    wdgLayerStyles.stylesStack->addWidget(texture);
    satin = new Satin(this);
    wdgLayerStyles.stylesStack->addWidget(satin);
    colorOverlay = new ColorOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(colorOverlay);
    gradientOverlay = new GradientOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(gradientOverlay);
    patternOverlay = new PatternOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(patternOverlay);
    stroke = new Stroke(this);
    wdgLayerStyles.stylesStack->addWidget(stroke);

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
