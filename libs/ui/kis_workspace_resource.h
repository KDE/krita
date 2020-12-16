/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_WORKSPACE_RESOURCE_H
#define KIS_WORKSPACE_RESOURCE_H

#include <KoResource.h>
#include <kis_properties_configuration.h>
#include "kritaui_export.h"

/// Resource for storing of workspaces
class KRITAUI_EXPORT KisWorkspaceResource : public KoResource , public KisPropertiesConfiguration
{

public:
    KisWorkspaceResource(const QString& filename);
    ~KisWorkspaceResource() override;
    KisWorkspaceResource(const KisWorkspaceResource &rhs);
    KisWorkspaceResource &operator=(const KisWorkspaceResource &rhs) = delete;
    KoResourceSP clone() const override;

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool saveToDevice(QIODevice* dev) const override;
    QString defaultFileExtension() const override;
    QPair<QString, QString> resourceType() const override
    {
        return QPair<QString, QString>(ResourceType::Workspaces, "");
    }

    void setDockerState(const QByteArray& state);
    QByteArray dockerState();

private:
    QByteArray m_dockerState;
};

typedef QSharedPointer<KisWorkspaceResource> KisWorkspaceResourceSP;

#endif // KIS_WORKSPACE_RESOURCE_H
