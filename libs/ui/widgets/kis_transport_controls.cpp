/*
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transport_controls.h"

#include <QKeyEvent>
#include <QHBoxLayout>
#include <QPushButton>

#include "kis_debug.h"
#include "kis_icon.h"
#include "klocalizedstring.h"


KisTransportControls::KisTransportControls(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    buttonSkipBack = new QPushButton(KisIconUtils::loadIcon("prevkeyframe"), "", this);
    buttonSkipBack->setToolTip(i18n("Skip Back"));
    buttonSkipBack->setIconSize(QSize(22, 22));
    buttonSkipBack->setFlat(true);
    layout->addWidget(buttonSkipBack);
    connect(buttonSkipBack, SIGNAL(released()), this, SIGNAL(skipBack()));

    buttonBack = new QPushButton(KisIconUtils::loadIcon("prevframe"), "", this);
    buttonBack->setToolTip(i18n("Back"));
    buttonBack->setIconSize(QSize(22, 22));
    buttonBack->setFlat(true);
    layout->addWidget(buttonBack);
    connect(buttonBack, SIGNAL(released()), this, SIGNAL(back()));

    buttonStop = new QPushButton(KisIconUtils::loadIcon("animation_stop"), "", this);
    buttonStop->setToolTip(i18n("Stop"));
    buttonStop->setIconSize(QSize(22, 22));
    buttonStop->setFlat(true);
    layout->addWidget(buttonStop);
    connect(buttonStop, SIGNAL(released()), this, SIGNAL(stop()));

    buttonPlayPause = new QPushButton(KisIconUtils::loadIcon("animation_play"), "", this);
    buttonPlayPause->setToolTip(i18n("Play/Pause"));
    buttonPlayPause->setIconSize(QSize(22, 22));
    buttonPlayPause->setFlat(true);
    layout->addWidget(buttonPlayPause);
    connect(buttonPlayPause, SIGNAL(released()), this, SIGNAL(playPause()));

    buttonForward = new QPushButton(KisIconUtils::loadIcon("nextframe"), "", this);
    buttonForward->setToolTip(i18n("Forward"));
    buttonForward->setIconSize(QSize(22, 22));
    buttonForward->setFlat(true);
    layout->addWidget(buttonForward);
    connect(buttonForward, SIGNAL(released()), this, SIGNAL(forward()));

    buttonSkipForward = new QPushButton(KisIconUtils::loadIcon("nextkeyframe"), "", this);
    buttonSkipForward->setToolTip(i18n("Skip Forward"));
    buttonSkipForward->setIconSize(QSize(22, 22));
    buttonSkipForward->setFlat(true);
    layout->addWidget(buttonSkipForward);
    connect(buttonSkipForward, SIGNAL(released()), this, SIGNAL(skipForward()));

    showStateButtons(true);
    showSeekButtons(true);
    showSkipButtons(false);

    setFocusPolicy(Qt::ClickFocus);
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

void KisTransportControls::showStateButtons(bool show)
{
    if (show) {
        buttonPlayPause->show();
        buttonStop->show();
    } else {
        buttonPlayPause->hide();
        buttonStop->hide();
    }
}

void KisTransportControls::showSeekButtons(bool show)
{
    if (show) {
        buttonBack->show();
        buttonForward->show();
    } else {
        buttonBack->hide();
        buttonForward->hide();
    }
}

void KisTransportControls::showSkipButtons(bool show)
{
    if (show) {
        buttonSkipBack->show();
        buttonSkipForward->show();
    } else {
        buttonSkipBack->hide();
        buttonSkipForward->hide();
    }
}
