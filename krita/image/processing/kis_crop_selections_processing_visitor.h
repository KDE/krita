/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    ~KisCropSelectionsProcessingVisitor();

    using KisDoNothingProcessingVisitor::visit;
    void visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter);

private:
    QScopedPointer<KisCropProcessingVisitor> m_cropVisitor;
};

#endif /* __KIS_CROP_SELECTIONS_PROCESSING_VISITOR_H */
