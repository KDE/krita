/*
 *  Copyright (c) 2018 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef KISSESSIONRESOURCE_H
#define KISSESSIONRESOURCE_H

#include "KisWindowLayoutResource.h"

class KisSessionResource : public KisWindowLayoutResource
{
public:
    KisSessionResource(const QString &filename);
    ~KisSessionResource();
    KisSessionResource(const KisSessionResource &rhs);
    KisSessionResource &operator=(const KisSessionResource &rhs);
    KoResourceSP clone() const override;

    void storeCurrentWindows();
    void restore();

    QString defaultFileExtension() const override;

protected:
    void saveXml(QDomDocument &doc, QDomElement &root) const override;

    void loadXml(const QDomElement &root) const override;

    QString resourceType() const override
    {
        return ResourceType::Sessions;
    }

private:
    struct Private;
    QScopedPointer<Private> d;
};


typedef QSharedPointer<KisSessionResource> KisSessionResourceSP;

#endif
