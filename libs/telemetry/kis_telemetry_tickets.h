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
#ifndef KIS_TELEMETRY_TICKETS_H
#define KIS_TELEMETRY_TICKETS_H

#include <QDateTime>
#include <QSize>
#include <QFileInfo>
#include "kritatelemetry_export.h"

class KRITATELEMETRY_EXPORT KisTelemetryTicket {
public:
    KisTelemetryTicket() {}
    KisTelemetryTicket(QString id);
    QString ticketId() const;
    void setTickedId(QString id);
    virtual ~KisTelemetryTicket() {}
protected:
    QString m_id;
};

class KRITATELEMETRY_EXPORT KisTimeTicket : public KisTelemetryTicket {
public:
    KisTimeTicket(QString id);
    void setStartTime(QDateTime& time);
    void setEndTime(QDateTime& time);
    QDateTime startTime() const;
    QDateTime endTime() const;
    int useTimeMSeconds() const;
    void addMSecs(int seconds);

private:
    QDateTime m_start;
    QDateTime m_end;
};

class KRITATELEMETRY_EXPORT KisImagePropertiesTicket : public KisTelemetryTicket {
public:
    struct ImageInfo {
        QSize size;
        QString filename;
        QString colorProfile;
        QString colorSpace;
        int numLayers;
    };
public:
    KisImagePropertiesTicket(KisImagePropertiesTicket::ImageInfo imageInfo, QString id);

    QSize size() const;
    int getNumLayers() const;
    QString getFileFormat() const;
    QString getColorSpace() const;
    qint64 getImageSize() const;
    QString getColorProfile() const;

private:
    KisImagePropertiesTicket::ImageInfo m_imageInfo;
    QFileInfo m_fileInfo;
};

class KRITATELEMETRY_EXPORT KisActionInfoTicket : public KisTelemetryTicket {
public:
    struct ActionInfo{
        QString name;
        QString source;
    };
public:
    KisActionInfoTicket(KisActionInfoTicket::ActionInfo actionInfo, QString id);

    KisActionInfoTicket::ActionInfo actionInfo() const;

private:
    KisActionInfoTicket::ActionInfo m_actionInfo;
};

#endif
