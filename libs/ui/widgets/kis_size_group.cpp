/*
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
