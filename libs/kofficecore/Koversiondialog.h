/* This file is part of the KDE project
   Copyright (C) 2005 Laurent Montel <montel@kde.org>

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

#ifndef __VERSION_DIALOG__
#define __VERSION_DIALOG__

#include <kdialogbase.h>

class QPushButton;
class QToolButton;
class K3ListView;
class Q3MultiLineEdit;

class KoVersionDialog : public KDialogBase
{
    Q_OBJECT
public:
    KoVersionDialog(  QWidget* parent, const char* name = 0L );
    ~KoVersionDialog();

public slots:
    virtual void slotOk();
    void slotRemove();
    void slotAdd();
    void slotOpen();
    void slotModify();

protected:

    void init();
    void updateButton();

    K3ListView * list;
    QPushButton* m_pRemove;
    QPushButton* m_pAdd;
    QPushButton* m_pOpen;
    QPushButton* m_pModify;
};

class KoVersionModifyDialog : public KDialogBase
{
    Q_OBJECT
public:
    KoVersionModifyDialog(  QWidget* parent, const QString &_comment = QString::null , const char* name = 0L );

    QString comment() const;

private:
    Q3MultiLineEdit *m_multiline;
};

#endif
