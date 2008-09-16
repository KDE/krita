/* This file is part of the KDE project
 * Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.net>
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

#ifndef KOPAMASTERPAGEDOCKER_H
#define KOPAMASTERPAGEDOCKER_H

#include <QWidget>

class KoPAView;

class KoPAMasterPageDocker : public QWidget
{
    Q_OBJECT
public:
    explicit KoPAMasterPageDocker( QWidget* parent = 0, Qt::WindowFlags flags = 0 ) { Q_UNUSED(parent); Q_UNUSED(flags); }
    void setView( KoPAView* view ) { Q_UNUSED(view); }
};

#endif //KOPAMASTERPAGEDOCKER_H
