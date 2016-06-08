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

#include "kis_size_group.h"
#include "kis_size_group_p.h"


KisSizeGroup::KisSizeGroup(QObject *parent, KisSizeGroup::mode mode, bool ignoreHidden)
    : QObject(parent)
    , d(new KisSizeGroupPrivate(this, mode, ignoreHidden))
{}

KisSizeGroup::~KisSizeGroup()
{
    delete d;
}

void KisSizeGroup::setMode(KisSizeGroup::mode mode)
{
    if (d->m_mode != mode) {
        d->m_mode = mode;
        d->scheduleSizeUpdate();
    }
}

KisSizeGroup::mode KisSizeGroup::getMode() const
{
    return d->m_mode;
}

void KisSizeGroup::setIgnoreHidden(bool ignoreHidden)
{
    if (d->m_ignoreHidden != ignoreHidden) {
        d->m_ignoreHidden = ignoreHidden;
        d->scheduleSizeUpdate();
    }
}

bool KisSizeGroup::isIgnoreHidden() const
{
    return d->m_ignoreHidden;
}

void KisSizeGroup::addWidget(QWidget *widget)
{
    d->addWidget(widget);
    d->scheduleSizeUpdate();
}

void KisSizeGroup::removeWidget(QWidget *widget)
{
    d->removeWidget(widget);
    d->scheduleSizeUpdate();
}
