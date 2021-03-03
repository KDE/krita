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
    // TODO: add some slots to reload the icons on theme change
    const QIcon chainIcon = KisIconUtils::loadIcon("chain-icon");
    const QIcon brokenChainIcon = KisIconUtils::loadIcon("chain-broken-icon");
    const QPixmap chain = chainIcon.pixmap(9, 24);
    const QPixmap brokenChain = brokenChainIcon.pixmap(9, 24);
    bool keepAspect;
};

KoAspectButton::KoAspectButton(QWidget *parent)
    : QAbstractButton(parent),
    d( new Private() )
{
    //setPixmap(d->chain);
    //setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}

KoAspectButton::~KoAspectButton()
{
    delete d;
}

void KoAspectButton::mouseReleaseEvent (QMouseEvent *ev) {
    if(! isEnabled() || ev->button() != Qt::LeftButton)
        return;
    setKeepAspectRatio(!d->keepAspect);
}

void KoAspectButton::setKeepAspectRatio(bool on) {
    if(d->keepAspect == on)
        return;
    d->keepAspect = on;
    update();
    emit keepAspectRatioChanged(d->keepAspect);
}

void KoAspectButton::paintEvent (QPaintEvent *) {
    QPainter painter(this);
    painter.drawPixmap(0, (height() - 24) / 2, 9, 24, d->keepAspect ? d->chain : d->brokenChain, 0, 0, 9, 24);
    painter.end();
}

QSize KoAspectButton::sizeHint () const {
    return QSize(9, 24);
}

void KoAspectButton::keyReleaseEvent (QKeyEvent *e) {
    if(e->text() == " ") {
        setKeepAspectRatio(! d->keepAspect);
        e->accept();
    }
}

bool KoAspectButton::keepAspectRatio() const
{
    return d->keepAspect;
}
