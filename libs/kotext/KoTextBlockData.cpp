/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoTextBlockData.h"
#include "KoTextBlockBorderData.h"

class KoTextBlockData::Private
{
public:
    Private() : counterWidth(0), counterSpacing(0), counterIndex(1), border(0) {}
    ~Private() {
        if (border && !border->deref())
            delete border;
    }
    qreal counterWidth;
    qreal counterSpacing;
    QString counterText;
    QString partialCounterText;
    int counterIndex;
    QPointF counterPos;
    KoTextBlockBorderData *border;
};

KoTextBlockData::KoTextBlockData()
        : d(new Private())
{
    d->counterWidth = -1.0;
}

KoTextBlockData::~KoTextBlockData()
{
    delete d;
}

bool KoTextBlockData::hasCounterData() const
{
    return d->counterWidth >= 0 && !d->counterText.isNull();
}

qreal KoTextBlockData::counterWidth() const
{
    return qMax(qreal(0), d->counterWidth);
}

void KoTextBlockData::setBorder(KoTextBlockBorderData *border)
{
    if (d->border && !d->border->deref())
        delete d->border;
    d->border = border;
    if (d->border)
        d->border->ref();
}

void KoTextBlockData::setCounterWidth(qreal width)
{
    d->counterWidth = width;
}

qreal KoTextBlockData::counterSpacing() const
{
    return d->counterSpacing;
}

void KoTextBlockData::setCounterSpacing(qreal spacing)
{
    d->counterSpacing = spacing;
}

void KoTextBlockData::setCounterText(const QString &text)
{
    d->counterText = text;
}

QString KoTextBlockData::counterText() const
{
    return d->counterText;
}

void KoTextBlockData::setPartialCounterText(const QString &text)
{
    d->partialCounterText = text;
}

QString KoTextBlockData::partialCounterText() const
{
    return d->partialCounterText;
}

void KoTextBlockData::setCounterIndex(int index)
{
    d->counterIndex = index;
}

int KoTextBlockData::counterIndex() const
{
    return d->counterIndex;
}

void KoTextBlockData::setCounterPosition(const QPointF &position)
{
    d->counterPos = position;
}

QPointF KoTextBlockData::counterPosition() const
{
    return d->counterPos;
}

KoTextBlockBorderData *KoTextBlockData::border() const
{
    return d->border;
}

