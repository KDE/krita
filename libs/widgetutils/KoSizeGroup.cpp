/*
 *  Copyright (C) 2013 Juan Palacios <jpalaciosdev@gmail.com>
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

#include "KoSizeGroup.h"
#include "KoSizeGroupPrivate.h"


KoSizeGroup::KoSizeGroup(QObject *parent, KoSizeGroup::mode mode, bool ignoreHidden)
    : QObject(parent)
    , d(new KoSizeGroupPrivate(this, mode, ignoreHidden))
{}

KoSizeGroup::~KoSizeGroup()
{
    delete d;
}

void KoSizeGroup::setMode(KoSizeGroup::mode mode)
{
    if (d->m_mode != mode) {
        d->m_mode = mode;
        d->scheduleSizeUpdate();
    }
}

KoSizeGroup::mode KoSizeGroup::getMode() const
{
    return d->m_mode;
}

void KoSizeGroup::setIgnoreHidden(bool ignoreHidden)
{
    if (d->m_ignoreHidden != ignoreHidden) {
        d->m_ignoreHidden = ignoreHidden;
        d->scheduleSizeUpdate();
    }
}

bool KoSizeGroup::isIgnoreHidden() const
{
    return d->m_ignoreHidden;
}

void KoSizeGroup::addWidget(QWidget *widget)
{
    d->addWidget(widget);
    d->scheduleSizeUpdate();
}

void KoSizeGroup::removeWidget(QWidget *widget)
{
    d->removeWidget(widget);
    d->scheduleSizeUpdate();
}
