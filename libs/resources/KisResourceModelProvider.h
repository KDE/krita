/*
 *  Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KISRESOURCEMODELPROVIDER_H
#define KISRESOURCEMODELPROVIDER_H

#include <qglobal.h>

#include "kritaresources_export.h"

class KisResourceModel;
class KisTagModel;
class KisTagResourceModel;

/**
 * KisResourceModelProvider should be used to retrieve resource models.
 * For every resource type, there is only one instance of the resource model,
 * so all views on these models show the same state.
 */ 
class KRITARESOURCES_EXPORT KisResourceModelProvider
{
public:
    KisResourceModelProvider();
    ~KisResourceModelProvider();

    static KisResourceModel *resourceModel(const QString &resourceType);
    static KisTagModel *tagModel(const QString& resourceType);
    static KisTagResourceModel *tagResourceModel(const QString& resourceType);

private:

    struct Private;
    Private *const d;

    Q_DISABLE_COPY(KisResourceModelProvider)
};

#endif // KISRESOURCEMODELPROVIDER_H
