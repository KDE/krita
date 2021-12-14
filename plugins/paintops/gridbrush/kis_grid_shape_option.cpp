/*
 * SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_grid_shape_option.h"
#include <klocalizedstring.h>

#include "ui_wdggridbrushshapeoptions.h"

class KisShapeOptionsWidget: public QWidget, public Ui::WdgGridBrushShapeOptions
{
public:
    KisShapeOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

KisGridShapeOption::KisGridShapeOption()
    : KisPaintOpOption(i18n("Particle type"), KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisGridShapeOption");

    m_checkable = false;
    m_options = new KisShapeOptionsWidget();
    connect(m_options->shapeCBox, SIGNAL(currentIndexChanged(int)), SLOT(emitSettingChanged()));
    setConfigurationPage(m_options);
}

KisGridShapeOption::~KisGridShapeOption()
{
    delete m_options;
}


int KisGridShapeOption::shape() const
{
    return m_options->shapeCBox->currentIndex();
}

void KisGridShapeOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    setting->setProperty(GRIDSHAPE_SHAPE, shape());
}


void KisGridShapeOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    m_options->shapeCBox->setCurrentIndex(setting->getInt(GRIDSHAPE_SHAPE));
}
