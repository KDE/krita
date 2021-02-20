/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_stroke_job_strategy.h"

#include <QtGlobal>


KisStrokeJobData::KisStrokeJobData(Sequentiality sequentiality,
                                   Exclusivity exclusivity)
    : m_sequentiality(sequentiality),
      m_exclusivity(exclusivity),
      m_isCancellable(true),
      m_levelOfDetailOverride(-1)
{
}

KisStrokeJobData::KisStrokeJobData(const KisStrokeJobData &rhs)
    : m_sequentiality(rhs.m_sequentiality),
      m_exclusivity(rhs.m_exclusivity),
      m_isCancellable(rhs.m_isCancellable),
      m_levelOfDetailOverride(rhs.m_levelOfDetailOverride)
{
}

KisStrokeJobData::~KisStrokeJobData()
{
}

bool KisStrokeJobData::isBarrier() const
{
    return m_sequentiality == BARRIER;
}

bool KisStrokeJobData::isSequential() const
{
    return m_sequentiality == SEQUENTIAL;
}

bool KisStrokeJobData::isExclusive() const
{
    return m_exclusivity == EXCLUSIVE;
}

KisStrokeJobData* KisStrokeJobData::createLodClone(int levelOfDetail)
{
    Q_UNUSED(levelOfDetail);
    return 0;
}

bool KisStrokeJobData::isCancellable() const
{
    return m_isCancellable;
}

void KisStrokeJobData::setCancellable(bool value)
{
    m_isCancellable = value;
}

int KisStrokeJobData::levelOfDetailOverride() const
{
    return m_levelOfDetailOverride;
}

void KisStrokeJobData::setLevelOfDetailOverride(int value)
{
    m_levelOfDetailOverride = value;
}

KisStrokeJobStrategy::KisStrokeJobStrategy()
{
}

KisStrokeJobStrategy::~KisStrokeJobStrategy()
{
}


