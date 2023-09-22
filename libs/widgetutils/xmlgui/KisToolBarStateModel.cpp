/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisToolBarStateModel.h"

KisToolBarStateModel::KisToolBarStateModel()
    : m_toolBarsLocked{true}
    , LAGER_QT(toolBarsLocked) {m_toolBarsLocked}
{
}
