/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2011  Adam Pigg <adam@piggz.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef KOREPORTASYNCITEMMANAGER_H
#define KOREPORTASYNCITEMMANAGER_H

#include <QObject>
#include "KoReportASyncItemBase.h"
#include <qqueue.h>

class RenderData {
public:
    KoReportASyncItemBase* item;
    OROPage* page;
    OROSection* section;
    QPointF offset;
    QVariant data;
    KRScriptHandler* script;
};

class KoReportASyncItemManager : public QObject
{
    Q_OBJECT
    
public:
    KoReportASyncItemManager(QObject *parent);
    virtual ~KoReportASyncItemManager();
    
    void addItem(KoReportASyncItemBase *item, OROPage *page, OROSection *section, QPointF offset, QVariant data, KRScriptHandler *script);
    
    void startRendering();
    
private:
    QQueue<RenderData*> m_renderList;
    QList<KoReportASyncItemBase*> m_itemList;
    
private slots:
    void itemFinished();
    
signals:
    void finished();
    
};

#endif // KOREPORTASYNCITEMMANAGER_H
