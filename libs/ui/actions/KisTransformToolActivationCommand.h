/*
 *  Copyright (c) 2018 Sven Langkamp <sven.langkamp@gmail.com>
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
