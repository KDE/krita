/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#include <kis_gmic_input_output_widget.h>
#include <QGridLayout>
#include <QComboBox>
#include <QStringListModel>
#include <QLabel>
#include <QDebug>

#include <kis_gmic_filter_settings.h>

KisGmicInputOutputWidget::KisGmicInputOutputWidget(): QWidget(), m_inputMode(ACTIVE_LAYER), m_outputMode(IN_PLACE)
{
    createMainLayout();
}

KisGmicInputOutputWidget::~KisGmicInputOutputWidget()
{

}

void KisGmicInputOutputWidget::createMainLayout()
{
    QComboBox * inputCombo = new QComboBox;
    QStringListModel * inputModel = new QStringListModel(INPUT_MODE_STRINGS);
    inputCombo->setModel(inputModel);
    QObject::connect(inputCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setIntputMode(int)));
    inputCombo->setCurrentIndex(static_cast<int>(m_inputMode));

    QComboBox * outputCombo = new QComboBox;
    QStringListModel * outputModel = new QStringListModel(OUTPUT_MODE_STRINGS);
    outputCombo->setModel(outputModel);
    QObject::connect(outputCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setOutputMode(int)));
    outputCombo->setCurrentIndex(static_cast<int>(m_outputMode));

    QGridLayout * gridLayout = new QGridLayout;
    int row = 0;
    gridLayout->addWidget(new QLabel("Input"), row, 0);
    gridLayout->addWidget(inputCombo, row, 1, 1, 2);
    row++;
    gridLayout->addWidget(new QLabel("Output"), row, 0);
    gridLayout->addWidget(outputCombo, row, 1, 1, 2);

    setLayout(gridLayout);
}


void KisGmicInputOutputWidget::setIntputMode(int index)
{
        m_inputMode = static_cast<InputLayerMode>(index);
        qDebug() << "Selecting " << INPUT_MODE_STRINGS.at(index);
        emit sigConfigurationChanged();

}


void KisGmicInputOutputWidget::setOutputMode(int index)
{
        m_outputMode = static_cast<OutputMode>(index);
        qDebug() << "Selecting " << OUTPUT_MODE_STRINGS.at(index);
        emit sigConfigurationChanged();
}
