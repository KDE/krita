#include "KisSelectionBasedProcessingHelper.h"

#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_transaction_based_command.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"

KisSelectionBasedProcessingHelper::KisSelectionBasedProcessingHelper(KisSelectionSP selection, Functor func)
    : m_selection(selection),
      m_func(func)
{
}

void KisSelectionBasedProcessingHelper::setSelection(KisSelectionSP selection)
{
    m_selection = selection;
}

KUndo2Command *KisSelectionBasedProcessingHelper::createInitCommand(Functor func)
{
    if (!m_selection) return 0;

    struct ProcessSelectionCommand : KisTransactionBasedCommand {
        ProcessSelectionCommand(KisSelectionSP selection,
                                KisSelectionSP cutSelection,
                                std::function<void(KisPaintDeviceSP)> func)
            : m_selection(selection),
              m_cutSelection(cutSelection),
              m_func(func)
        {
        }

        KUndo2Command* paint() {
            m_cutSelection->pixelSelection()->makeCloneFromRough(m_selection->pixelSelection(), m_selection->selectedRect());

            KisTransaction t(m_selection->pixelSelection());
            m_func(m_selection->pixelSelection());

            return t.endAndTake();
        }

        KisSelectionSP m_selection;
        KisSelectionSP m_cutSelection;
        Functor m_func;
    };

    m_cutSelection = new KisSelection();
    return new ProcessSelectionCommand(m_selection, m_cutSelection, func);
}

KUndo2Command *KisSelectionBasedProcessingHelper::createInitCommand()
{
    return createInitCommand(m_func);
}

void KisSelectionBasedProcessingHelper::transformPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter)
{
    transformPaintDevice(device, undoAdapter, m_func);
}

void KisSelectionBasedProcessingHelper::transformPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter, Functor func)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!!m_selection == !!m_cutSelection);

    if (m_selection && m_cutSelection) {
        // we have already processed the selection in the init command so try to skip it
        if (device != static_cast<KisPaintDevice*>(m_selection->pixelSelection().data())) {
            KisTransaction transaction(device);

            const QRect cutBounds = m_cutSelection->selectedExactRect();
            const QRect pasteBounds = m_selection->selectedExactRect();


            KisPaintDeviceSP tempDev = new KisPaintDevice(device->colorSpace());
            tempDev->makeCloneFromRough(device, cutBounds);

            func(tempDev);

            device->clearSelection(m_cutSelection);
            KisPainter::copyAreaOptimized(pasteBounds.topLeft(), tempDev, device, pasteBounds, m_selection);
            transaction.commit(undoAdapter);

        }
    } else {
        KisTransaction transaction(device);
        func(device);
        transaction.commit(undoAdapter);
    }
}
