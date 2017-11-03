/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier<griffinvalley@mail.com>
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
#ifndef KIS_CHANGE_FILE_LAYER_COMMAND_H
#define KIS_CHANGE_FILE_LAYER_COMMAND_H
#include <kundo2command.h>
#include "kis_types.h"
#include "kis_file_layer.h"
class KisChangeFileLayerCmd : public KUndo2Command
{

public:
    KisChangeFileLayerCmd(KisFileLayerSP fileLayer,
                          const QString &oldPath,
                          const QString &oldFileName,
                          const KisFileLayer::ScalingMethod &oldMethod,
                          const QString &newPath,
                          const QString &newFileName,
                          const KisFileLayer::ScalingMethod &newMethod)
        : KUndo2Command(kundo2_i18n("Change File Layer")) {
        m_node = fileLayer;

        m_oldPath = oldPath;
        m_newPath = newPath;
        m_oldFileName = oldFileName;
        m_newFileName = newFileName;
        m_oldMethod = oldMethod;
        m_newMethod = newMethod;
    }
public:
    void redo() override {
        // setFileName() automatically issues a setDirty call
        m_node->setScalingMethod(m_newMethod);
        m_node->setFileName(m_newPath, m_newFileName);
    }

    void undo() override {
        // setFileName() automatically issues a setDirty call
        m_node->setScalingMethod(m_oldMethod);
        m_node->setFileName(m_oldPath, m_oldFileName);
    }
private:
    KisFileLayerSP m_node;

    QString m_oldPath;
    QString m_newPath;
    QString m_oldFileName;
    QString m_newFileName;
    KisFileLayer::ScalingMethod m_oldMethod;
    KisFileLayer::ScalingMethod m_newMethod;
};
#endif // KIS_CHANGE_FILE_LAYER_COMMAND_H
