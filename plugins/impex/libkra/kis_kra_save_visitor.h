/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_KRA_SAVE_VISITOR_H_
#define KIS_KRA_SAVE_VISITOR_H_

#include <QRect>
#include <QStringList>

#include "kis_types.h"
#include "kis_node_visitor.h"
#include "kis_image.h"
#include "kritalibkra_export.h"

class KisPaintDeviceWriter;
class KoStore;

class KRITALIBKRA_EXPORT KisKraSaveVisitor : public KisNodeVisitor
{
public:
    KisKraSaveVisitor(KoStore *store, const QString & name, QMap<const KisNode*, QString> nodeFileNames);
    ~KisKraSaveVisitor() override;
    using KisNodeVisitor::visit;

public:
    void setExternalUri(const QString &uri);

    bool visit(KisNode*) override {
        return true;
    }

    bool visit(KisExternalLayer *) override;

    bool visit(KisPaintLayer *layer) override;

    bool visit(KisGroupLayer *layer) override;

    bool visit(KisAdjustmentLayer* layer) override;

    bool visit(KisGeneratorLayer * layer) override;

    bool visit(KisCloneLayer *layer) override;

    bool visit(KisFilterMask *mask) override;

    bool visit(KisTransformMask *mask) override;

    bool visit(KisTransparencyMask *mask) override;

    bool visit(KisSelectionMask *mask) override;

    bool visit(KisColorizeMask *mask) override;

    /// @return a list with everything that went wrong while saving
    QStringList errorMessages() const;

private:

    bool savePaintDevice(KisPaintDeviceSP device, QString location);

    template<class DevicePolicy>
    bool savePaintDeviceFrame(KisPaintDeviceSP device, QString location, DevicePolicy policy);

    bool saveAnnotations(KisLayer* layer);
    bool saveSelection(KisNode* node);
    bool saveFilterConfiguration(KisNode* node);
    bool saveMetaData(KisNode* node);
    QString getLocation(KisNode* node, const QString& suffix = QString());
    QString getLocation(const QString &filename, const QString &suffix = QString());

private:

    KoStore *m_store;
    bool m_external;
    QString m_uri;
    QString m_name;
    QMap<const KisNode*, QString> m_nodeFileNames;
    KisPaintDeviceWriter *m_writer;
    QStringList m_errorMessages;
};

#endif // KIS_KRA_SAVE_VISITOR_H_

