/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_COLOR_SELECTOR_CONFIGURATION_H
#define KIS_COLOR_SELECTOR_CONFIGURATION_H

#include <QString>
#include <QStringList>

#include "kritawidgets_export.h"

class KRITAWIDGETS_EXPORT KisColorSelectorConfiguration {

public:

    enum Type {Ring, Square, Wheel, Triangle, Slider};
    enum Parameters {H, Hluma, hsvS, V, hslS, L, SL, SV, SV2, hsvSH, hslSH, VH, LH, SI, SY, hsiSH, hsySH, I, Y, IH, YH, hsiS, hsyS};

    Type mainType;
    Type subType;
    Parameters mainTypeParameter;
    Parameters subTypeParameter;

    KisColorSelectorConfiguration(Type mainT = Triangle, Type subT = Ring, Parameters mainTP = SL, Parameters subTP = H)
        : mainType(mainT)
        , subType(subT)
        , mainTypeParameter(mainTP)
        , subTypeParameter(subTP)
    {
    }

    KisColorSelectorConfiguration(QString string)
    {
        readString(string);
    }

    QString toString() const
    {
        return QString("%1|%2|%3|%4").arg(mainType).arg(subType).arg(mainTypeParameter).arg(subTypeParameter);
    }
    void readString(QString string)
    {
        QStringList strili = string.split('|');
        if(strili.length()!=4) return;

        int imt=strili.at(0).toInt();
        int ist=strili.at(1).toInt();
        int imtp=strili.at(2).toInt();
        int istp=strili.at(3).toInt();

        // Makes sure that Type and Parameters are within bounds.
        if(imt>Slider || ist>Slider || imtp>hsyS || istp>hsyS)
            return;

        mainType = Type(imt);
        subType = Type(ist);
        mainTypeParameter = Parameters(imtp);
        subTypeParameter = Parameters(istp);
    }

    static KisColorSelectorConfiguration fromString(QString string)
    {
        KisColorSelectorConfiguration ret;
        ret.readString(string);
        return ret;
    }
};

#endif
