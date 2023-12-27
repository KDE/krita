#ifndef KISROLLINGSUMACCUMULATORWRAPPER_H
#define KISROLLINGSUMACCUMULATORWRAPPER_H

#include <QtGlobal>
#include <QScopedPointer>
#include "kritaglobal_export.h"

/**
 * @brief A simple wrapper class that hides boost includes from QtCreator preventing it
 * from crashing when one adds boost's accumulator into a file
 */

class KRITAGLOBAL_EXPORT KisRollingSumAccumulatorWrapper
{
public:
    /**
     * Create a rolling sum and count accumulator with window \p windowSize
     */
    KisRollingSumAccumulatorWrapper(int windowSize);
    ~KisRollingSumAccumulatorWrapper();

    /**
     * Add \p value to a set of numbers
     */
    void operator()(qreal value);

    /**
     * Get rolling sum of the numbers passed to the operator
     */
    qreal rollingSum() const;


    /**
     * Get the number of elements in the rolling window
     */
    int rollingCount() const;

    /**
     * Reset  accumulator and any stored value
     */
    void reset(int windowSize);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISROLLINGSUMACCUMULATORWRAPPER_H
