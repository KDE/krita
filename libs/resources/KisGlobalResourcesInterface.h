/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KISGLOBALRESOURCESINTERFACE_H
#define KISGLOBALRESOURCESINTERFACE_H

#include "kritaresources_export.h"
#include "KisResourcesInterface.h"

/**
 * @brief the main resource source in Krita
 *
 * This class wraps KisResourceModel into a KisResourcesInterface and provides
 * all Krita resources to consumers.
 *
 * WARNING: this class should never be accessed in non-GUI thread
 */
class KRITARESOURCES_EXPORT KisGlobalResourcesInterface : public KisResourcesInterface
{
public:
    static KisResourcesInterfaceSP instance();

protected:
    ResourceSourceAdapter* createSourceImpl(const QString &type) const override;
};

#endif // KISGLOBALRESOURCESINTERFACE_H
