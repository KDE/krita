/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_min_cut_worker.h"

struct KisMinCutWorker::Private
{
};

KisMinCutWorker::KisMinCutWorker()
    : m_d(new Private)
{
}

KisMinCutWorker::~KisMinCutWorker()
{
}
