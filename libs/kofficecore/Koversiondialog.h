/* This file is part of the KDE project
   Copyright (C) 2005 Laurent Montel <montel@kde.org>
   Copyright (C) 2006 Fredrik Edemar <f_edemar@linux.se>

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

#include <kdialog.h>

#include "KoDocument.h"

class QPushButton;
class QToolButton;
class QTreeWidget;
class Q3MultiLineEdit;

class KoVersionDialog : public KDialog
{
    Q_OBJECT
public:
    KoVersionDialog(  QWidget* parent, KoDocument *doc );
    ~KoVersionDialog();

public slots:
    void slotRemove();
    void slotAdd();
    void slotOpen();
    void slotModify();

protected:

    void init();
    void updateButton();

    QTreeWidget * list;
    QPushButton* m_pRemove;
    QPushButton* m_pAdd;
    QPushButton* m_pOpen;
    QPushButton* m_pModify;
    KoDocument *m_doc;
};

class KoVersionModifyDialog : public KDialog
{
    Q_OBJECT
public:
    KoVersionModifyDialog(  QWidget* parent, KoVersionInfo *info );

    QString comment() const;

private:
    Q3MultiLineEdit *m_multiline;
};

#endif
