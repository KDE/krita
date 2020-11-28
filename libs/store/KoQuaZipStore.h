/*
 * SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
