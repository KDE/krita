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

#ifndef KIS_ANIMATION_STORE_H
#define KIS_ANIMATION_STORE_H

#include <QByteArray>
#include <QDir>
#include <QFile>

class KisAnimationStore
{
public:
    KisAnimationStore(QString filename);
    ~KisAnimationStore();

public:
    void enterDirectory(QString directory);

    void leaveDirectory();

    void writeDataToFile(QByteArray data);

    void writeDataToFile(const char *data, qint64 length);

    void readFromFile(char* buffer, qint64 length);

    void openFileWriting(QString filename);

    void openFileReading(QString filename);

    void closeFile();

    void setMimetype();

    bool hasFile(QString location) const;

    QIODevice* getDevice(QString location);

private:
    QFile* m_currentFile;
    QDir* m_dir;
    int m_dataLength;
};

#endif // KIS_ANIMATION_STORE_H
