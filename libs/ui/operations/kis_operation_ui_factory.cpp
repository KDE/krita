/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_operation_ui_factory.h"

class Q_DECL_HIDDEN KisOperationUIFactory::Private {

public:
    Private() {}
    QString id;
};

KisOperationUIFactory::KisOperationUIFactory(const QString& id): d(new KisOperationUIFactory::Private)
{
    d->id = id;
}

KisOperationUIFactory::~KisOperationUIFactory()
{
    delete d;
}

QString KisOperationUIFactory::id() const
{
    return d->id;
}


