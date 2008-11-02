/* This file is part of the KDE project
 * Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOPABACKGROUNDTOOL_H
#define KOPABACKGROUNDTOOL_H

#include <KoTool.h>

class KoPAView;

class KoPABackgroundTool : public KoTool
{
    Q_OBJECT
public:
    KoPABackgroundTool( KoCanvasBase* base );
    virtual ~KoPABackgroundTool();

    ///Reimplemented from KoTool
    virtual void paint( QPainter &painter, const KoViewConverter &converter );
    ///Reimplemented from KoTool
    virtual void activate( bool temporary );
    ///Reimplemented from KoTool
    virtual void deactivate();
    ///Reimplemented from KoTool
    virtual void mousePressEvent( KoPointerEvent *event );
    ///Reimplemented from KoTool
    virtual void mouseMoveEvent( KoPointerEvent *event );
    ///Reimplemented from KoTool
    virtual void mouseReleaseEvent( KoPointerEvent *event );

    KoPAView * view() const;

public slots:
    void slotActivePageChanged();

protected:
    ///Reimplemented from KoTool
    virtual QMap<QString, QWidget *> createOptionWidgets();

private:
    KoPAView * m_view;
};

#endif //KOPABACKGROUNDTOOL_H
