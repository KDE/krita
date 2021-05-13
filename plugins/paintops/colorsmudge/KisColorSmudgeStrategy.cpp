/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisColorSmudgeStrategy.h"

KisColorSmudgeStrategy::KisColorSmudgeStrategy()
        : m_memoryAllocator(new KisOptimizedByteArray::PooledMemoryAllocator())
{
}
