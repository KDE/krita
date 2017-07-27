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

#ifndef KIS_TELEMETRY_ACTIONS_H
#define KIS_TELEMETRY_ACTIONS_H
#include "kritatelemetry_export.h"
#include <QSize>
#include <QString>

class KisTelemetryAbstract;
class KisTelemetryAction {
public:
    virtual ~KisTelemetryAction() = default;
};

class KRITATELEMETRY_EXPORT KisToolsActivate : public KisTelemetryAction {
};

class KRITATELEMETRY_EXPORT KisToolsDeactivate : public KisTelemetryAction {
};

class KRITATELEMETRY_EXPORT KisToolsStartUse : public KisTelemetryAction {
};

class KRITATELEMETRY_EXPORT KisToolsStopUse : public KisTelemetryAction {
};

class KRITATELEMETRY_EXPORT KisSaveImageProperties : public KisTelemetryAction {
public:
    struct ImageInfo {
        QSize size;
        QString filename;
        QString colorProfile;
        QString colorSpace;
        int numLayers;
    };
public:
    KisSaveImageProperties(ImageInfo imageInfo);
    QString fileName() const;
    ImageInfo imageInfo() const;

private:
    ImageInfo m_imageInfo;
};

class KRITATELEMETRY_EXPORT KisSaveActionInfo : public KisTelemetryAction {
public:
    struct ActionInfo{
        QString name;
        QString source;
    };
public:
    KisSaveActionInfo(ActionInfo actionInfo);
    ActionInfo actionInfo() const;

private:
    ActionInfo m_actionInfo;
};


#endif
