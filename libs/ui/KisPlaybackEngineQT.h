#ifndef KISPLAYBACKENGINEQT_H
#define KISPLAYBACKENGINEQT_H

#include "KisPlaybackEngine.h"

#include <kritaui_export.h>

class KRITAUI_EXPORT KisPlaybackEngineQT : public KisPlaybackEngine
{
    Q_OBJECT
public:
    explicit KisPlaybackEngineQT(QObject *parent = nullptr);
};

#endif // KISPLAYBACKENGINEQT_H
