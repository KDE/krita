/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __MOVE_SELECTION_STROKE_STRATEGY_H
#define __MOVE_SELECTION_STROKE_STRATEGY_H

#include "kis_stroke_strategy_undo_command_based.h"
#include "kis_types.h"
#include "kis_selection.h"
#include "kis_paint_layer.h"

class KisPostExecutionUndoAdapter;
class KisUpdatesFacade;


class MoveSelectionStrokeStrategy : public QObject, public KisStrokeStrategyUndoCommandBased
{
    Q_OBJECT

public:
    struct ShowSelectionData : public KisStrokeJobData
    {
        ShowSelectionData(bool _showSelection)
            : KisStrokeJobData(),
              showSelection(_showSelection)
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override;

        bool showSelection = false;

    protected:
        ShowSelectionData(const ShowSelectionData &rhs, int levelOfDetail);
    };

public:
    MoveSelectionStrokeStrategy(KisPaintLayerSP paintLayer,
                                KisSelectionSP selection,
                                KisUpdatesFacade *updatesFacade,
                                KisStrokeUndoFacade *undoFacade);

    void initStrokeCallback() override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;

Q_SIGNALS:
    void sigHandlesRectCalculated(const QRect &handlesRect);
    void sigStrokeStartedEmpty();

private:
    MoveSelectionStrokeStrategy(const MoveSelectionStrokeStrategy &rhs);

    KisStrokeStrategy* createLodClone(int levelOfDetail) override;

private:
    KisPaintLayerSP m_paintLayer;
    KisSelectionSP m_selection;
    KisUpdatesFacade *m_updatesFacade;
    QPoint m_finalOffset;
    QPoint m_initialDeviceOffset;
    QPoint m_initialSelectionOffset;
};

#endif /* __MOVE_SELECTION_STROKE_STRATEGY_H */
