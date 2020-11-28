/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    bool saveIccProfile(KisNode* node, const KoColorProfile *profile);
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

