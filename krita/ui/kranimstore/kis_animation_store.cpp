/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
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
 */

#include "kis_animation_store.h"
#include <KDebug>

KisAnimationStore::KisAnimationStore(QString filename)
{
    m_zip = new KZip(filename);
    m_currentDir = 0;

    m_zip->open(QIODevice::ReadWrite);

    QByteArray mimetype("x-krita-animation");

    m_zip->writeFile("mimetype", "user", "group", mimetype.data(), mimetype.size());

    m_zip->close();
}

KisAnimationStore::~KisAnimationStore()
{

}

void KisAnimationStore::enterDirectory(QString directory)
{

}

void KisAnimationStore::leaveDirectory()
{

}

void KisAnimationStore::writeDataToFile(QByteArray data, QString filename)
{
    m_zip->open(QIODevice::ReadWrite);

    int length = data.length();
    QByteArray prevData;

    if(m_zip->directory()->entry(filename)) {
        prevData = static_cast<const KZipFileEntry*>(m_zip->directory()->entry(filename))->data();
        length += prevData.length();
    }

    m_zip->prepareWriting(filename, "", "", length);

    m_zip->writeData(prevData.data(), prevData.length());
    m_zip->writeData(data.data(), data.length());

    m_zip->finishWriting(length);

    m_zip->close();
}

void KisAnimationStore::writeDataToFile(const char *data, qint64 length, QString filename)
{
    m_zip->open(QIODevice::ReadWrite);

    int dataLength = length;
    QByteArray prevData;

    if(m_zip->directory()->entry(filename)) {
        prevData = static_cast<const KZipFileEntry*>(m_zip->directory()->entry(filename))->data();
        dataLength += prevData.length();
    }

    m_zip->prepareWriting(filename, "", "", dataLength);

    m_zip->writeData(prevData.data(), prevData.length());
    m_zip->writeData(data, length);

    m_zip->finishWriting(dataLength);

    m_zip->close();
}

void KisAnimationStore::setCompressionEnabled(bool e)
{
    if(e) {
        m_zip->setCompression(KZip::DeflateCompression);
    }
    else {
        m_zip->setCompression(KZip::NoCompression);
    }
}
