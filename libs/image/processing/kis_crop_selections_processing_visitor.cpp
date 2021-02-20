/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_crop_selections_processing_visitor.h"
#include "kis_crop_processing_visitor.h"

KisCropSelectionsProcessingVisitor::KisCropSelectionsProcessingVisitor(const QRect &rect)
    : m_cropVisitor(new KisCropProcessingVisitor(rect, true, false))
{
}

KisCropSelectionsProcessingVisitor::~KisCropSelectionsProcessingVisitor()
{
}

void KisCropSelectionsProcessingVisitor::visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter)
{
    m_cropVisitor->visit(mask, undoAdapter);
}
