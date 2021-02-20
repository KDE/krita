/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisDeleteLaterWrapper.h"

#include <QApplication>


void KisDeleteLaterWrapperPrivate::moveToGuiThread(QObject *object)
{
    object->moveToThread(qApp->thread());
}
