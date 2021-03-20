/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDecoratedNodeInterface.h"

KisDecoratedNodeInterface::~KisDecoratedNodeInterface()
{
}

void KisDecoratedNodeInterface::setDecorationsVisible(bool value)
{
    setDecorationsVisible(value, true);
}
