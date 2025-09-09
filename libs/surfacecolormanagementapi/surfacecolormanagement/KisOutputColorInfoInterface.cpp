/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisOutputColorInfoInterface.h"

KisOutputColorInfoInterface::KisOutputColorInfoInterface(QObject *parent)
    : QObject(parent)
{
}

KisOutputColorInfoInterface::~KisOutputColorInfoInterface()
{
}

#include <moc_KisOutputColorInfoInterface.cpp>