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

#include <kaction.h>
#include <krita_export.h>
#include <kdebug.h>

class KisActionManager;

class KRITAUI_EXPORT KisAction : public KAction
{
    Q_OBJECT
public:
    enum ActivationFlag {
        NONE = 0,
        ACTIVE_NODE = 0x1,
        ACTIVE_DEVICE = 0x2,
        ACTIVE_LAYER = 0x4,
        ACTIVE_TRANSPARENCY_MASK = 0x8,
        ACTIVE_SHAPE_LAYER = 0x10,
        PIXELS_SELECTED = 0x20,
        SHAPES_SELECTED = 0x40,
        PIXEL_SELECTION_WITH_PIXELS = 0x80,
        PIXELS_IN_CLIPBOARD = 0x100,
        SHAPES_IN_CLIPBOARD = 0x200,
        NEVER_ACTIVATE = 0x400
    };
    Q_DECLARE_FLAGS(ActivationFlags, ActivationFlag)

    enum ActivationCondition {
        NO_CONDITION = 0,
        ACTIVE_NODE_EDITABLE = 0x1,
        ACTIVE_NODE_EDITABLE_PAINT_DEVICE = 0x2,
        SELECTION_EDITABLE = 0x4
    };
    Q_DECLARE_FLAGS(ActivationConditions, ActivationCondition)
    
    explicit KisAction(QObject* parent = 0);
    KisAction(const QString& text, QObject* parent = 0);
    KisAction(const KIcon& icon, const QString& text, QObject* parent = 0);
    virtual ~KisAction();

    void setActivationFlags(ActivationFlags flags);
    ActivationFlags activationFlags();

    void setActivationConditions(ActivationConditions conditions);
    ActivationConditions activationConditions();

    void setExcludedNodeTypes(const QStringList &nodeTypes);
    const QStringList& excludedNodeTypes() const;

    virtual void setActionEnabled(bool enabled);

   /**
    * Set the action manager. Only used by KisActionManager
    */
    void setActionManager(KisActionManager* actionManager);

   /**
    * Set operation id. This will used to run an operation in the KisActionManager
    */
    void setOperationID(const QString& id);

signals:
    void sigEnableSlaves(bool value);

private slots:
    void slotTriggered();
    void slotChanged();

private:
    class Private;
    Private* const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisAction::ActivationFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisAction::ActivationConditions)


#endif // KIS_ACTION_H
