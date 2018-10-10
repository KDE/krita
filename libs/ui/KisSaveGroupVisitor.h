/*
 *  Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KISSAVEGROUPVISITOR_H
#define KISSAVEGROUPVISITOR_H

#include "kritaui_export.h"

#include <QUrl>
#include <QString>

#include <kis_types.h>
#include <kis_node_visitor.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_node.h>
#include <kis_image.h>


/**
 * @brief The KisSaveGroupVisitor class saves the groups in
 * a Krita image to separate images.
 */
class KRITAUI_EXPORT KisSaveGroupVisitor : public KisNodeVisitor
{
public:

    /**
     * Create a KisSaveGroupVisitor
     *
     * @param image: the image to save
     * @param saveInvisible: also save invisible layers
     * @param saveTopLevelOnly: if true, only save the toplevel layers, otherwise
     *        descend into groups and save the bottom-most groups (groups that do
     *        not contain another group.
     * @param url the base location where the images will be saved
     * @param baseName the basename of the images
     * @param extension the file format extension
     * @param mimeFilter the export image type
     */
    KisSaveGroupVisitor(KisImageWSP image,
                        bool saveInvisible,
                        bool saveTopLevelOnly,
                        const QString &path,
                        const QString &baseName,
                        const QString &extension,
                        const QString &mimeFilter);

    ~KisSaveGroupVisitor() override;

public:

    bool visit(KisNode* ) override;

    bool visit(KisPaintLayer *) override;

    bool visit(KisAdjustmentLayer *) override;

    bool visit(KisExternalLayer *) override;

    bool visit(KisCloneLayer *) override;

    bool visit(KisFilterMask *) override;

    bool visit(KisTransformMask *) override;

    bool visit(KisTransparencyMask *) override;

    bool visit(KisGeneratorLayer * ) override;

    bool visit(KisSelectionMask* ) override;

    bool visit(KisColorizeMask* ) override;

    bool visit(KisGroupLayer *layer) override;

private:

    KisImageWSP m_image;
    bool m_saveInvisible;
    bool m_saveTopLevelOnly;
    QString m_path;
    QString m_baseName;
    QString m_extension;
    QString m_mimeFilter;
};


#endif // KISSAVEGROUPVISITOR_H
