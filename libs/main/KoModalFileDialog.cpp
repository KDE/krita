/* This file is part of the KDE project
   Copyright (C) 2013 Yue Liu <yue.liu@mail.com>

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

#include "KoModalFileDialog.h"

QString KoModalFileDialog::getOpenFileName(QWidget * parent, const QString & caption,
                        const QString & dir, const QString & filter)
{
    QFileDialog* dialog = new QFileDialog(parent, caption, dir, filter);
    dialog->open(parent, SLOT(getFileName(QString)));

    return m_fileName;
}

void KoModalFileDialog::getFileName(QString fileName)
{
    m_fileName = fileName;
}
