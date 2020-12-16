/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CROP_SELECTIONS_PROCESSING_VISITOR_H
#define __KIS_CROP_SELECTIONS_PROCESSING_VISITOR_H

#include <QScopedPointer>

#include "kis_do_nothing_processing_visitor.h"

class QRect;
class KisCropProcessingVisitor;


class KisCropSelectionsProcessingVisitor : public KisDoNothingProcessingVisitor
{
public:
    KisCropSelectionsProcessingVisitor(const QRect &rect);
    ~KisCropSelectionsProcessingVisitor() override;

    using KisDoNothingProcessingVisitor::visit;
    void visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter) override;

private:
    QScopedPointer<KisCropProcessingVisitor> m_cropVisitor;
};

#endif /* __KIS_CROP_SELECTIONS_PROCESSING_VISITOR_H */
