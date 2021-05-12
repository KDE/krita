/*
 * SPDX-FileCopyrightText: 2021 Srirupa Datta <srirupa.sps@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef MultiTransparencyMaskCheck_H
#define MultiTransparencyMaskCheck_H

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>
#include <kis_group_layer.h> 
#include <kis_layer_utils.h>

class MultiTransparencyMaskCheck : public KisExportCheckBase
{
public:

    MultiTransparencyMaskCheck(const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning, true)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning", "The image has <b>more than one transparency mask on a layer</b>. For all layers that have multiple transparency masks, only the rendered result will be saved.");
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {        
        KisNodeSP layer = KisLayerUtils::recursiveFindNode(image->rootLayer(), 
            [] (KisNodeSP node)
            {
                quint32 transparencyMasks = 0;
                KisNodeSP mask;

                for (mask = node->firstChild(); mask != 0; mask = mask->nextSibling()) {
                    if (mask->inherits("KisTransparencyMask")) {
                        transparencyMasks += 1;
                        if (transparencyMasks > 1) {
                            return true;
                        }
                    }
                }
                    
                return false;
            });
        
        return (layer != 0);
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }

};

class MultiTransparencyMaskCheckFactory : public KisExportCheckFactory
{
public:

    MultiTransparencyMaskCheckFactory() {}

    ~MultiTransparencyMaskCheckFactory() override {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new MultiTransparencyMaskCheck(id(), level, customWarning);
    }

    QString id() const override {
        return "MultiTransparencyMaskCheck";
    }
};

#endif // MultiTransparencyMaskrCheck_H
