#ifndef KISELAPSEDTIMER_H
#define KISELAPSEDTIMER_H

#include "kritaglobal_export.h"

#include <QElapsedTimer>
#include <QtGlobal>

/** @class KisElapsedTimer
 *  @brief A wrapper for QElapsedTimer that allows for timer carryover. Useful for frame timing.
 *  This class only really exists to try to increase the usefulness of QElapsed timer. Basically,
 *  some support for reseting the clock with some time offset can help us achieve more accurate frame
 *  timing. */
class KRITAGLOBAL_EXPORT KisElapsedTimer
{
public:
    KisElapsedTimer();
    ~KisElapsedTimer();

    void start();
    void restart(int carryover = 0); // Defaults to no carryover, same as QElapsedTimer.
    qint64 elapsed() const noexcept;

    int carryOver() const { return m_carryover; }
    void setCarryOver(int value) { m_carryover = value; }

private:
    QElapsedTimer m_internalTimer;
    qint64 m_carryover;
};

#endif // KISELAPSEDTIMER_H
