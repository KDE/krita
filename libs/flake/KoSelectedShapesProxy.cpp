/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSelectedShapesProxy.h"

KoSelectedShapesProxy::KoSelectedShapesProxy(QObject *parent)
    : QObject(parent)
{
}

bool KoSelectedShapesProxy::isRequestingToBeEdited()
{
    return m_isRequestingEditing;
}

void KoSelectedShapesProxy::setRequestingToBeEdited(bool value)
{
    m_isRequestingEditing = value;
}
