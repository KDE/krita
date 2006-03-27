/* This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <trobin@kde.org>
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include <qlabel.h>
#include <qlayout.h>
#include <q3listbox.h>
//Added by qt3to4:
#include <QMouseEvent>

#include <kaction.h>
#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kshortcutlist.h>
#include <kstdaccel.h>
#include <kstdaction.h>

#include "KoCommandHistory.h"

KoListBox::KoListBox( QWidget *parent , const char *name , Qt::WFlags f)
    : Q3ListBox( parent, name, f)
{
    setVScrollBarMode( AlwaysOn );
}

void KoListBox::contentsMouseMoveEvent ( QMouseEvent * e)
{
    Q3ListBoxItem *item_p = itemAt( contentsToViewport(e->pos()));
    if ( item_p )
    {
        int itemIndex = index( item_p );
        for ( int i = 0; i<=itemIndex;i++)
            setSelected ( i, true );
        for ( int i = itemIndex+1; i<(int)count();i++)
            setSelected ( i, false );
        emit changeNumberOfSelectedItem( itemIndex);
    }
}

QSize KoListBox::sizeHint() const
{
  return QSize(qMin(maxItemWidth() + verticalScrollBar()->width() + 4, 400),
               qMin(count() * itemHeight() + horizontalScrollBar()->height() + 4,300));
}

class KoCommandHistory::KoCommandHistoryPrivate {
public:
    KoCommandHistoryPrivate() {
        m_savedAt=-1;
        m_present=0;
    }
    ~KoCommandHistoryPrivate() {}
    int m_savedAt;
    KCommand *m_present;
    KoListBox *m_undoListBox;
    KoListBox *m_redoListBox;
    QLabel *m_undoLabel;
    QLabel *m_redoLabel;
};

////////////

KoCommandHistory::KoCommandHistory() :
    m_undo(0), m_redo(0), m_undoLimit(50), m_redoLimit(30), m_first(false)
{
    d=new KoCommandHistoryPrivate();
    m_commands.setAutoDelete(true);
    clear();
}

KoCommandHistory::KoCommandHistory(KActionCollection * actionCollection, bool withMenus) :
    m_undoLimit(50), m_redoLimit(30), m_first(false)
{
    d=new KoCommandHistoryPrivate();
    if (withMenus)
    {
        KToolBarPopupAction * undo = new KToolBarPopupAction( i18n("&Undo"), "undo",
                                                              KStdAccel::undo(), this, SLOT( undo() ),
                                                              actionCollection, /*KStdAction::stdName( KStdAction::Undo )*/"koffice_undo" );
        connect( undo->popupMenu(), SIGNAL( aboutToShow() ), this, SLOT( slotUndoAboutToShow() ) );
        connect( undo->popupMenu(), SIGNAL( activated( int ) ), this, SLOT( slotUndoActivated( int ) ) );
        m_undo = undo;
        m_undoPopup = undo->popupMenu();
        d->m_undoListBox = new KoListBox( m_undoPopup );
        d->m_undoListBox->setSelectionMode( Q3ListBox::Multi );

        m_undoPopup->insertItem(d->m_undoListBox);
        d->m_undoLabel = new QLabel( m_undoPopup);
        m_undoPopup->insertItem(d->m_undoLabel);

        connect( d->m_undoListBox, SIGNAL( selected( int ) ), this, SLOT( slotUndoActivated( int ) ) );
        connect( d->m_undoListBox, SIGNAL(clicked ( Q3ListBoxItem *)), this, SLOT( slotUndoActivated( Q3ListBoxItem * ) ) );

        connect( d->m_undoListBox, SIGNAL( changeNumberOfSelectedItem( int )), this, SLOT( slotChangeUndoNumberOfSelectedItem( int )));

        KToolBarPopupAction * redo = new KToolBarPopupAction( i18n("&Redo"), "redo",
                                                              KStdAccel::redo(), this, SLOT( redo() ),
                                                              actionCollection, /*KStdAction::stdName( KStdAction::Redo )*/
                                                              "koffice_redo");
        connect( redo->popupMenu(), SIGNAL( aboutToShow() ), this, SLOT( slotRedoAboutToShow() ) );
        connect( redo->popupMenu(), SIGNAL( activated( int ) ), this, SLOT( slotRedoActivated( int ) ) );
        m_redo = redo;
        m_redoPopup = redo->popupMenu();
        d->m_redoListBox = new KoListBox( m_redoPopup );
        d->m_redoListBox->setSelectionMode( Q3ListBox::Multi );
        m_redoPopup->insertItem(d->m_redoListBox);

        d->m_redoLabel = new QLabel( m_redoPopup);
        m_redoPopup->insertItem(d->m_redoLabel);


        connect( d->m_redoListBox, SIGNAL( selected( int ) ), this, SLOT( slotRedoActivated( int ) ) );
        connect( d->m_redoListBox, SIGNAL(clicked ( Q3ListBoxItem *)), this, SLOT( slotRedoActivated( Q3ListBoxItem * ) ) );
        connect( d->m_redoListBox, SIGNAL( changeNumberOfSelectedItem( int )), this, SLOT( slotChangeRedoNumberOfSelectedItem( int )));

    }
    else
    {
        m_undo = KStdAction::undo( this, SLOT( undo() ), actionCollection, "koffice_undo");
        m_redo = KStdAction::redo( this, SLOT( redo() ), actionCollection, "koffice_redo");
        m_undoPopup = 0L;
        m_redoPopup = 0L;
        d->m_redoListBox = 0L;
        d->m_undoListBox = 0L;
        d->m_redoLabel = 0L;
        d->m_undoLabel = 0L;
    }
    m_commands.setAutoDelete(true);
    clear();
}

KoCommandHistory::~KoCommandHistory() {
    delete d;
}

KCommand * KoCommandHistory::presentCommand ( )
{   return d->m_present;
}


void KoCommandHistory::clear() {
    if (m_undo != 0) {
        m_undo->setEnabled(false);
        m_undo->setText(i18n("&Undo"));
    }
    if (m_redo != 0) {
        m_redo->setEnabled(false);
        m_redo->setText(i18n("&Redo"));
    }
    d->m_present = 0L;
    d->m_savedAt=-42;
}

void KoCommandHistory::addCommand(KCommand *command, bool execute) {

    if(command==0L)
        return;

    int index;
    if(d->m_present!=0L && (index=m_commands.findRef(d->m_present))!=-1) {
        if (m_first)
            --index;
        m_commands.insert(index+1, command);
        // truncate history
        unsigned int count=m_commands.count();
        for(unsigned int i=index+2; i<count; ++i)
            m_commands.removeLast();
        // check whether we still can reach savedAt
        if(index<d->m_savedAt)
            d->m_savedAt=-1;
        d->m_present=command;
        m_first=false;
        if (m_undo != 0) {
            m_undo->setEnabled(true);
            m_undo->setText(i18n("&Undo: %1").arg(d->m_present->name()));
        }
        if((m_redo != 0) && m_redo->isEnabled()) {
            m_redo->setEnabled(false);
            m_redo->setText(i18n("&Redo"));
        }
        clipCommands();
    }
    else { // either this is the first time we add a Command or something has gone wrong
        kDebug(230) << "KoCommandHistory: Initializing the Command History" << endl;
        m_commands.clear();
        m_commands.append(command);
        d->m_present=command;
        if (m_undo != 0) {
            m_undo->setEnabled(true);
            m_undo->setText(i18n("&Undo: %1").arg(d->m_present->name()));
        }
        if (m_redo != 0) {
            m_redo->setEnabled(false);
            m_redo->setText(i18n("&Redo"));
        }
        m_first=false;    // Michael B: yes, that *is* correct :-)
    }
    if ( execute )
    {
        command->execute();
        emit commandExecuted();
        emit commandExecuted(command);
    }
}

void KoCommandHistory::undo() {

    if (m_first || (d->m_present == 0L))
        return;

    d->m_present->unexecute();
    KCommand *commandUndone = d->m_present;

    if (m_redo != 0) {
        m_redo->setEnabled(true);
        m_redo->setText(i18n("&Redo: %1").arg(d->m_present->name()));
    }
    int index;
    if((index=m_commands.findRef(d->m_present))!=-1 && m_commands.prev()!=0) {
        d->m_present=m_commands.current();
        emit commandExecuted();
        emit commandExecuted(commandUndone);
        if (m_undo != 0) {
            m_undo->setEnabled(true);
            m_undo->setText(i18n("&Undo: %1").arg(d->m_present->name()));
        }
        --index;
        if(index==d->m_savedAt)
            emit documentRestored();
    }
    else {
        emit commandExecuted();
        emit commandExecuted(commandUndone);
        if (m_undo != 0) {
            m_undo->setEnabled(false);
            m_undo->setText(i18n("&Undo"));
        }
        if(d->m_savedAt==-42)
            emit documentRestored();
        m_first=true;
    }
    clipCommands(); // only needed here and in addCommand, NOT in redo
}

void KoCommandHistory::redo() {

    int index;
    if(m_first) {
        d->m_present->execute();
        emit commandExecuted();
        emit commandExecuted(d->m_present);
        m_first=false;
        m_commands.first();
        if(d->m_savedAt==0)
            emit documentRestored();
    }
    else if((index=m_commands.findRef(d->m_present))!=-1 && m_commands.next()!=0) {
        d->m_present=m_commands.current();
        d->m_present->execute();
        emit commandExecuted();
        emit commandExecuted(d->m_present);
        ++index;
        if(index==d->m_savedAt)
            emit documentRestored();
    }

    if (m_undo != 0) {
        m_undo->setEnabled(true);
        m_undo->setText(i18n("&Undo: %1").arg(d->m_present->name()));
    }

    if(m_commands.next()!=0) {
        if (m_redo != 0) {
            m_redo->setEnabled(true);
            m_redo->setText(i18n("&Redo: %1").arg(m_commands.current()->name()));
        }
    }
    else {
        if((m_redo != 0) && m_redo->isEnabled()) {
            m_redo->setEnabled(false);
            m_redo->setText(i18n("&Redo"));
        }
    }
}

void KoCommandHistory::documentSaved() {
    if(d->m_present!=0 && !m_first)
        d->m_savedAt=m_commands.findRef(d->m_present);
    else if(d->m_present==0 && !m_first)
        d->m_savedAt=-42;  // this value signals that the document has
                        // been saved with an empty history.
    else if(m_first)
        d->m_savedAt=-42;
}

void KoCommandHistory::setUndoLimit(int limit) {

    if(limit>0 && limit!=m_undoLimit) {
        m_undoLimit=limit;
        clipCommands();
    }
}

void KoCommandHistory::setRedoLimit(int limit) {

    if(limit>0 && limit!=m_redoLimit) {
        m_redoLimit=limit;
        clipCommands();
    }
}

void KoCommandHistory::clipCommands() {

    int count=m_commands.count();
    if(count<=m_undoLimit && count<=m_redoLimit)
        return;

    int index=m_commands.findRef(d->m_present);
    if(index>=m_undoLimit) {
        for(int i=0; i<=(index-m_undoLimit); ++i) {
            m_commands.removeFirst();
            --d->m_savedAt;
            if(d->m_savedAt==-1)
                d->m_savedAt=-42;
        }
        index=m_commands.findRef(d->m_present); // calculate the new
        count=m_commands.count();            // values (for the redo-branch :)
        // make it easier for us... d->m_savedAt==-1 -> invalid
        if(d->m_savedAt!=-42 && d->m_savedAt<-1)
            d->m_savedAt=-1;
    }
    // adjust the index if it's the first command
    if(m_first)
        index=-1;
    if((index+m_redoLimit+1)<count) {
        if(d->m_savedAt>(index+m_redoLimit))
            d->m_savedAt=-1;
        for(int i=0; i<(count-(index+m_redoLimit+1)); ++i)
            m_commands.removeLast();
    }
}

void KoCommandHistory::slotUndoAboutToShow()
{
    d->m_undoListBox->clear();
    slotChangeUndoNumberOfSelectedItem( -1 );
    int i = 0;
    QStringList lst;
    if (m_commands.findRef(d->m_present)!=-1)
        while ( m_commands.current() && i<10 ) // TODO make number of items configurable ?
        {
            lst.append(i18n("Undo: %1").arg(m_commands.current()->name()));
            m_commands.prev();
        }
    d->m_undoListBox->insertStringList( lst );
}

void KoCommandHistory::slotUndoActivated( int pos )
{
    kDebug(230) << "KoCommandHistory::slotUndoActivated " << pos << endl;
    for ( int i = 0 ; i < pos+1; ++i )
        undo();
    m_undoPopup->hide();
}

void KoCommandHistory::slotUndoActivated( Q3ListBoxItem * item)
{
    if ( item )
        slotUndoActivated( item->listBox()->index(item));
}

void KoCommandHistory::slotRedoActivated( Q3ListBoxItem * item)
{
    if ( item )
        slotRedoActivated( item->listBox()->index(item));
}

void KoCommandHistory::slotChangeUndoNumberOfSelectedItem( int pos)
{
    d->m_undoLabel->setText( i18n("Undo %n action", "Undo %n actions", pos+1) );
}

void KoCommandHistory::slotChangeRedoNumberOfSelectedItem( int pos)
{
    d->m_redoLabel->setText( i18n("Redo %n action", "Redo %n actions", pos+1) );
}


void KoCommandHistory::slotRedoAboutToShow()
{
    d->m_redoListBox->clear();
    slotChangeRedoNumberOfSelectedItem( -1 );
    QStringList lst;
    int i = 0;
    if (m_first)
    {
        d->m_present = m_commands.first();
        lst.append(i18n("Redo: %1").arg(d->m_present->name()));
    }
    if (m_commands.findRef(d->m_present)!=-1 && m_commands.next())
        while ( m_commands.current() && i<10 ) // TODO make number of items configurable ?
        {
            lst.append(i18n("Redo: %1").arg(m_commands.current()->name()));
            m_commands.next();
        }
    d->m_redoListBox->insertStringList( lst );

}

void KoCommandHistory::slotRedoActivated( int pos )
{
    kDebug(230) << "KoCommandHistory::slotRedoActivated " << pos << endl;
    for ( int i = 0 ; i < pos+1; ++i )
        redo();
    m_redoPopup->hide();
}

void KoCommandHistory::updateActions()
{
    if ( m_undo && m_redo )
    {
        m_undo->setEnabled( !m_first && ( d->m_present != 0L ) );
        m_redo->setEnabled(m_first || (m_commands.findRef(d->m_present)!=-1 && m_commands.next()!=0));
    }
}

void KoCommandHistory::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "KoCommandHistory.moc"
