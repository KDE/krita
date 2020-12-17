/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@mail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
