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
#if EXIV2_TEST_VERSION(0,28,0)
    using ptr_type = Exiv2::BasicIo::UniquePtr;
#else
    using ptr_type = Exiv2::BasicIo::AutoPtr;
#endif

    KisExiv2IODevice(QString path);
    ~KisExiv2IODevice() override;

    int open() override;
    int close() override;
#if EXIV2_TEST_VERSION(0,28,0)
    size_t write(const Exiv2::byte *data, size_t wcount) override;
    size_t write(Exiv2::BasicIo &src) override;
    int putb(Exiv2::byte data) override;
    Exiv2::DataBuf read(size_t rcount) override;
    size_t read(Exiv2::byte *buf, size_t rcount) override;
#else
    long write(const Exiv2::byte *data, long wcount) override;
    long write(Exiv2::BasicIo &src) override;
    int putb(Exiv2::byte data) override;
    Exiv2::DataBuf read(long rcount) override;
    long read(Exiv2::byte *buf, long rcount) override;
#endif
    int getb() override;
    void transfer(BasicIo &src) override;
#if defined(_MSC_VER) || EXIV2_TEST_VERSION(0,28,0)
    int seek(int64_t offset, Position pos) override;
#else
    int seek(long offset, Position pos) override;
#endif

    Exiv2::byte *mmap(bool isWriteable = false) override;
#if EXIV2_TEST_VERSION(0,28,0)
    void populateFakeData() override;
#endif
    int munmap() override;
#if EXIV2_TEST_VERSION(0,28,0)
    size_t tell() const override;
#else
    long tell() const override;
#endif
    size_t size() const override;
    bool isopen() const override;
    int error() const override;
    bool eof() const override;
#if EXIV2_TEST_VERSION(0,28,0)
    const std::string& path() const noexcept override;
#else
    std::string path() const override;
#endif

private:
    bool open(QFile::OpenMode mode);
    bool renameToCurrent(const QString srcPath);
    QString filePathQString() const;

    mutable QFile m_file;

    Exiv2::byte *m_mappedArea;
};

#endif // __KISEXIV2IODEVICE_H_
