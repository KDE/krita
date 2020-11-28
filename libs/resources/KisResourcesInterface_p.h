/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

/// added to Qt in 5.14.0
/// https://codereview.qt-project.org/c/qt/qtbase/+/261819

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
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

    virtual ~KisResourcesInterfacePrivate() {}
};

#endif // KisResourcesInterface_P_H
