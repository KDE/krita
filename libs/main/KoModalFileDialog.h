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

#ifndef KOMODALFILEDIALOG_H
#define KOMODALFILEDIALOG_H

#include "komain_export.h"

#include <QFileDialog>
#include <QString>

/**
 * Wrapper around QFileDialog to provide static method for creating modal file
 * chooser sheet under os x, in QFileDialog static methods don't support modal file
 * chooser sheet on os x.
 * @note Used only when a modal dialog is really needed!
 */
class KOMAIN_EXPORT KoModalFileDialog : QObject
{
    Q_OBJECT
public:
    KoModalFileDialog() {}
    QString getOpenFileName(QWidget * parent = 0,
                                   const QString & caption = QString(),
                                   const QString & dir = QString(),
                                   const QString & filter = QString());

private slots:
    void getFileName(QString filename);

private:
    QString m_fileName;
};

#endif /* KOMODALFILEDIALOG_H */
