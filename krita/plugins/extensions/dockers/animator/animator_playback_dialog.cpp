/*
 *  Copyright (c) 2014 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
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

#include "animator_playback_dialog.h"
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QGridLayout>
#include <kis_config.h>

AnimatorPlaybackDialog::AnimatorPlaybackDialog(QWidget *parent)
    : QDialog(parent)
{
    this->setWindowTitle("Playback options");
    this->setMaximumSize(300, 200);

    KisConfig cfg;
    QCheckBox* loopState = new QCheckBox(this);
    loopState->setText("Enable Looping");
    loopState->setChecked(cfg.defLoopingEnabled());

    connect(loopState, SIGNAL(clicked(bool)), this, SLOT(enableLooping(bool)));

    QLabel* fpsLabel = new QLabel("Frame per second:", this);

    QSpinBox* fpsInput = new QSpinBox(this);
    fpsInput->setRange(1, 30);
    fpsInput->setValue(cfg.defFps());

    connect(fpsInput, SIGNAL(valueChanged(int)), this, SLOT(setFps(int)));

    QLabel* localPlaybackRangeLabel = new QLabel("Local playback range:", this);

    QSpinBox* localPlaybackRangeInput = new QSpinBox(this);
    localPlaybackRangeInput->setRange(1, 10000);
    localPlaybackRangeInput->setValue(cfg.defLocalPlaybackRange());

    connect(localPlaybackRangeInput, SIGNAL(valueChanged(int)), this, SLOT(setLocalPlaybackRange(int)));

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(loopState, 0, 0);
    mainLayout->addWidget(fpsLabel, 1, 0);
    mainLayout->addWidget(fpsInput, 1, 1);
    mainLayout->addWidget(localPlaybackRangeLabel, 2, 0);
    mainLayout->addWidget(localPlaybackRangeInput, 2, 1);

    this->setLayout(mainLayout);
}

void AnimatorPlaybackDialog::enableLooping(bool enable)
{
    m_model->enableLooping(enable);
    emit playbackStateChanged();
}

void AnimatorPlaybackDialog::setFps(int value)
{
    m_model->setFps(value);
    emit playbackStateChanged();
}

void AnimatorPlaybackDialog::setLocalPlaybackRange(int value)
{
    m_model->setLocalPlaybackRange(value);
}

void AnimatorPlaybackDialog::setModel(KisAnimation *model)
{
    m_model = model;
}
