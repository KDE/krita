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

void KisAnimationStore::writeDataToFile(QByteArray data)
{
    m_dataLength = data.length();
    m_zip->writeData(data.data(), data.length());
}

void KisAnimationStore::writeDataToFile(const char *data, qint64 length)
{
    m_dataLength = length;
    m_zip->writeData(data, length);
}

void KisAnimationStore::openFile(QString filename)
{
    m_zip->open(QIODevice::ReadWrite);
    m_zip->prepareWriting(filename, "", "", 0);
}

void KisAnimationStore::closeFile()
{
    m_zip->finishWriting(m_dataLength);
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
