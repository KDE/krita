/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include <KisResourceLoader.h>
#include <QDebug>
#include <KisMimeDatabase.h>

/**
 * @return a set of filters ("*.bla,*.foo") that is suitable for filtering
 * the contents of a directory.
 */
QStringList KisResourceLoaderBase::filters() const
{
    QStringList filters;
    Q_FOREACH(const QString &mimeType, mimetypes()) {
        QStringList suffixes = KisMimeDatabase::suffixesForMimeType(mimeType);
        Q_FOREACH(const QString &suffix, suffixes) {
                filters << "*." + suffix;
        }
    }

    return filters;
}
