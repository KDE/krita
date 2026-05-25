/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCONTENTLIGHTLEVELPROCESSINGVISTOR_H
#define KISCONTENTLIGHTLEVELPROCESSINGVISTOR_H

#include "kis_types.h"
#include <kis_do_nothing_processing_visitor.h>
#include <QScopedPointer>

struct KisContentLightLevelInformation;
/**
 * @brief The KisContentLightLevelProcessingVistor class
 * This processing visitor checks the projection of the root level node,
 * and calculates the Content Light Level Information for it.
 */
class KisContentLightLevelProcessingVistor : public KisDoNothingProcessingVisitor
{

public:
    KisContentLightLevelProcessingVistor();
    ~KisContentLightLevelProcessingVistor() override;

    using KisDoNothingProcessingVisitor::visit;
    void visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter) override;

    KisContentLightLevelInformation contentLightLevelInformation() const;

private:
    KisContentLightLevelInformation calculateForDev(KisPaintDeviceSP dev);
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KISCONTENTLIGHTLEVELPROCESSINGVISTOR_H
