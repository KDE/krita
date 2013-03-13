/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_ACTION_H
#define KIS_ACTION_H

#include <KAction>
#include <krita_export.h>
#include <kdebug.h>

class KisActionManager;

class KRITAUI_EXPORT KisAction : public KAction
{
    Q_OBJECT
public:
    enum ActivationFlag {
        NONE = 0,
        ACTIVE_NODE = 1,
        ACTIVE_DEVICE = 2,
        ACTIVE_LAYER = 4,
        ACTIVE_SHAPE_LAYER = 8,
        PIXELS_SELECTED = 16,
        SHAPES_SELECTED = 32,
        PIXEL_SELECTION_WITH_PIXELS = 64,
        PIXELS_IN_CLIPBOARD = 128,
        SHAPES_IN_CLIPBOARD = 256,
        NEVER_ACTIVATE = 512
    };
    Q_DECLARE_FLAGS(ActivationFlags, ActivationFlag)

    enum ActivationCondition {
        NO_CONDITION = 0,
        ACTIVE_NODE_EDITABLE = 1,
        SELECTION_EDITABLE = 2
    };
    Q_DECLARE_FLAGS(ActivationConditions, ActivationCondition)
    
    explicit KisAction(QObject* parent);
    KisAction(const QString& text, QObject* parent);
    KisAction(const KIcon& icon, const QString& text, QObject* parent);
    virtual ~KisAction();

    void setActivationFlags(ActivationFlags flags);
    ActivationFlags activationFlags();

    void setActivationConditions(ActivationConditions conditions);
    ActivationConditions activationConditions();

    virtual void setActionEnabled(bool enabled);

   /**
    * Set the action manager. Only used by KisActionManager
    */
    void setActionManager(KisActionManager* actionManager);

   /**
    * Set operation id. This will used to run an operation in the KisActionManager
    */
    void setOperationID(const QString& id);

private slots:
    void slotTriggered();

private:
    class Private;
    Private* const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisAction::ActivationFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisAction::ActivationConditions)


#endif // KIS_ACTION_H
