/* This file is part of the KDE project
 * Copyright (C) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
    KisWorkspaceResource &operator=(const KisWorkspaceResource &rhs);
    KoResourceSP clone() const override;

    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;
    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;
    QString defaultFileExtension() const override;
    QString resourceType() const override
    {
        return ResourceType::Workspaces;
    }

    void setDockerState(const QByteArray& state);
    QByteArray dockerState();

private:
    QByteArray m_dockerState;
};

typedef QSharedPointer<KisWorkspaceResource> KisWorkspaceResourceSP;

#endif // KIS_WORKSPACE_RESOURCE_H
