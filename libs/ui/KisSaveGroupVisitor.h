/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
     * @param path the base location where the images will be saved
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
