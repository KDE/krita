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
#ifndef KIS_TICKETS_H
#define KIS_TICKETS_H

#include <QDateTime>
#include <QSize>
#include <kis_types.h>
#include "kis_telemetry_actions.h"
#include <QFileInfo>

class KisTicket {
public:
    KisTicket() {}
    KisTicket(QString id);
    QString ticketId() const;
    void setTickedId(QString id);
    virtual ~KisTicket() {}
protected:
    QString m_id;
};

class KisTimeTicket : public KisTicket {
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

class KisImagePropertiesTicket : public KisTicket {
public:
    KisImagePropertiesTicket(KisSaveImageProperties::ImageInfo imageInfo, QString id);

    QSize size() const;
    int getNumLayers() const;
    QString getFileFormat() const;
    QString getColorSpace() const;
    qint64 getImageSize() const;
    QString getColorProfile() const;

private:
    KisSaveImageProperties::ImageInfo m_imageInfo;
    QFileInfo m_fileInfo;
};

class KisActionInfoTicket : public KisTicket {
public:
    KisActionInfoTicket(KisSaveActionInfo::ActionInfo actionInfo, QString id);

    KisSaveActionInfo::ActionInfo actionInfo() const;

private:
    KisSaveActionInfo::ActionInfo m_actionInfo;
};

#endif
