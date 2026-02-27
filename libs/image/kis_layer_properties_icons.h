/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LAYER_PROPERTIES_ICONS_H
#define __KIS_LAYER_PROPERTIES_ICONS_H

#include <QScopedPointer>
#include <KoID.h>

#include <kis_base_node.h>
#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisLayerPropertiesIcons
{
public:
    KisLayerPropertiesIcons();
    ~KisLayerPropertiesIcons();

    static const KoID locked;
    static const KoID visible;
    static const KoID layerStyle;
    static const KoID inheritAlpha;
    static const KoID alphaLocked;
    static const KoID onionSkins;
    static const KoID passThrough;
    static const KoID selectionActive;
    static const KoID colorLabelIndex;
    static const KoID colorOverlay;
    static const KoID colorizeNeedsUpdate;
    static const KoID colorizeEditKeyStrokes;
    static const KoID colorizeShowColoring;
    static const KoID openFileLayerFile;
    static const KoID layerError;
    static const KoID layerColorSpaceMismatch;
    static const KoID antialiased;

    static KisLayerPropertiesIcons* instance();

    static KisBaseNode::Property getProperty(const KoID &id, bool state);
    static KisBaseNode::Property getProperty(const KoID &id, bool state,
                                              bool isInStasis, bool stateInStasis);

    static KisBaseNode::Property getErrorProperty(const QString &message);
    static KisBaseNode::Property getColorSpaceMismatchProperty(const KoColorSpace *cs);

    /**
     * Sets the specified property of the node and updates it
     */
    static void setNodePropertyAutoUndo(KisNodeSP node, const KoID &id, const QVariant &value, KisImageSP image);
    static void setNodeProperty(KisBaseNode::PropertyList *props, const KoID &id, const QVariant &value);

    /**
     * Gets the specified property of the node
     */
    static QVariant nodeProperty(KisNodeSP node, const KoID &id, const QVariant &defaultValue);

    void updateIcons();

    /**
     * Check if the property is actually stateless, i.e.
     * cannot be modified by a requesting change. These
     * properties are used for things like errors, color
     * space mismatch requests or colorize mask updates.
     *
     * For such properties we should toggle them, but do
     * not expect the value to actually change. That also
     * means that they don't need undo/redo functionality.
     */
    static bool isStatelessProperty(const QString &id);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_LAYER_PROPERTIES_ICONS_H */
