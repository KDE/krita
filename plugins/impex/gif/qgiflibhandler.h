/*
 * SPDX-FileCopyrightText: 2009 Shawn T. Rutledge (shawn.t.rutledge@gmail.com)
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef QGIFLIBHANDLER_H
#define QGIFLIBHANDLER_H

#include <QImageIOHandler>
#include <QImage>
#include <QVariant>

class QGIFLibHandler : public QImageIOHandler
{
public:
    QGIFLibHandler();
    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;
    static bool canRead(QIODevice *device);
    bool supportsOption(ImageOption option) const override;
    void setOption(ImageOption option, const QVariant &value) override;
    QVariant option(ImageOption option) const override;

private:
    QString m_description;
};

#endif // QGIFLIBHANDLER_H
