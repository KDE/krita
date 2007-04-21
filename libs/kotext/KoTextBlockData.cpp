/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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

class KoTextBlockData::Private {
public:
    Private() : counterWidth(0), counterSpacing(0), border(0) {}
    ~Private() {
        if(border && border->removeUser() == 0)
            delete border;
    }
    double counterWidth;
    double counterSpacing;
    QString counterText;
    QString partialCounterText;
    QPointF counterPos;
    KoTextBlockBorderData *border;
};

KoTextBlockData::KoTextBlockData()
    : d(new Private())
{
    d->counterWidth = -1.0;
}

KoTextBlockData::~KoTextBlockData() {
    delete d;
}

bool KoTextBlockData::hasCounterData() const {
    return d->counterWidth >= 0 && !d->counterText.isNull();
}

double KoTextBlockData::counterWidth() const {
    return qMax(0., d->counterWidth);
}

void KoTextBlockData::setBorder(KoTextBlockBorderData *border) {
    if(d->border && d->border->removeUser() == 0)
        delete d->border;
    d->border = border;
    if(d->border)
        d->border->addUser();
}

void KoTextBlockData::setCounterWidth(double width) {
    d->counterWidth = width;
}

double KoTextBlockData::counterSpacing() const {
    return d->counterSpacing;
}

void KoTextBlockData::setCounterSpacing(double spacing) {
    d->counterSpacing = spacing;
}

void KoTextBlockData::setCounterText(const QString &text) {
    d->counterText = text;
}

const QString &KoTextBlockData::counterText() const {
    return d->counterText;
}

void KoTextBlockData::setPartialCounterText(const QString &text) {
    d->partialCounterText = text;
}

const QString &KoTextBlockData::partialCounterText() const {
    return d->partialCounterText;
}

void KoTextBlockData::setCounterPosition(QPointF position) {
    d->counterPos = position;
}

const QPointF &KoTextBlockData::counterPosition() const {
    return d->counterPos;
}

KoTextBlockBorderData *KoTextBlockData::border() const {
    return d->border;
}

