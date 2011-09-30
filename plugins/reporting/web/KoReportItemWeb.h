/* This file is part of the KDE project
   Copyright Shreya Pandit <shreya@shreyapandit.com>

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

#ifndef KOREPORTITEMWEB_H
#define KOREPORTITEMWEB_H

#include <KoReportItemBase.h>
#include "krpos.h"
#include "krsize.h"
#include "KoReportData.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>

#include <KoGlobal.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kglobalsettings.h>

#include <QRect>
#include <QGraphicsScene>
#include <QtGui/QWidget>
#include <QtCore/QUrl>

class QAction;
class QWebHistory;
class QUrl;
class QGraphicsItem;
class QWebPage;
class QImage;

namespace Scripting
{
class Web;
}

/**
 @author Shreya Pandit
*/
class KoReportItemWeb : public KoReportItemBase
{
    Q_OBJECT
public:
    KoReportItemWeb();
    KoReportItemWeb(QDomNode &element);
    virtual ~KoReportItemWeb();
    virtual QString typeName() const;

    virtual int render(OROPage *page, OROSection *section,  QPointF offset,
                       QVariant data, KRScriptHandler *script);
    using KoReportItemBase::render;

public slots:
    void setUrl(const QString &url);
    //   void rendering();
private slots:
    //void update();
    void loadFinished(bool);
private:
    void init();
    bool m_loaded;

protected:
    KoProperty::Property *url;
    KoProperty::Property *m_controlSource;
    QUrl m_url;
    QWebPage *m_webPage;
    virtual void createProperties();
    QImage *m_webImage;

    friend class Scripting::Web;
};

#endif
