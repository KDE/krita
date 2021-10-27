/*
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transport_controls.h"

#include <QKeyEvent>
#include <QHBoxLayout>
#include <QToolButton>

#include "kis_debug.h"
#include "kis_icon.h"
#include "klocalizedstring.h"


KisTransportControls::KisTransportControls(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    buttonSkipBack = new QToolButton(this);
    buttonSkipBack->setIcon(KisIconUtils::loadIcon("prevkeyframe"));
    buttonSkipBack->setToolTip(i18n("Skip Back"));
    buttonSkipBack->setIconSize(QSize(22, 22));
    buttonSkipBack->setFocusPolicy(Qt::NoFocus);
    buttonSkipBack->setAutoRaise(true);
    layout->addWidget(buttonSkipBack);
    connect(buttonSkipBack, SIGNAL(released()), this, SIGNAL(skipBack()));

    buttonBack = new QToolButton(this);
    buttonBack->setIcon(KisIconUtils::loadIcon("prevframe"));
    buttonBack->setToolTip(i18n("Back"));
    buttonBack->setIconSize(QSize(22, 22));
    buttonBack->setFocusPolicy(Qt::NoFocus);
    buttonBack->setAutoRaise(true);
    layout->addWidget(buttonBack);
    connect(buttonBack, SIGNAL(released()), this, SIGNAL(back()));

    buttonStop = new QToolButton(this);
    buttonStop->setIcon(KisIconUtils::loadIcon("animation_stop"));
    buttonStop->setToolTip(i18n("Stop"));
    buttonStop->setIconSize(QSize(22, 22));
    buttonStop->setFocusPolicy(Qt::NoFocus);
    buttonStop->setAutoRaise(true);
    layout->addWidget(buttonStop);
    connect(buttonStop, SIGNAL(released()), this, SIGNAL(stop()));

    buttonPlayPause = new QToolButton(this);
    buttonPlayPause->setIcon(KisIconUtils::loadIcon("animation_play"));
    buttonPlayPause->setToolTip(i18n("Play/Pause"));
    buttonPlayPause->setIconSize(QSize(22, 22));
    buttonPlayPause->setFocusPolicy(Qt::NoFocus);
    buttonPlayPause->setAutoRaise(true);
    layout->addWidget(buttonPlayPause);
    connect(buttonPlayPause, SIGNAL(released()), this, SIGNAL(playPause()));

    buttonForward = new QToolButton(this);
    buttonForward->setIcon(KisIconUtils::loadIcon("nextframe"));
    buttonForward->setToolTip(i18n("Forward"));
    buttonForward->setIconSize(QSize(22, 22));
    buttonForward->setFocusPolicy(Qt::NoFocus);
    buttonForward->setAutoRaise(true);
    layout->addWidget(buttonForward);
    connect(buttonForward, SIGNAL(released()), this, SIGNAL(forward()));

    buttonSkipForward = new QToolButton(this);
    buttonSkipForward->setIcon(KisIconUtils::loadIcon("nextkeyframe"));
    buttonSkipForward->setToolTip(i18n("Skip Forward"));
    buttonSkipForward->setIconSize(QSize(22, 22));
    buttonSkipForward->setFocusPolicy(Qt::NoFocus);
    buttonSkipForward->setAutoRaise(true);
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
