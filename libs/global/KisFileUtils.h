/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KISFILEUTILS_H
#define KISFILEUTILS_H

#include "kritaglobal_export.h"


class QString;

namespace KritaUtils {

/**
 * @brief Resolve absolute file path from a file path and base dir
 *
 * If the @p filePath is absolute, just return this path, otherwise
 * try to merge @p baseDir and @p filePath to form an absolute file
 * path
 */
QString KRITAGLOBAL_EXPORT resolveAbsoluteFilePath(const QString &baseDir, const QString &filePath);


}

#endif // KISFILEUTILS_H
