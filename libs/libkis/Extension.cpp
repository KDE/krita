/*
 *  Copyright (c) 2015 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "Extension.h"

#include <QDebug>

Extension::Extension(QObject* parent)
    : QObject(parent)
{
}

Extension::~Extension()
{
}
