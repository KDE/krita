/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisResourceItemChooserSync.h"

#include <QGlobalStatic>

Q_GLOBAL_STATIC(KisResourceItemChooserSync, s_instance)

struct Q_DECL_HIDDEN KisResourceItemChooserSync::Private
{
    int baseLength;
};


KisResourceItemChooserSync::KisResourceItemChooserSync()
    : d(new Private)
{
    d->baseLength = 50;
}

KisResourceItemChooserSync::~KisResourceItemChooserSync()
{
}

KisResourceItemChooserSync* KisResourceItemChooserSync::instance()
{
    return s_instance;
}

int KisResourceItemChooserSync::baseLength()
{
    return d->baseLength;
}

void KisResourceItemChooserSync::setBaseLength(int length)
{
    d->baseLength = qBound(25, length, 100);
    emit baseLengthChanged(d->baseLength);
}







