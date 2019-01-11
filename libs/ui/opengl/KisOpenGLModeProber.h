#ifndef KISOPENGLMODEPROBER_H
#define KISOPENGLMODEPROBER_H

#include "kritaui_export.h"
#include "kis_config.h"
#include <QSurfaceFormat>

class KoColorProfile;

class KRITAUI_EXPORT KisOpenGLModeProber
{
public:
    KisOpenGLModeProber();
    ~KisOpenGLModeProber();

    static KisOpenGLModeProber* instance();

    bool useHDRMode() const;
    QSurfaceFormat surfaceformatInUse() const;

    const KoColorProfile *rootSurfaceColorProfile() const;

public:
    static void initSurfaceFormatFromConfig(KisConfig::RootSurfaceFormat config,
                                            QSurfaceFormat *format);
    static bool isFormatHDR(const QSurfaceFormat &format);
};

#endif // KISOPENGLMODEPROBER_H
