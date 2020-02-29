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
#ifndef KisResourcesInterface_P_H
#define KisResourcesInterface_P_H

#include "kritaresources_export.h"
#include "KisResourcesInterface.h"
#include <unordered_map>
#include <memory>

#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

#include "kis_assert.h"

#if (QT_VERSION <= QT_VERSION_CHECK(5, 12, 0))
namespace std
{
    template<> struct hash<QString>
    {
        std::size_t operator()(const QString &s) const noexcept {
            return qHash(s);
        }
    };
}
#endif

class KRITARESOURCES_EXPORT KisResourcesInterfacePrivate
{
public:
    mutable std::unordered_map<QString,
                       std::unique_ptr<
                           KisResourcesInterface::ResourceSourceAdapter>> sourceAdapters;
    mutable QReadWriteLock lock;

    KisResourcesInterface::ResourceSourceAdapter* findExistingSource(const QString &type) const {
        auto it = this->sourceAdapters.find(type);
        if (it != this->sourceAdapters.end()) {
            KIS_ASSERT(bool(it->second));

            return it->second.get();
        }

        return nullptr;
    }
};

#endif // KisResourcesInterface_P_H
