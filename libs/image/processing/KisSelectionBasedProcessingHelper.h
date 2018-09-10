#ifndef KISSELECTIONBASEDPROCESSINGHELPER_H
#define KISSELECTIONBASEDPROCESSINGHELPER_H

#include "kritaimage_export.h"
#include "kis_types.h"

#include <functional>
#include <QRect>

class KisUndoAdapter;


class KRITAIMAGE_EXPORT KisSelectionBasedProcessingHelper
{
public:
    using Functor = std::function<void(KisPaintDeviceSP)>;
public:
    KisSelectionBasedProcessingHelper(KisSelectionSP selection, Functor func);

    void setSelection(KisSelectionSP selection);

    KUndo2Command *createInitCommand();
    KUndo2Command *createInitCommand(Functor func);


    void transformPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter);

    void transformPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter, Functor func);


private:
    KisSelectionSP m_selection;
    KisSelectionSP m_cutSelection;
    Functor m_func;
};

#endif // KISSELECTIONBASEDPROCESSINGHELPER_H
