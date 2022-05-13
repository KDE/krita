/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KISFILEUTILS_H
#define KISFILEUTILS_H

#include "kritaglobal_export.h"


class QString;

namespace KisFileUtils {

/**
 * @brief Resolve absolute file path from a file path and base dir
 *
 * If the @p filePath is absolute, just return this path, otherwise
 * try to merge @p baseDir and @p filePath to form an absolute file
 * path
 */
QString KRITAGLOBAL_EXPORT resolveAbsoluteFilePath(const QString &baseDir, const QString &filePath);



/// This percent-encodes characters that commonly cause problems when creating files
QString KRITAGLOBAL_EXPORT sanitizeFileName(QString filename);


}

#endif // KISFILEUTILS_H
