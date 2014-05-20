/*
 * Copyright 2014 Manuel Riecke <spell1337@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

#include "palettegeneratorconfig.h"
#include <QTextStream>

PaletteGeneratorConfig::PaletteGeneratorConfig()
{
    for(int j = 0; j < 4; ++j)
    {
        colors[0][j] = QColor(Qt::white);
        colors[1][j] = QColor(Qt::yellow);
        colors[2][j] = QColor(Qt::gray);
        colors[3][j] = QColor(Qt::black);
    }

    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            colorsEnabled[i][j] = (j == 0);

    for(int i = 0; i < 3; ++i)
        gradientSteps[i] = 4;

    inbetweenRampSteps = 2;
    diagonalGradients = true;
}

QByteArray PaletteGeneratorConfig::toByteArray()
{
    QByteArray retVal;
    QDataStream stream(&retVal, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_6);
    stream.setByteOrder(QDataStream::BigEndian);

    // Version int
    stream << 0;

    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            stream << colors[i][j];

    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            stream << colorsEnabled[i][j];

    for(int i = 0; i < 3; ++i)
        stream << gradientSteps[i];

    stream << inbetweenRampSteps;
    stream << diagonalGradients;
    return retVal;
}

void PaletteGeneratorConfig::fromByteArray(const QByteArray& str)
{
    QDataStream stream(str);
    stream.setVersion(QDataStream::Qt_4_6);
    stream.setByteOrder(QDataStream::BigEndian);

    int version;
    stream >> version;
    if(version == 0)
    {
        for(int i = 0; i < 4; ++i)
            for(int j = 0; j < 4; ++j)
                stream >> colors[i][j];

        for(int i = 0; i < 4; ++i)
            for(int j = 0; j < 4; ++j)
                stream >> colorsEnabled[i][j];

        for(int i = 0; i < 3; ++i)
            stream >> gradientSteps[i];

        stream >> inbetweenRampSteps;
        stream >> diagonalGradients;
    }
    else
        qDebug("PaletteGeneratorConfig::FromByteArray: Unsupported data version");
}
