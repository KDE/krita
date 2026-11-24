/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KISFILEUTILS_H
#define KISFILEUTILS_H

#include "kritaglobal_export.h"
#include <functional>


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

QString KRITAGLOBAL_EXPORT deduplicateFileName(const QString &fileName,
                                               const QString &separator,
                                               std::function<bool(QString)> fileAllowedCallback);

/**
 * Checks if a file or directory at \p targetPath belongs to \p baseDir
 *
 * If any of the paths contain symlinks, the symlinks will be resolved
 * using QFileInfo::canonicalFilePath().
 */
bool KRITAGLOBAL_EXPORT isSubPath(const QString &baseDir, const QString &targetPath);
}

#endif // KISFILEUTILS_H
