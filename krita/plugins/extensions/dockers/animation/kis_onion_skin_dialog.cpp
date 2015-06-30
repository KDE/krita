#include "kis_onion_skin_dialog.h"
#include "ui_kis_onion_skin_dialog.h"

#include <QSlider>
#include <QFrame>
#include <QGridLayout>
#include "klocale.h"

#include "kis_image_config.h"
#include "kis_onion_skin_compositor.h"

static const int MAX_SKIN_COUNT = 10;

KisOnionSkinDialog::KisOnionSkinDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KisOnionSkinDialog)
{
    KisImageConfig config;
    ui->setupUi(this);

    ui->doubleTintFactor->setMinimum(0);
    ui->doubleTintFactor->setMaximum(100);
    ui->doubleTintFactor->setPrefix(i18n("Tint: "));
    ui->doubleTintFactor->setSuffix("%");
    ui->doubleTintFactor->setValue(config.onionSkinTintFactor() * 100.0 / 255);

    ui->btnBackwardColor->setColor(config.onionSkinTintColorBackward());
    ui->btnForwardColor->setColor(config.onionSkinTintColorForward());
    ui->btnBackwardColor->setToolTip(i18n("Tint color for past frames"));
    ui->btnForwardColor->setToolTip(i18n("Tint color for future frames"));

    QGridLayout *grid = ui->gridLayout;

    numberOfSkins.setOrientation(Qt::Horizontal);
    numberOfSkins.setMinimum(1);
    numberOfSkins.setMaximum(MAX_SKIN_COUNT);
    numberOfSkins.setValue(config.numberOfOnionSkins());
    numberOfSkins.setToolTip(i18n("Number of onion skins"));
    grid->addWidget(&numberOfSkins, 1, MAX_SKIN_COUNT + 1, 1, MAX_SKIN_COUNT);

    for (int i = 0; i < MAX_SKIN_COUNT; i++) {
        QSlider *opacitySlider;

        opacitySlider = new QSlider();
        opacitySlider->setMinimum(0);
        opacitySlider->setMaximum(255);
        opacitySlider->setValue(config.onionSkinOpacity(-(i+1)));
        connect(opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(changed()));
        backwardOpacities.append(opacitySlider);

        opacitySlider = new QSlider();
        opacitySlider->setMinimum(0);
        opacitySlider->setMaximum(255);
        opacitySlider->setValue(config.onionSkinOpacity(i+1));
        connect(opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(changed()));
        forwardOpacities.append(opacitySlider);
    }

    for (int column = 0; column < MAX_SKIN_COUNT * 2 + 1; column++) {
        if (column == MAX_SKIN_COUNT) {
            QFrame *separator = new QFrame(this);
            separator->setFrameShape(QFrame::VLine);
            separator->setFrameShadow(QFrame::Raised);
            grid->addWidget(separator, 2, MAX_SKIN_COUNT, 1, 1, Qt::AlignHCenter);
            grid->setColumnMinimumWidth(MAX_SKIN_COUNT, 16);
        } else {
            int index = abs(column - MAX_SKIN_COUNT) - 1;

            QSlider *slider = (column < MAX_SKIN_COUNT) ?
                        backwardOpacities.at(index) :
                        forwardOpacities.at(index);

            grid->addWidget(slider, 2, column, 1, 1, Qt::AlignHCenter);
        }
    }

    connect(&numberOfSkins, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(ui->btnBackwardColor, SIGNAL(changed(QColor)), this, SLOT(changed()));
    connect(ui->btnForwardColor, SIGNAL(changed(QColor)), this, SLOT(changed()));
    connect(ui->doubleTintFactor, SIGNAL(valueChanged(qreal)), this, SLOT(changed()));
}

KisOnionSkinDialog::~KisOnionSkinDialog()
{
    delete ui;
}

void KisOnionSkinDialog::changed()
{
    KisImageConfig config;

    config.setNumberOfOnionSkins(numberOfSkins.value());

    for (int i = 0; i < MAX_SKIN_COUNT; i++) {
        config.setOnionSkinOpacity(-(i+1), backwardOpacities.at(i)->value());
        config.setOnionSkinOpacity( (i+1), forwardOpacities.at(i)->value());
    }

    config.setOnionSkinTintFactor(ui->doubleTintFactor->value() * 255 / 100);
    config.setOnionSkinTintColorBackward(ui->btnBackwardColor->color());
    config.setOnionSkinTintColorForward(ui->btnForwardColor->color());

    KisOnionSkinCompositor::instance()->configChanged();
}
