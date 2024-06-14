/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoMultiArchBuildSupport.h>

#include <KConfigGroup>
#include <KSharedConfig>
#include <kis_debug.h>


std::tuple<bool, bool> vectorizationConfiguration()
{
    static const std::tuple<bool, bool> vectorization = [&]() {
        KConfigGroup cfg = KSharedConfig::openConfig()->group("");
        // use the old key name for compatibility
        const bool useVectorization =
            !cfg.readEntry("amdDisableVectorWorkaround", false);
        const bool disableAVXOptimizations =
            cfg.readEntry("disableAVXOptimizations", false);

        return std::make_tuple(useVectorization, disableAVXOptimizations);
    }();

    return vectorization;
}
