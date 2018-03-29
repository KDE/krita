/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISWELCOMEPAGE_H
#define KISWELCOMEPAGE_H

#include <QtQuickWidgets/QQuickWidget>

/**
 * @brief The KisWelcomPage class is shown before any image is
 * actually open and offers the user a choice of getting started
 * options in an unobtrusive way.
 *
 * See https://phabricator.kde.org/T7782
 */
class KisWelcomePage : public QQuickWidget
{
public:
    KisWelcomePage(QWidget *parent);
};

#endif // KISWELCOMPAGE_H
