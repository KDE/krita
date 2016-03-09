/*
 *  Copyright (c) 2016 Laszlo Fazekas <mneko@freemail.hu>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CSV_READ_LINE_H_
#define CSV_READ_LINE_H_

#include <QString>
#include <QIODevice>
#include <QByteArray>

#include "csv_layer_record.h"

/* The .png file names are within 20 characters, only the layer
 * names can be longer. If the length exceeds this, it will be
 * truncated.
 */
#define CSV_FIELD_MAX 50

class CSVReadLine
{

public:
    CSVReadLine();
    ~CSVReadLine();

    int nextLine(QIODevice* io);
    QString nextField();
    void rewind();

private:

    QString splitNext(int* pos);
    bool setLayer(CSVLayerRecord* );
    bool readFrame(int* pos);
    bool finalize();
    int readImpl(QIODevice* io);

    char       m_separator;
    int        m_row;
    QByteArray m_linebuf;
    int        m_pos;
};

#endif
