/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisExiv2IODevice.h"

#include <QDebug>
#include <QFileInfo>

KisExiv2IODevice::KisExiv2IODevice(QString path)
    : m_file(path)
    , m_mappedArea(nullptr)
{
}

KisExiv2IODevice::~KisExiv2IODevice()
{
    m_file.close();
}

int KisExiv2IODevice::open()
{
    if (m_file.isOpen()) {
        m_file.close();
    }

    // return zero if successful
    return !m_file.open(QFile::ReadWrite);
}

int KisExiv2IODevice::close()
{
    if (munmap() != 0) {
        return 1;
    }
    m_file.close();
    return 0;
}

#if EXIV2_TEST_VERSION(0,28,0)
size_t KisExiv2IODevice::write(const Exiv2::byte *data, size_t wcount)
#else
long KisExiv2IODevice::write(const Exiv2::byte *data, long wcount)
#endif
{
    if (!m_file.isWritable()) {
        qWarning() << "KisExiv2IODevice: File not open for writing.";
        return 0;
    }
    const qint64 writeCount = m_file.write(reinterpret_cast<const char *>(data), wcount);
    if (writeCount > 0) {
        return writeCount;
    }

    return 0;
}

#if EXIV2_TEST_VERSION(0,28,0)
size_t KisExiv2IODevice::write(Exiv2::BasicIo &src)
#else
long KisExiv2IODevice::write(Exiv2::BasicIo &src)
#endif
{
    if (static_cast<BasicIo *>(this) == &src) {
        return 0;
    }
    if (!src.isopen()) {
        return 0;
    }
    if (!m_file.isWritable()) {
        qWarning() << "KisExiv2IODevice: File not open for writing.";
        return 0;
    }
    Exiv2::byte buffer[4096];
    long readCount = 0;
    long totalWriteCount = 0;
    while ((readCount = src.read(buffer, sizeof(buffer))) != 0) {
        totalWriteCount += write(buffer, readCount);
    }

    return totalWriteCount;
}

int KisExiv2IODevice::putb(Exiv2::byte data)
{
    if (!m_file.isWritable()) {
        qWarning() << "KisExiv2IODevice: File not open for writing.";
        return 0;
    }
    if (m_file.putChar(data)) {
        return data;
    } else {
        return EOF;
    }
}

#if EXIV2_TEST_VERSION(0,28,0)
Exiv2::DataBuf KisExiv2IODevice::read(size_t rcount)
#else
Exiv2::DataBuf KisExiv2IODevice::read(long rcount)
#endif
{
    Exiv2::DataBuf buf(rcount);
#if EXIV2_TEST_VERSION(0,28,0)
    const size_t readCount = read(buf.data(), buf.size());
    buf.resize(readCount);
#else
    const long readCount = read(buf.pData_, buf.size_);
    buf.size_ = readCount;
#endif
    return buf;
}

#if EXIV2_TEST_VERSION(0,28,0)
size_t KisExiv2IODevice::read(Exiv2::byte *buf, size_t rcount)
#else
long KisExiv2IODevice::read(Exiv2::byte *buf, long rcount)
#endif
{
    const qint64 bytesRead = m_file.read(reinterpret_cast<char *>(buf), rcount);
    if (bytesRead > 0) {
        return bytesRead;
    } else {
        qWarning() << "KisExiv2IODevice: Couldn't read file:" << m_file.errorString();
        // some error or EOF
        return 0;
    }
}

int KisExiv2IODevice::getb()
{
    char c;
    if (m_file.getChar(&c)) {
        return c;
    } else {
        return EOF;
    }
}

void KisExiv2IODevice::transfer(Exiv2::BasicIo &src)
{
    bool isFileBased = (dynamic_cast<Exiv2::FileIo *>(&src) || dynamic_cast<KisExiv2IODevice *>(&src));
    bool useFallback = false;

    if (isFileBased) {
        const QString srcPath = QString::fromStdString(src.path());
        // use fallback if copying failed (e.g on Android :( )
        useFallback = !renameToCurrent(srcPath);
    }

    if (!isFileBased || useFallback) {
        const bool wasOpen = isopen();
        const QIODevice::OpenMode oldMode = m_file.openMode();

        // this sets file positioner to the beginning.
        if (src.open() != 0) {
            qWarning() << "KisExiv2IODevice::transfer: Couldn't open src file" << QString::fromStdString(src.path());
            return;
        }

        if (!open(QIODevice::ReadWrite | QIODevice::Truncate)) {
            qWarning() << "KisExiv2IODevice::transfer: Couldn't open dest file" << filePathQString();
            return;
        }
        write(src);
        src.close();

        if (wasOpen) {
            open(oldMode);
        } else {
            close();
        }
    }
}

#if defined(_MSC_VER) || EXIV2_TEST_VERSION(0,28,0)
int KisExiv2IODevice::seek(int64_t offset, Exiv2::BasicIo::Position position)
#else
int KisExiv2IODevice::seek(long offset, Exiv2::BasicIo::Position position)
#endif
{
    qint64 pos = 0;
    switch (position) {
    case Exiv2::BasicIo::beg:
        pos = offset;
        break;
    case Exiv2::BasicIo::cur:
        pos = tell() + offset;
        break;
    case Exiv2::BasicIo::end:
        pos = size() + offset;
        break;
    }
    return m_file.seek(pos);
}

Exiv2::byte *KisExiv2IODevice::mmap(bool isWriteable)
{
    Q_UNUSED(isWriteable);

    if (munmap() != 0) {
        qWarning() << "KisExiv2IODevice::mmap: Couldn't unmap the mapped file";
        return nullptr;
    }

    m_mappedArea = m_file.map(0, size(), QFile::NoOptions);
    if (!m_mappedArea) {
        // We show a warning, but originally we should be throwing an exception.
        qWarning() << "KisExiv2IODevice::mmap: Couldn't map the file" << m_file.fileName();
    }
    return m_mappedArea;
}

int KisExiv2IODevice::munmap()
{
    if (m_mappedArea) {
        bool successful = m_file.unmap(m_mappedArea);
        m_mappedArea = nullptr;
        return !successful;
    }
    return 0;
}

#if EXIV2_TEST_VERSION(0,28,0)
void KisExiv2IODevice::populateFakeData()
{
    return;
}
#endif

#if EXIV2_TEST_VERSION(0,28,0)
size_t KisExiv2IODevice::tell() const
#else
long KisExiv2IODevice::tell() const
#endif
{
    return m_file.pos();
}

size_t KisExiv2IODevice::size() const
{
    if (m_file.isWritable()) {
        m_file.flush();
    }
    return m_file.size();
}

bool KisExiv2IODevice::isopen() const
{
    return m_file.isOpen();
}

int KisExiv2IODevice::error() const
{
    // zero if no error
    return m_file.error() != QFileDevice::NoError;
}

bool KisExiv2IODevice::eof() const
{
    return m_file.atEnd();
}

#if EXIV2_TEST_VERSION(0,28,0)
const std::string& KisExiv2IODevice::path() const noexcept
#else
std::string KisExiv2IODevice::path() const
#endif
{
    return filePathQString().toStdString();
}

bool KisExiv2IODevice::open(QFile::OpenMode mode)
{
    if (m_file.isOpen()) {
        m_file.close();
    }
    return m_file.open(mode);
}

bool KisExiv2IODevice::renameToCurrent(const QString srcPath)
{
    QFile::Permissions permissions = QFile::permissions(filePathQString());
    if (QFile::exists(filePathQString())) {
        QFile::remove(filePathQString());
    }

    if (!QFile(srcPath).rename(filePathQString())) {
        qWarning() << "KisExiv2IODevice:renameToCurrent Couldn't copy file from" << srcPath << "to" << filePathQString();
        return false;
    }
    return QFile(filePathQString()).setPermissions(permissions);
}

QString KisExiv2IODevice::filePathQString() const
{
    return QFileInfo(m_file).absoluteFilePath();
}
