/*
   This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
                 2000 Werner Trobin <trobin@kde.org>

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

#ifndef koTemplateCreateDia_h
#define koTemplateCreateDia_h

#include <kdialogbase.h>
#include <koffice_export.h>
//Added by qt3to4:
#include <QPixmap>
#include <QByteArray>

class QString;
class QPixmap;
class QWidget;
class KInstance;
class KLineEdit;
class Q3ListViewItem;
class KoTemplateCreateDiaPrivate;

/****************************************************************************
 *
 * Class: koTemplateCreateDia
 *
 ****************************************************************************/

class KOFFICEUI_EXPORT KoTemplateCreateDia : public KDialogBase
{
    Q_OBJECT

public:
    KoTemplateCreateDia( const QByteArray &templateType, KInstance *instance,
                         const QString &file, const QPixmap &pix, QWidget *parent=0L );
    ~KoTemplateCreateDia();

    static void createTemplate( const QByteArray &templateType, KInstance *instance,
                                const QString &file, const QPixmap &pix, QWidget *parent=0L );

protected:
    void slotOk();

private slots:
    void slotDefault();
    void slotCustom();
    void slotSelect();
    void slotNameChanged(const QString &name);

    void slotAddGroup();
    void slotRemove();
    void slotSelectionChanged();
private:
    void updatePixmap();
    void fillGroupTree();

    QString m_file;
    QPixmap m_pixmap;
    KoTemplateCreateDiaPrivate *d;
};

#endif
