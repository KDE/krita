/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_aspect_ratio_locker.h"

#include <QSpinBox>
#include <KoAspectButton.h>

#include "kis_signals_blocker.h"


struct KisAspectRatioLocker::Private
{
    Private() : aspectRatio(1.0) {}

    QSpinBox *spinOne;
    QSpinBox *spinTwo;
    KoAspectButton *aspectButton;

    qreal aspectRatio;
};


KisAspectRatioLocker::KisAspectRatioLocker(QObject *parent)
    : QObject(parent),
      m_d(new Private)
{
}

KisAspectRatioLocker::~KisAspectRatioLocker()
{
}

void KisAspectRatioLocker::connectSpinBoxes(QSpinBox *spinOne,
                                            QSpinBox *spinTwo,
                                            KoAspectButton *aspectButton)
{
    m_d->spinOne = spinOne;
    m_d->spinTwo = spinTwo;
    m_d->aspectButton = aspectButton;

    connect(m_d->spinOne, SIGNAL(valueChanged(int)), SLOT(slotSpinOneChanged()));
    connect(m_d->spinTwo, SIGNAL(valueChanged(int)), SLOT(slotSpinTwoChanged()));
    connect(m_d->aspectButton, SIGNAL(keepAspectRatioChanged(bool)), SLOT(slotAspectButtonChanged()));
    slotAspectButtonChanged();
}

void KisAspectRatioLocker::slotSpinOneChanged()
{
    if (m_d->aspectButton->keepAspectRatio()) {
        KisSignalsBlocker b(m_d->spinTwo);
        m_d->spinTwo->setValue(qRound(m_d->aspectRatio * m_d->spinOne->value()));
    }
}

void KisAspectRatioLocker::slotSpinTwoChanged()
{
    if (m_d->aspectButton->keepAspectRatio()) {
        KisSignalsBlocker b(m_d->spinOne);
        m_d->spinOne->setValue(qRound(m_d->spinTwo->value() / m_d->aspectRatio));
    }
}

void KisAspectRatioLocker::slotAspectButtonChanged()
{
    if (m_d->aspectButton->keepAspectRatio() &&
        m_d->spinTwo->value() > 0 &&
        m_d->spinOne->value() > 0) {

        m_d->aspectRatio = qreal(m_d->spinTwo->value()) / m_d->spinOne->value();
    } else {
        m_d->aspectRatio = 1.0;
    }
}
