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

#ifndef KIS_TELEMETRY_REGULAR_PROVIDER_H
#define KIS_TELEMETRY_REGULAR_PROVIDER_H
#include "QScopedPointer"
#include <KUserFeedback/AbstractDataSource>
#include <KUserFeedback/CompilerInfoSource>
#include <KUserFeedback/LocaleInfoSource>
#include <KUserFeedback/OpenGLInfoSource>
#include <KUserFeedback/PlatformInfoSource>
#include <KUserFeedback/QtVersionSource>
#include <KUserFeedback/QtVersionSource>
#include <KUserFeedback/ScreenInfoSource>
#include <KUserFeedback/provider.h>
#include <QVariant>

#include "cpu_info_source.h"

#include "kritatelemetry_export.h"
#include <QMultiMap>
#include <QVector>
#include <QWeakPointer>
#include <kis_telemetry_abstract.h>
#include <memory>
#include "kis_telemetry_tickets.h"

class KRITATELEMETRY_EXPORT TelemetryProvider : public KisTelemetryAbstract {
public:
    TelemetryProvider();
    void sendData(QString path, QString adress = QString()) override;

    virtual ~TelemetryProvider();

protected:
    void getTimeTicket(QString id) override;
    void putTimeTicket(QString id) override;
    void saveImageProperites(QString fileName, KisImagePropertiesTicket::ImageInfo imageInfo) override;
    void saveActionInfo(QString id, KisActionInfoTicket::ActionInfo actionInfo) override;
    void saveAssertInfo(QString id, KisAssertInfoTicket::AssertInfo assertInfo) override;

private:
    enum TelemetryCategory {
        tools,
        install,
        asserts,
        fatalAsserts,
        imageProperties,
        actions
    };

private:
    QScopedPointer<KUserFeedback::Provider> m_toolsProvider;
    std::vector<std::unique_ptr<KUserFeedback::AbstractDataSource> > m_toolSources;
    QScopedPointer<KUserFeedback::Provider> m_installProvider;
    std::vector<std::unique_ptr<KUserFeedback::AbstractDataSource> > m_installSources;
    //TODO Fatal asserts and usual asserts
    QScopedPointer<KUserFeedback::Provider> m_assertsProvider;
    std::vector<std::unique_ptr<KUserFeedback::AbstractDataSource> > m_assertsSources;
    QScopedPointer<KUserFeedback::Provider> m_fatalAssertsProvider;
    std::vector<std::unique_ptr<KUserFeedback::AbstractDataSource> > m_fatalAssertsSources;
    QScopedPointer<KUserFeedback::Provider> m_imagePropertiesProvider;
    std::vector<std::unique_ptr<KUserFeedback::AbstractDataSource> > m_imagePropertiesSources;
    QScopedPointer<KUserFeedback::Provider> m_actionsInfoProvider;
    std::vector<std::unique_ptr<KUserFeedback::AbstractDataSource> > m_actionsSources;
    QMultiMap<QString, QWeakPointer<KisTelemetryTicket> > m_tickets;

private:
    TelemetryCategory pathToKind(QString path);
};

#endif
