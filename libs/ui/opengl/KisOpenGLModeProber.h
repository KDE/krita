#ifndef KISOPENGLMODEPROBER_H
#define KISOPENGLMODEPROBER_H

#include "kritaui_export.h"
#include "kis_config.h"
#include <QSurfaceFormat>

class KRITAUI_EXPORT KisOpenGLModeProber
{
public:
    KisOpenGLModeProber();
    ~KisOpenGLModeProber();

    static KisOpenGLModeProber* instance();

    bool useHDRMode() const;
    QSurfaceFormat surfaceformatInUse() const;

public:
    static void initSurfaceFormatFromConfig(KisConfig::RootSurfaceFormat config,
                                            QSurfaceFormat *format);
    static bool isFormatHDR(const QSurfaceFormat &format);
};

#endif // KISOPENGLMODEPROBER_H
