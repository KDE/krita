/*
 * SPDX-FileCopyrightText: 2023 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KISANDROIDFILEPROXY_H_
#define __KISANDROIDFILEPROXY_H_

#include <QString>


class KisAndroidFileProxy {
public:

    /**
     * This clones the file to internal storage so we can return it. This is useful for APIs
     * which don't work on file descriptors.
     */
    static QString getFileFromContentUri(QString contentUri);

};


#endif // __KISANDROIDFILEPROXY_H_
