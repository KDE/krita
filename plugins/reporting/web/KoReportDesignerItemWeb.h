/* This file is part of the KDE project
   Copyright Shreya Pandit <shreya@shreyapandit.com>
   Copyright 2011 Adam Pigg <adam@piggz.co.uk>

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

#ifndef KOREPORTDESIGNERITEMWEB_H
#define KOREPORTDESIGNERITEMWEB_H

#include <KoViewConverter.h>
#include <KoReportDesignerItemRectBase.h>
#include "KoReportItemWeb.h"

class QGraphicsScene;

/**
 @author Shreya Pandit <shreya@shreyapandit.com>
*/
class KoReportDesignerItemWeb : public KoReportItemWeb, public KoReportDesignerItemRectBase
{
    Q_OBJECT
public:
    KoReportDesignerItemWeb(KoReportDesigner *rw, QGraphicsScene *scene, const QPointF &pos);
    KoReportDesignerItemWeb(QDomNode &element, KoReportDesigner *rw, QGraphicsScene *scene);
    virtual ~KoReportDesignerItemWeb();
    
    void init(QGraphicsScene *scene);
    
    virtual void buildXML(QDomDocument &doc, QDomElement &parent);
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual KoReportDesignerItemWeb *clone();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    void init(QGraphicsScene *, KoReportDesigner *r);

private slots:
    void slotPropertyChanged(KoProperty::Set &, KoProperty::Property &);
};

#endif
