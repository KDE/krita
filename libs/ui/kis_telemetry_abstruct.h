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
#include "kis_telemetry_actions.h"
#include <kis_types.h>


class KRITAFLAKE_EXPORT KisTelemetryAbstruct {
public:
    virtual void sendData(QString path, QString adress = QString()) = 0;
    virtual ~KisTelemetryAbstruct() {}
    void doTicket(KisToolsActivate &action, QString id);
    void doTicket(KisToolsDeactivate &action, QString id);
    void doTicket(KisToolsStartUse &action, QString id);
    void doTicket(KisToolsStopUse &action, QString id);
    void doTicket(KisSaveImageProperties &action, QString id);


protected:
    virtual void getTimeTicket(QString id) = 0;
    virtual void putTimeTicket(QString id) = 0;
    virtual void saveImageProperites(QString fileName, KisImageSP &image) = 0;

protected:
    QString m_adress = "http://localhost:8080/";
    // QString m_adress = "http://akapustin.me:8080/";
private:
    enum UseMode{
        Activate,
        Use
    };
private:
    QString getToolId(QString id, UseMode mode = Activate);
    QString getUseMode(UseMode mode);
};

#endif
