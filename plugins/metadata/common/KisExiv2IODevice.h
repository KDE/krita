/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KISEXIV2IODEVICE_H_
#define __KISEXIV2IODEVICE_H_

#include <QFile>

#include <exiv2/exiv2.hpp>

#include <kis_global.h>

#include "kritaexifcommon_export.h"

class KRITAEXIFCOMMON_EXPORT KisExiv2IODevice : public Exiv2::BasicIo
{
public:
    using ptr_type = Exiv2::BasicIo::AutoPtr;

    KisExiv2IODevice(QString path);
    ~KisExiv2IODevice() override;

    int open() override;
    int close() override;
    long write(const Exiv2::byte *data, long wcount) override;
    long write(Exiv2::BasicIo &src) override;
    int putb(Exiv2::byte data) override;
    Exiv2::DataBuf read(long rcount) override;
    long read(Exiv2::byte *buf, long rcount) override;
    int getb() override;
    void transfer(BasicIo &src) override;
#if defined(_MSC_VER)
    int seek(int64_t offset, Position pos) override;
#else
    int seek(long offset, Position pos) override;
#endif

    Exiv2::byte *mmap(bool isWriteable = false) override;
    int munmap() override;
    long tell() const override;
    size_t size() const override;
    bool isopen() const override;
    int error() const override;
    bool eof() const override;
    std::string path() const override;

private:
    bool open(QFile::OpenMode mode);
    bool renameToCurrent(const QString srcPath);
    QString filePathQString() const;

    mutable QFile m_file;

    Exiv2::byte *m_mappedArea;
};

#endif // __KISEXIV2IODEVICE_H_
