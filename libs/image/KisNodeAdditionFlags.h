#ifndef KISNODEADDITIONFLAGS_H
#define KISNODEADDITIONFLAGS_H

#include <QFlags>

enum class KisNodeAdditionFlag
{
    None = 0x0,

    /**
     * The node will not be set as current/selected after addition
     */
    DontActivateNode = 0x1
};

Q_DECLARE_FLAGS(KisNodeAdditionFlags, KisNodeAdditionFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisNodeAdditionFlags)

#endif // KISNODEADDITIONFLAGS_H
