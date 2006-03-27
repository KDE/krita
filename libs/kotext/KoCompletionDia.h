/* This file is part of the KDE project
   Copyright (C) 2005 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef kocompletiondia_h
#define kocompletiondia_h

#include "KoCompletionBase.h"
#include <kdialogbase.h>
#include <KoAutoFormat.h>

/**
 * Completion configuration widget.
 */
class KoCompletion : public KoCompletionBase {
    Q_OBJECT

public:
    KoCompletion(QWidget *parent, KoAutoFormat *autoFormat );
    void saveSettings();

protected slots:
    void changeButtonStatus();
    void slotResetConf();
    void slotAddCompletionEntry();
    void slotRemoveCompletionEntry();
    void slotCompletionWordSelected( const QString & );
    void slotSaveCompletionEntry();

protected:
    KoAutoFormat m_autoFormat; // The copy we're working on
    KoAutoFormat * m_docAutoFormat; // Pointer to the real one (in KWDocument)
    QStringList m_listCompletion; // The copy of the completion items - don't use m_autoFormat.getCompletion()!
};

/**
 * Completion configuration dialog.
 */
class KOTEXT_EXPORT KoCompletionDia : public KDialogBase {
    Q_OBJECT

public:
    KoCompletionDia( QWidget *parent, const char *name, KoAutoFormat * autoFormat );

protected slots:
    virtual void slotOk();

protected:
    void setup();

private:
    KoCompletion *m_widget;
};
#endif
