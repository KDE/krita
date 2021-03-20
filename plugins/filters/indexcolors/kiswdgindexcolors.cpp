/*
 * SPDX-FileCopyrightText: 2014 Manuel Riecke <spell1337@gmail.com>
 *
 * SPDX-License-Identifier: ICS
 */

#include "filter/kis_color_transformation_configuration.h"

#include "kiswdgindexcolors.h"
#include "palettegeneratorconfig.h"
#include "ui_kiswdgindexcolors.h"
#include <KisGlobalResourcesInterface.h>

#include "kis_int_parse_spin_box.h"

#include <kis_color_button.h>

KisWdgIndexColors::KisWdgIndexColors(QWidget* parent, Qt::WindowFlags f, int delay): KisConfigWidget(parent, f, delay)
{
    ui = new Ui::KisWdgIndexColors;
    ui->setupUi(this);

    connect(ui->diagCheck,  SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->inbetweenSpinBox, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->alphaStepsSpinBox, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));

    connect(ui->colorLimit, SIGNAL(valueChanged(int)), SLOT(slotColorLimitChanged(int)));
    connect(ui->colorLimit, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));

    connect(ui->colorLimitCheck, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));

    connect(ui->luminanceSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->aSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->bSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
}

void KisWdgIndexColors::slotColorLimitChanged(int value)
{
    ui->colorLimit->setSuffix(i18ncp("suffix for a spinbox",
                                     " color", " colors", value));
}

void KisWdgIndexColors::setup(QStringList shadesLabels, int ramps)
{
    int rows     = shadesLabels.length();
    int columns = ramps;

    m_colorSelectors.resize(rows);
    m_stepSpinners.resize(rows-1);
    // Labels for the shades
    for(int row = 0; row < rows; ++row)
    {
        QLabel* l = new QLabel(shadesLabels[row], ui->colorsBox);
        ui->layoutColors->addWidget(l, row+1, 0);
        m_colorSelectors[row].resize(columns);
    }
    // Labels for the ramps
    /*for(int col = 0; col < columns; ++col)
    {
        QLabel* l = new QLabel(rampsLabels[col], ui->colorsBox);
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->layoutColors->addWidget(l, 0, col+1);
    }*/
    // Step selectors for the shade gradients
    for(int row = 0; row < (rows-1); ++row)
    {
        QLabel* l0 = new QLabel(shadesLabels[row+1]);
        QLabel* l1 = new QLabel(QString::fromUtf8("↔"));
        QLabel* l2 = new QLabel(shadesLabels[row]);

        QSpinBox* s = new KisIntParseSpinBox();
        s->setMinimum(0);
        s->setMaximum(32);
        s->setValue(2);

        connect(s, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationItemChanged()));
        m_stepSpinners[row] = s;

        ui->layoutRowSteps->addWidget(l0, row, 0);
        ui->layoutRowSteps->addWidget(l1, row, 1);
        ui->layoutRowSteps->addWidget(l2, row, 2);
        ui->layoutRowSteps->addWidget(s,  row, 3);
    }
    // Color selectors
    for(int y = 0; y < rows; ++y)
        for(int x = 0; x < columns; ++x)
        {
            KisColorButton* b = new KisColorButton;
            QCheckBox* c = new QCheckBox;
            c->setChecked(false);
            b->setEnabled(false);
            b->setMaximumWidth(50);
            c->setMaximumWidth(21); // Ugh. I hope this won't be causing any issues. Trying to get rid of the unnecessary spacing after it.

            connect(c, SIGNAL(toggled(bool)), b, SLOT(setEnabled(bool)));
            connect(c, SIGNAL(toggled(bool)), this, SIGNAL(sigConfigurationItemChanged()));
            connect(b, SIGNAL(changed(KoColor)), this, SIGNAL(sigConfigurationItemChanged()));

            QHBoxLayout* cell = new QHBoxLayout();
            cell->setSpacing(0);
            cell->setContentsMargins(0, 0, 0, 0);
            cell->addWidget(c);
            cell->addWidget(b);
            ui->layoutColors->addLayout(cell, 1+y, 1+x);

            m_colorSelectors[y][x].button = b;
            m_colorSelectors[y][x].checkbox = c;
        }
}


KisPropertiesConfigurationSP KisWdgIndexColors::configuration() const
{
    KisColorTransformationConfigurationSP config = new KisColorTransformationConfiguration("indexcolors", 1, KisGlobalResourcesInterface::instance());

    PaletteGeneratorConfig palCfg;

    for(int y = 0; y < 4; ++y)
        for(int x = 0; x < 4; ++x)
        {
            palCfg.colors[y][x] = m_colorSelectors[y][x].button->color().toQColor();
            palCfg.colorsEnabled[y][x] = m_colorSelectors[y][x].button->isEnabled();
        }

    for(int y = 0; y < 3; ++y)
        palCfg.gradientSteps[y] = m_stepSpinners[y]->value();

    palCfg.diagonalGradients = ui->diagCheck->isChecked();
    palCfg.inbetweenRampSteps = ui->inbetweenSpinBox->value();
    
    IndexColorPalette pal = palCfg.generate();
    ui->colorCount->setText(QString::number(pal.numColors()));

    config->setProperty("paletteGen", palCfg.toByteArray());

    config->setProperty("LFactor",   ui->luminanceSlider->value() / 100.f);
    config->setProperty("aFactor",   ui->aSlider->value() / 100.f);
    config->setProperty("bFactor",   ui->bSlider->value() / 100.f);
    
    config->setProperty("reduceColorsEnabled", ui->colorLimitCheck->isChecked());
    config->setProperty("colorLimit", ui->colorLimit->value());

    config->setProperty("alphaSteps", ui->alphaStepsSpinBox->value());
    return config;
}

void KisWdgIndexColors::setConfiguration(const KisPropertiesConfigurationSP config)
{
    PaletteGeneratorConfig palCfg;
    palCfg.fromByteArray(config->getProperty("paletteGen").toByteArray());
    
    ui->luminanceSlider->setValue(config->getFloat("LFactor")*100);
    ui->aSlider->setValue(config->getFloat("aFactor")*100);
    ui->bSlider->setValue(config->getFloat("bFactor")*100);
    ui->alphaStepsSpinBox->setValue(config->getInt("alphaSteps"));
    ui->colorLimitCheck->setChecked(config->getBool("reduceColorsEnabled"));
    ui->colorLimit->setEnabled(config->getBool("reduceColorsEnabled"));
    ui->colorLimit->setValue(config->getInt("colorLimit"));
    ui->diagCheck->setChecked(palCfg.diagonalGradients);
    ui->inbetweenSpinBox->setValue(palCfg.inbetweenRampSteps);

    for(int y = 0; y < 4; ++y)
        for(int x = 0; x < 4; ++x)
        {
            m_colorSelectors[y][x].checkbox->setChecked(palCfg.colorsEnabled[y][x]);
            m_colorSelectors[y][x].button->setEnabled(palCfg.colorsEnabled[y][x]);
            KoColor c;
            c.fromQColor(palCfg.colors[y][x]);
            m_colorSelectors[y][x].button->setColor(c);
        }

    for(int y = 0; y < 3; ++y)
        m_stepSpinners[y]->setValue(palCfg.gradientSteps[y]);
    
    IndexColorPalette pal = palCfg.generate();
    ui->colorCount->setText(QString::number(pal.numColors()));
}
