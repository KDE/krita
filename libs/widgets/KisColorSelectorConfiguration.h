/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_COLOR_SELECTOR_CONFIGURATION_H
#define KIS_COLOR_SELECTOR_CONFIGURATION_H

#include <QString>
#include <QStringList>

#include "kritawidgets_export.h"

class KRITAWIDGETS_EXPORT KisColorSelectorConfiguration {

public:

    enum Type {Ring, Square, Wheel, Triangle, Slider};
    enum Parameters {H, hsvS, V, hslS, L, SL, SV, SV2, hsvSH, hslSH, VH, LH, SI, SY, hsiSH, hsySH, I, Y, IH, YH, hsiS, hsyS, Hluma};

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
        if(imt>Slider || ist>Slider || imtp>Hluma || istp>Hluma)
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
