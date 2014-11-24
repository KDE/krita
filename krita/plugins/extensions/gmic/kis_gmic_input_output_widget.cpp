/*
 * Copyright (c) 2013-2014 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <KoIcon.h>

#include <kis_gmic_input_output_widget.h>

#include <kis_debug.h>
#include <kis_gmic_filter_settings.h>

#include <QGridLayout>
#include <QComboBox>
#include <QStringListModel>
#include <QLabel>

KisGmicInputOutputWidget::KisGmicInputOutputWidget(QWidget * parent):
    QWidget(parent),
    m_inputMode(ACTIVE_LAYER),
    m_outputMode(IN_PLACE),
    m_previewMode(FIRST),
    m_previewSize(TINY)
{
    setupUi(this);
    createMainLayout();
}

KisGmicInputOutputWidget::~KisGmicInputOutputWidget()
{

}

KisFilterPreviewWidget* KisGmicInputOutputWidget::previewWidget()
{
    return previewViewport;
}



void KisGmicInputOutputWidget::createMainLayout()
{
    zoomInButton->setIcon(koIcon("zoom-in"));
    zoomOutButton->setIcon(koIcon("zoom-out"));

    QStringListModel * inputModel = new QStringListModel(INPUT_MODE_STRINGS);
    inputCombo->setModel(inputModel);
    QObject::connect(inputCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setIntputMode(int)));
    inputCombo->setCurrentIndex(static_cast<int>(m_inputMode));

    QStringListModel * outputModel = new QStringListModel(OUTPUT_MODE_STRINGS);
    outputCombo->setModel(outputModel);
    QObject::connect(outputCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setOutputMode(int)));
    outputCombo->setCurrentIndex(static_cast<int>(m_outputMode));

    QStringListModel * previewModeModel = new QStringListModel(PREVIEW_MODE);
    outputPreviewCombo->setModel(previewModeModel);
    QObject::connect(outputPreviewCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setPreviewMode(int)));
    outputPreviewCombo->setCurrentIndex(static_cast<int>(m_previewMode));

    QStringListModel * previewSizeModel = new QStringListModel(PREVIEW_SIZE);
    previewSizeCombo->setModel(previewSizeModel);
    QObject::connect(previewSizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setPreviewSize(int)));
    previewSizeCombo->setCurrentIndex(static_cast<int>(m_previewSize));
}


void KisGmicInputOutputWidget::setIntputMode(int index)
{
    m_inputMode = static_cast<InputLayerMode>(index);
    emit sigConfigurationChanged();
}


void KisGmicInputOutputWidget::setOutputMode(int index)
{
    m_outputMode = static_cast<OutputMode>(index);
    emit sigConfigurationChanged();
}

void KisGmicInputOutputWidget::setPreviewMode(int index)
{
    m_previewMode = static_cast<OutputPreviewMode>(index);
    emit sigConfigurationChanged();
}

void KisGmicInputOutputWidget::setPreviewSize(int index)
{
    m_previewSize = static_cast<PreviewSize>(index);
    emit sigConfigurationChanged();
}
