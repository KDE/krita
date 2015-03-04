/* This file is part of the KDE project
 * Copyright (C) 2011 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KOSTARTUPDIALOG_H
#define KOSTARTUPDIALOG_H

#include <QDialog>

/**
 * @brief The KoStartupDialog class shows the file selectioed, custom document
 * widgets and template lists. A bit like it was in KOffice 1.4...
 */
class KoStartupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KoStartupDialog(QWidget *parent = 0);
    
Q_SIGNALS:
    
public Q_SLOTS:
    
};

#endif // KOSTARTUPDIALOG_H
