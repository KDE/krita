/*
 *  Copyright (c) 2014 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "animator_playback_dialog.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include "kis_config.h"

AnimatorPlaybackDialog::AnimatorPlaybackDialog(QWidget *parent)
    : QDialog(parent)
{
    this->setWindowTitle(i18n("Playback options"));
    this->setMaximumSize(300, 200);

    KisConfig cfg;
    m_loopState = new QCheckBox(this);
    m_loopState->setText(i18n("Enable Looping"));
    m_loopState->setChecked(cfg.defLoopingEnabled());

    QLabel* fpsLabel = new QLabel(i18n("Frame per second:"), this);

    m_fpsInput = new QSpinBox(this);
    m_fpsInput->setRange(1, 30);
    m_fpsInput->setValue(cfg.defFps());

    QLabel* localPlaybackRangeLabel = new QLabel(i18n("Local playback range:"), this);

    m_localPlaybackRangeInput = new QSpinBox(this);
    m_localPlaybackRangeInput->setRange(1, 10000);
    m_localPlaybackRangeInput->setValue(cfg.defLocalPlaybackRange());

    QDialogButtonBox* buttonBox = new QDialogButtonBox();

    buttonBox->addButton(QDialogButtonBox::Ok);
    QPushButton* okButton = buttonBox->button(QDialogButtonBox::Ok);
    connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));

    buttonBox->addButton(QDialogButtonBox::Cancel);
    QPushButton* cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()));

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(m_loopState, 0, 0);
    mainLayout->addWidget(fpsLabel, 1, 0);
    mainLayout->addWidget(m_fpsInput, 1, 1);
    mainLayout->addWidget(localPlaybackRangeLabel, 2, 0);
    mainLayout->addWidget(m_localPlaybackRangeInput, 2, 1);
    mainLayout->addWidget(buttonBox, 3, 1);

    this->setLayout(mainLayout);
}

void AnimatorPlaybackDialog::setModel(KisAnimation *model)
{
    m_model = model;
}

void AnimatorPlaybackDialog::okClicked()
{
    m_model->enableLooping(m_loopState->isChecked());
    m_model->setFps(m_fpsInput->value());
    emit playbackStateChanged();

    m_model->setLocalPlaybackRange(m_localPlaybackRangeInput->value());

    this->close();
}

void AnimatorPlaybackDialog::cancelClicked()
{
    this->close();
}
