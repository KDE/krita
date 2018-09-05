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

#include <QWidgetAction>
#include <kritaui_export.h>
#include <kis_debug.h>
#include <QIcon>
class KisActionManager;



/**
 *  KisAction, inheriting from QWidgetAction, is a convenience class for GUI
 *  actions, with Krita's configuration system and GUI states. A widget like a
 *  "save" button may be enabled/disabled, hidden or shown depending on the
 *  state of the application, e.g. whether the image currently being viewed was
 *  modified since it was opened.
 *
 *  Copies of these actions are created for each MainWindow instance. They are
 *  owned by a KisActionManager, of which there is one for each MainWindow. Most
 *  of these instantiations happen inside the constructor for KisMainWindow as
 *  well as the various functions called in KisViewManager::setupManagers().
 *
 **/

class KRITAUI_EXPORT KisAction : public QWidgetAction
{
    Q_OBJECT
public:

    /**
     * If you re-order these, you must change the associated values in
     * krita.action and kritamenu.action!
     */
    enum ActivationFlag {
        NONE                        = 0x0000, ///< Always activate
        ACTIVE_IMAGE                = 0x0001, ///< Activate if there is at least one image
        MULTIPLE_IMAGES             = 0x0002, ///< Activate if there is more than one image open
        CURRENT_IMAGE_MODIFIED      = 0x0004, ///< Activate if the current image is modified
        ACTIVE_NODE                 = 0x0008, ///< Activate if there's an active node (layer or mask)
        ACTIVE_DEVICE               = 0x0010, ///< Activate if the active node has a paint device, i.e. there are pixels to be modified
        ACTIVE_LAYER                = 0x0020, ///< Activate if the current node is a layer (vector or pixel)
        ACTIVE_TRANSPARENCY_MASK    = 0x0040, ///< Activate if the current node is a transparency mask
        ACTIVE_SHAPE_LAYER          = 0x0080, ///< Activate if the current node is a vector layer
        PIXELS_SELECTED             = 0x0100, ///< Activate if any pixels are selcted (with any kind of selection)
        SHAPES_SELECTED             = 0x0200, ///< Activate if any vector shape is selected
        ANY_SELECTION_WITH_PIXELS   = 0x0400, ///< ???
        PIXELS_IN_CLIPBOARD         = 0x0800, ///< Activate if the clipboard contains pixels
        SHAPES_IN_CLIPBOARD         = 0x1000, ///< Activate if the clipboard contains vector data
        NEVER_ACTIVATE              = 0x2000, ///< ???
        LAYERS_IN_CLIPBOARD         = 0x4000, ///< ???
        IMAGE_HAS_ANIMATION         = 0x8000, ///< Activate if the image has an animation
        SHAPE_SELECTION_WITH_SHAPES = 0x10000, ///< Activate there is a vector selection active
        PIXEL_SELECTION_WITH_PIXELS = 0x20000, ///< Activate there is a raster selection active
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
    KisAction(const QIcon& icon, const QString& text, QObject* parent = 0);
    ~KisAction() override;

    void setDefaultShortcut(const QKeySequence & shortcut);
    QKeySequence defaultShortcut() const;

    void setActivationFlags(ActivationFlags flags);
    ActivationFlags activationFlags();

    void setActivationConditions(ActivationConditions conditions);
    ActivationConditions activationConditions();

    void setExcludedNodeTypes(const QStringList &nodeTypes);
    const QStringList& excludedNodeTypes() const;

    virtual void setActionEnabled(bool enabled);

   /**
    * Set operation id. This will used to run an operation in the KisActionManager
    */
    void setOperationID(const QString& id);

Q_SIGNALS:
    void sigEnableSlaves(bool value);

private Q_SLOTS:
    void slotTriggered();
    void slotChanged();

private:

    friend class KisActionManager;

    /**
     * Set the action manager. Only used by KisActionManager
     */
     void setActionManager(KisActionManager* actionManager);


    class Private;
    Private* const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisAction::ActivationFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisAction::ActivationConditions)


#endif // KIS_ACTION_H
