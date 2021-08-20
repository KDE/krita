#include "KisElapsedTimer.h"

KisElapsedTimer::KisElapsedTimer()
    : m_carryover(0)
{

}

KisElapsedTimer::~KisElapsedTimer()
{

}

void KisElapsedTimer::start()
{
    m_internalTimer.start();
}

void KisElapsedTimer::restart(int carryover)
{
    m_carryover = carryover;
    m_internalTimer.restart();
}

qint64 KisElapsedTimer::elapsed() const noexcept
{
    return m_internalTimer.elapsed() + m_carryover;
}


