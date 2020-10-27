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
