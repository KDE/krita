#include "kis_colselng_bar.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <KIcon>

KisColSelNgBar::KisColSelNgBar(QWidget *parent) :
    QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    QPushButton* colorSelButton = new QPushButton(this);
    QPushButton* myPaintSelButton = new QPushButton(this);
    QPushButton* patchesButton = new QPushButton(this);
    QPushButton* pipetteButton = new QPushButton(this);
    QPushButton* settingsButton = new QPushButton(this);

    colorSelButton->setIcon(KIcon("kis_colselng_color_triangle"));
    myPaintSelButton->setIcon(KIcon("kis_colselng_my_paint_shade_selector"));
    patchesButton->setIcon(KIcon("kis_colselng_color_patches"));
    pipetteButton->setIcon(KIcon("krita_tool_color_picker"));
    settingsButton->setIcon(KIcon("configure"));

    colorSelButton->setMaximumWidth(24);
    myPaintSelButton->setMaximumWidth(24);
    patchesButton->setMaximumWidth(24);
    pipetteButton->setMaximumWidth(24);
    settingsButton->setMaximumWidth(24);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(colorSelButton);
    layout->addWidget(myPaintSelButton);
    layout->addWidget(patchesButton);
    layout->addWidget(pipetteButton);
    layout->addStretch(2);
    layout->addWidget(settingsButton);

}
