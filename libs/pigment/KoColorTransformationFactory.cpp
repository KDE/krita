/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "KoColorTransformationFactory.h"

struct Q_DECL_HIDDEN KoColorTransformationFactory::Private {
    QString id;
};

KoColorTransformationFactory::KoColorTransformationFactory(const QString &id)
    : d(new Private)
{
    d->id = id;
}

KoColorTransformationFactory::~KoColorTransformationFactory()
{
    delete d;
}

QString KoColorTransformationFactory::id() const
{
    return d->id;
}
