/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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


#include "kis_filter_selection_operation.h"
#include <commands_new/kis_transaction_based_command.h>
#include <KisViewManager.h>
#include <kis_stroke_job_strategy.h>
#include <kis_selection_filters.h>
#include <kis_pixel_selection.h>
#include <kis_processing_applicator.h>
#include <kis_image.h>
#include <kis_transaction.h>
#include <kis_selection_manager.h>

void KisFilterSelectionOperation::runFilter(KisSelectionFilter* filter, KisViewManager* view, const KisOperationConfiguration& config)
{
    KisSelectionSP selection = view->selection();
    if (!selection) return;

    struct FilterSelection : public KisTransactionBasedCommand {
        FilterSelection(KisImageSP image, KisSelectionSP sel, KisSelectionFilter *filter)
            : m_image(image), m_sel(sel), m_filter(filter) {}
        ~FilterSelection() override { delete m_filter;}
        KisImageSP m_image;
        KisSelectionSP m_sel;
        KisSelectionFilter *m_filter;

        KUndo2Command* paint() override {
            KisPixelSelectionSP mergedSelection = m_sel->pixelSelection();
            KisTransaction transaction(mergedSelection);
            QRect processingRect = m_filter->changeRect(mergedSelection->selectedExactRect(), mergedSelection->defaultBounds());
            m_filter->process(mergedSelection, processingRect);
            mergedSelection->setDirty(processingRect);
            return transaction.endAndTake();
        }
    };

    KisProcessingApplicator *ap = beginAction(view, filter->name());
    ap->applyCommand(new FilterSelection(view->image(), selection, filter),
                     KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);
    endAction(ap, config.toXML());
}
