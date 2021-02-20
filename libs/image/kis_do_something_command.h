/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_DO_SOMETHING_COMMAND_H
#define __KIS_DO_SOMETHING_COMMAND_H

#include <kundo2command.h>

template <template <class T> class DoSomethingOp, class LayerType>
class KisDoSomethingCommand : public KUndo2Command
{
public:
    KisDoSomethingCommand(LayerType layer, bool finalUpdate, KUndo2Command *parentCommand = 0)
        : KUndo2Command(parentCommand),
          m_layer(layer),
          m_finalUpdate(finalUpdate) {}

    void undo() override {
        DoSomethingOp<LayerType> op;
        if (!m_finalUpdate) {
            op(m_layer);
        }
    }

    void redo() override {
        DoSomethingOp<LayerType> op;
        if (m_finalUpdate) {
            op(m_layer);
        }
    }

private:
    LayerType m_layer;
    bool m_finalUpdate;
};

namespace KisDoSomethingCommandOps {

template <class LayerType>
struct ResetOp
{
    void operator() (LayerType layer) {
        layer->resetCache();
    }
};

template <class LayerType>
struct UpdateOp
{
    void operator() (LayerType layer) {
        layer->update();
    }
};

}
#endif /* __KIS_DO_SOMETHING_COMMAND_H */
