/*
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_transport_controls.h"

#include <QHBoxLayout>
#include <QPushButton>

#include "kis_icon.h"

KisTransportControls::KisTransportControls(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(parent);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    buttonBack = new QPushButton(KisIconUtils::loadIcon("prevframe"), "", this);
    layout->addWidget(buttonBack);
    connect(buttonBack, SIGNAL(released()), this, SIGNAL(back()));

    buttonStop = new QPushButton(KisIconUtils::loadIcon("animation_stop"), "", this);
    layout->addWidget(buttonStop);
    connect(buttonStop, SIGNAL(released()), this, SIGNAL(stop()));

    buttonPlayPause = new QPushButton(KisIconUtils::loadIcon("animation_play"), "", this);
    layout->addWidget(buttonPlayPause);
    connect(buttonPlayPause, SIGNAL(released()), this, SIGNAL(playPause()));

    buttonForward = new QPushButton(KisIconUtils::loadIcon("nextframe"), "", this);
    layout->addWidget(buttonForward);
    connect(buttonForward, SIGNAL(released()), this, SIGNAL(forward()));

    setLayout(layout);
}

KisTransportControls::~KisTransportControls()
{
}

QSize KisTransportControls::sizeHint() const
{
    return QSize(32, 32);
}

void KisTransportControls::setPlaying(bool playing)
{
    if (playing) {
        buttonPlayPause->setIcon(KisIconUtils::loadIcon("animation_pause"));
    } else {
        buttonPlayPause->setIcon(KisIconUtils::loadIcon("animation_play"));
    }
}
