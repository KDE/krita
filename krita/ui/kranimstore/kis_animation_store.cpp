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
#include <QDir>
#include <QFile>

KisAnimationStore::KisAnimationStore(QString filename)
{
    m_dir = new QDir(filename);
    m_dir->mkdir(filename);
}

KisAnimationStore::~KisAnimationStore()
{

}

void KisAnimationStore::enterDirectory(QString directory)
{
    if(!m_dir->cd(directory)){
        m_dir->mkdir(m_dir->absolutePath() + "/" + directory);
        m_dir->cd(directory);
    }
 }

void KisAnimationStore::leaveDirectory()
{
    m_dir->cdUp();
}

void KisAnimationStore::writeDataToFile(QByteArray data)
{
    m_currentFile->write(data.data(), data.length());
}

void KisAnimationStore::writeDataToFile(const char *data, qint64 length)
{
    m_currentFile->write(data, length);
}

void KisAnimationStore::openFileWriting(QString filename)
{
    m_currentFile = new QFile(m_dir->absoluteFilePath(filename));
    m_currentFile->open(QFile::WriteOnly);
}

void KisAnimationStore::openStore(QIODevice::OpenMode mode)
{

}

void KisAnimationStore::closeFileWriting()
{
    m_currentFile->close();
}

void KisAnimationStore::closeStore()
{

}

void KisAnimationStore::setCompressionEnabled(bool e)
{

}

QIODevice* KisAnimationStore::getDevice(QString location)
{
    return m_currentFile;
}

bool KisAnimationStore::hasFile(QString location) const
{
    return true;
}

void KisAnimationStore::readFromFile(QString filename, char *buffer, qint64 length)
{
    /*QIODevice* file = static_cast<const KZipFileEntry*>(m_zip->directory()->entry(filename))->createDevice();
    file->read(buffer, length);
    file->close();
    delete file;*/
}

void KisAnimationStore::setMimetype()
{
    QByteArray mimetype("x-krita-animation");
    this->openFileWriting("mimetype");
    this->writeDataToFile(mimetype);
    this->closeFileWriting();
}
