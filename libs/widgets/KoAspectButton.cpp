/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2005-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KoAspectButton.h"

#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>

#include <kis_icon_utils.h>

class Q_DECL_HIDDEN KoAspectButton::Private
{
public:
    Private()
        : keepAspect(true)
    {
    }
    bool keepAspect;
};

KoAspectButton::KoAspectButton(QWidget *parent)
    : QToolButton(parent),
    d( new Private() )
{
    setIconSize(QSize(9, 24));
    setFixedSize(19, 34);
    setAutoRaise(true);

    connect(this, SIGNAL(released()), this, SLOT(buttonReleased()));

    setKeepAspectRatio(false);
}

KoAspectButton::~KoAspectButton()
{
    delete d;
}

void KoAspectButton::buttonReleased() {
    if(! isEnabled())
        return;
    setKeepAspectRatio(!d->keepAspect);
}

void KoAspectButton::setKeepAspectRatio(bool on) {
    this->setIcon(on ? KisIconUtils::loadIcon("chain-icon") : KisIconUtils::loadIcon("chain-broken-icon"));
    update();

    if(d->keepAspect != on) {
        d->keepAspect = on;
        emit keepAspectRatioChanged(d->keepAspect);
    }
}

bool KoAspectButton::keepAspectRatio() const
{
    return d->keepAspect;
}
