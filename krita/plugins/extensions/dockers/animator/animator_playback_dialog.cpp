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

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(loopState, 0, 0);

    this->setLayout(mainLayout);
}

void AnimatorPlaybackDialog::enableLooping(bool enable)
{
    m_model->enableLooping(enable);
}

void AnimatorPlaybackDialog::setModel(KisAnimation *model)
{
    m_model = model;
}
