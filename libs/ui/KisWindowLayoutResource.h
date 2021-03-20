/*
 *  SPDX-FileCopyrightText: 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISWINDOWLAYOUTRESOURCE_H
#define KISWINDOWLAYOUTRESOURCE_H

#include <KoResource.h>
#include <KisMainWindow.h>

class KisWindowLayoutResource;
typedef QSharedPointer<KisWindowLayoutResource> KisWindowLayoutResourceSP;

#include "kritaui_export.h"

class KRITAUI_EXPORT KisWindowLayoutResource : public KoResource
{
public:
    explicit KisWindowLayoutResource(const QString &filename);
    ~KisWindowLayoutResource() override;
    KisWindowLayoutResource(const KisWindowLayoutResource &rhs);
    KisWindowLayoutResource &operator=(const KisWindowLayoutResource &rhs) = delete;
    KoResourceSP clone() const override;

    static KisWindowLayoutResourceSP fromCurrentWindows (
            const QString &filename, const QList<QPointer<KisMainWindow>> &mainWindows,
            bool showImageInAllWindows,
            bool primaryWorkspaceFollowsFocus,
            KisMainWindow *primaryWindow
            );

    void applyLayout();

    bool saveToDevice(QIODevice *dev) const override;
    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;


    QPair<QString, QString> resourceType() const override
    {
        return QPair<QString, QString>(ResourceType::WindowLayouts, "");
    }

    QString defaultFileExtension() const override;

protected:
    void setWindows(const QList<QPointer<KisMainWindow>> &mainWindows);

    virtual void saveXml(QDomDocument &doc, QDomElement &root) const;

    virtual void loadXml(const QDomElement &root) const;

private:
    struct Private;

    QScopedPointer<Private> d;
};


#endif
