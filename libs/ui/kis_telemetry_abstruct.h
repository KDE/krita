/* This file is part of the KDE project
   Copyright (C) 2017 Alexey Kapustin <akapust1n@mail.ru>


   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KIS_TELEMETRY_ABSTRUCT_H
#define KIS_TELEMETRY_ABSTRUCT_H
#include "QScopedPointer"
#include "kritaflake_export.h"
#include <KUserFeedback/AbstractDataSource>
#include <KUserFeedback/cpuinfosource.h>
#include <KUserFeedback/provider.h>
#include <QString>
#include <QVector>

class KRITAFLAKE_EXPORT KisTelemetryAbstruct {
public:
    enum UseMode{
        Activate,
        Use
    };
    enum Action{
        getTimeTicket_,
        putTimeTicket_
    };

public:
    virtual void sendData(QString path) = 0;
    virtual void getTimeTicket(QString id, UseMode mode = Activate) = 0;
    virtual void putTimeTicket(QString id, UseMode mode = Activate) = 0;
    QString getToolId(QString id, UseMode mode = Activate );
    virtual ~KisTelemetryAbstruct() {}

protected:
    QString m_adress = "http://localhost:8080/";
    // QString m_adress = "http://akapustin.me:8080/";
private:
    QString getUseMode(UseMode mode);
};

#endif
