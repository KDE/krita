/*
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transport_controls.h"

#include <QHBoxLayout>
#include <QPushButton>

#include "kis_icon.h"
#include "klocalizedstring.h"


KisTransportControls::KisTransportControls(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    buttonBack = new QPushButton(KisIconUtils::loadIcon("prevframe"), "", this);
    buttonBack->setToolTip(i18n("Back"));
    buttonBack->setFlat(true);
    layout->addWidget(buttonBack);
    connect(buttonBack, SIGNAL(released()), this, SIGNAL(back()));

    buttonStop = new QPushButton(KisIconUtils::loadIcon("animation_stop"), "", this);
    buttonStop->setToolTip(i18n("Stop"));
    buttonStop->setFlat(true);
    layout->addWidget(buttonStop);
    connect(buttonStop, SIGNAL(released()), this, SIGNAL(stop()));

    buttonPlayPause = new QPushButton(KisIconUtils::loadIcon("animation_play"), "", this);
    buttonPlayPause->setToolTip(i18n("Play/Pause"));
    buttonPlayPause->setFlat(true);
    layout->addWidget(buttonPlayPause);
    connect(buttonPlayPause, SIGNAL(released()), this, SIGNAL(playPause()));

    buttonForward = new QPushButton(KisIconUtils::loadIcon("nextframe"), "", this);
    buttonForward->setToolTip(i18n("Forward"));
    buttonForward->setFlat(true);
    layout->addWidget(buttonForward);
    connect(buttonForward, SIGNAL(released()), this, SIGNAL(forward()));
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
