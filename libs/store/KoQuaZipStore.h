/*
 * Copyright (C) 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KoQuaZipStore_h
#define KoQuaZipStore_h

#include "KoStore.h"
#include <QScopedPointer>

class QUrl;

class KoQuaZipStore : public KoStore
{
public:
    KoQuaZipStore(const QString & _filename, Mode _mode, const QByteArray & appIdentification,
                  bool writeMimetype = true);

    KoQuaZipStore(QIODevice *dev, Mode mode, const QByteArray & appIdentification,
                  bool writeMimetype = true);

    ~KoQuaZipStore() override;

    void setCompressionEnabled(bool enabled) override;
    qint64 write(const char* _data, qint64 _len) override;

    QStringList directoryList() const override;

protected:
    void init(const QByteArray& appIdentification);
    bool doFinalize() override;
    bool openWrite(const QString& name) override;
    bool openRead(const QString& name) override;
    bool closeWrite() override;
    bool closeRead() override;
    bool enterRelativeDirectory(const QString& dirName) override;
    bool enterAbsoluteDirectory(const QString& path) override;
    bool fileExists(const QString& absPath) const override;

private:
    struct Private;
    const QScopedPointer<Private> dd;
    Q_DECLARE_PRIVATE(KoStore)

};

#endif
