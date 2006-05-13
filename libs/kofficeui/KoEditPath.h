/* This file is part of the KDE project
   Copyright (C)  2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#ifndef __KOEditPathDia__
#define __KOEditPathDia__

#include <kdialogbase.h>
#include <QStringList>
#include <koffice_export.h>
class KEditListBox;
class KUrlRequester;
class QCheckBox;

class KOFFICEUI_EXPORT KoEditPathDia : public KDialogBase
{
    Q_OBJECT
public:
    KoEditPathDia( const QString & _path, QWidget *parent, const char *name );
    QString newPath()const;

private:
    KEditListBox *m_listpath;
    KUrlRequester *urlReq;
};

class KOFFICEUI_EXPORT KoChangePathDia : public KDialogBase
{
    Q_OBJECT
public:
    KoChangePathDia( const QString & _path, QWidget *parent, const char *name );
    QString newPath()const;
private slots:
    void slotChangeDefaultValue( bool );
private:
    KUrlRequester *m_urlReq;
    QCheckBox *m_defaultPath;
};

#endif
