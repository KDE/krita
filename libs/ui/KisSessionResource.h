/*
 *  SPDX-FileCopyrightText: 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSESSIONRESOURCE_H
#define KISSESSIONRESOURCE_H

#include "KisWindowLayoutResource.h"

#include "kritaui_export.h"

class KRITAUI_EXPORT KisSessionResource : public KisWindowLayoutResource
{
public:
    KisSessionResource(const QString &filename);
    ~KisSessionResource();
    KisSessionResource(const KisSessionResource &rhs);
    KisSessionResource &operator=(const KisSessionResource &rhs) = delete;
    KoResourceSP clone() const override;

    void storeCurrentWindows();


    QString defaultFileExtension() const override;

protected:
    void saveXml(QDomDocument &doc, QDomElement &root) const override;

    void loadXml(const QDomElement &root) const override;

    QPair<QString, QString> resourceType() const override
    {
        return QPair<QString, QString>(ResourceType::Sessions, "");
    }

private:

    // Only KisPart should be able to call restore() to make sure it contains the pointer to it
    void restore();
    friend class KisPart;

    struct Private;
    QScopedPointer<Private> d;
};


typedef QSharedPointer<KisSessionResource> KisSessionResourceSP;

#endif
