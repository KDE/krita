/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <kis_command_utils.h>
#include "commands/KisDeselectActiveSelectionCommand.h"

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
            KUndo2Command *savedCommand = transaction.endAndTake();
            mergedSelection->setDirty(processingRect);
            if (m_sel->selectedExactRect().isEmpty() || m_sel->pixelSelection()->outline().isEmpty()) {
                KisCommandUtils::CompositeCommand *cmd = new KisCommandUtils::CompositeCommand();
                cmd->addCommand(savedCommand);
                cmd->addCommand(new KisDeselectActiveSelectionCommand(m_sel, m_image));
                savedCommand = cmd;
            }

            return savedCommand;
        }
    };

    KisProcessingApplicator *ap = beginAction(view, filter->name());
    ap->applyCommand(new FilterSelection(view->image(), selection, filter),
                     KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);
    endAction(ap, config.toXML());
}
