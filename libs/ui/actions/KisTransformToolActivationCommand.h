/*
 *  SPDX-FileCopyrightText: 2018 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTRANSFORMTOOLACTIVATIONCOMMAND_H
#define KISTRANSFORMTOOLACTIVATIONCOMMAND_H

#include <QObject>
#include <kritaui_export.h>
#include <kundo2command.h>
#include "KisViewManager.h"

class KRITAUI_EXPORT KisTransformToolActivationCommand :  public QObject, public KUndo2Command
{
     Q_OBJECT
public:
    KisTransformToolActivationCommand(KisViewManager* view, KUndo2Command * parent = 0);
    ~KisTransformToolActivationCommand() override;

    void redo() override;
    void undo() override;


Q_SIGNALS:
    void requestTransformTool();

    
private:
    bool m_firstRedo;
    KisViewManager* m_view;
};

#endif // KISTRANSFORMTOOLACTIVATIONCOMMAND_H
