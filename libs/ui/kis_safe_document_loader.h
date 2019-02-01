/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_SAFE_DOCUMENT_LOADER_H
#define __KIS_SAFE_DOCUMENT_LOADER_H

#include <QObject>
#include "kis_types.h"

class KisSafeDocumentLoader : public QObject
{
    Q_OBJECT
private:
    friend class KisFileLayer;

    KisSafeDocumentLoader(const QString &path = "", QObject *parent = 0);
    ~KisSafeDocumentLoader() override;

    void setPath(const QString &path);
    void reloadImage();
private Q_SLOTS:
    void fileChanged(QString);
    void fileChangedCompressed(bool sync = false);
    void delayedLoadStart();

Q_SIGNALS:
    void loadingFinished(KisPaintDeviceSP paintDevice, int xRes, int yRes);

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_SAFE_DOCUMENT_LOADER_H */
