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
#ifndef KIS_TELEMETRY_INSTANCE_H
#define KIS_TELEMETRY_INSTANCE_H

#include "kis_telemetry_abstract.h"
#include "kritatelemetry_export.h"
#include <QScopedPointer>
#include <QObject>
#include <QElapsedTimer>

class KRITATELEMETRY_EXPORT KisTelemetryInstance: public QObject {
    Q_OBJECT
public:
    enum Actions {
        ToolActivate,
        ToolDeactivate,
        ToolsStartUse,
        ToolsStopUse
    };
    enum UseMode {
        Activate,
        Use
    };

public:
    KisTelemetryInstance();
    ~KisTelemetryInstance() = default;
    static KisTelemetryInstance* instance();

    void setProvider(KisTelemetryAbstract* provider);
    void notifyToolAcion(Actions action, QString id);
    void notifySaveImageProperties(KisImagePropertiesTicket::ImageInfo imageInfo, QString id);
    void notifySaveActionInfo(KisActionInfoTicket::ActionInfo imageInfo, QString id);
    void sendData(QString path, QString adress = QString());
    QString getToolId(QString id, UseMode mode = Activate);
public Q_SLOTS:
    void agregateData();
private:
    KisTelemetryInstance(KisTelemetryInstance const&) = delete;
    KisTelemetryInstance& operator=(KisTelemetryInstance const&) = delete;
    QScopedPointer<KisTelemetryAbstract> telemetryProvider;
    QString getUseMode(UseMode mode);
private:
     QElapsedTimer m_timer;
     qint64 m_checkTime;
};
#endif
