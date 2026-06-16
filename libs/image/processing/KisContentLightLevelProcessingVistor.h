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
#include <kis_hdr_metadata.h>
#include "kritaimage_export.h"

/**
 * @brief The KisContentLightLevelProcessingVistor class
 * This processing visitor checks the projection of the root level node,
 * and calculates the Content Light Level Information for it.
 */
class KRITAIMAGE_EXPORT KisContentLightLevelProcessingVistor : public KisDoNothingProcessingVisitor
{

public:
    KisContentLightLevelProcessingVistor(KisRelativeContentLightLevelInformation::CalculationType type = KisRelativeContentLightLevelInformation::XYZLuminance);
    ~KisContentLightLevelProcessingVistor() override;

    using KisDoNothingProcessingVisitor::visit;
    void visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter) override;

    KisRelativeContentLightLevelInformation contentLightLevelInformation() const;

private:
    KisRelativeContentLightLevelInformation calculateForDev(KisPaintDeviceSP dev);
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KISCONTENTLIGHTLEVELPROCESSINGVISTOR_H
