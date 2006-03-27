/* This file is part of the KDE project
   Copyright (C) 1999 by Dirk A. Mueller <dmuell@gmx.net>
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>

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

#include <qlayout.h>
#include <qvgroupbox.h>
#include <q3popupmenu.h>
#include <qmenubar.h>
//Added by qt3to4:
#include <Q3HBoxLayout>

#include <kdebug.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <KoTooluButton.h>
#include <coloraction_test.h>

#include <stdlib.h>
#include <time.h>


TopLevel::TopLevel( QWidget* parent, const char* name) : Q3MainWindow( parent, name )
{
    setCaption( QString::fromLatin1( "KColorAction test application" ) );

    QWidget *w = new QWidget( this );
    setCentralWidget( w );

    Q3BoxLayout* l = new Q3HBoxLayout( w, KDialog::marginHint(), KDialog::spacingHint() );
    Q3GroupBox* b1 = new QVGroupBox( QString::fromLatin1( "KoColorPanel 1" ), w );
    panel = new KoColorPanel( b1, "panel1" );
    connect( panel, SIGNAL( colorSelected( const QColor& ) ), SLOT( slotColorSelected( const QColor& ) ) );
    //panel->insertDefaultColors();
    l->addWidget( b1 );

    b1 = new QVGroupBox( QString::fromLatin1( "KoColorPanel 2" ), w );

    ( void ) new KoColorPanel( b1, "panel2" );
    l->addWidget( b1 );

    Q3PopupMenu* file = new Q3PopupMenu( this );
    menuBar()->insertItem( "&File", file );

    file->insertItem( "Custom + Default", KoColorPanel::createColorPopup( KoColorPanel::CustomColors, Qt::red, this,
                                                                          SLOT( slotColorSelected( const QColor& ) ), file, "blah" ) );
    file->insertItem( "Custom", KoColorPanel::createColorPopup( KoColorPanel::CustomColors, QColor(), this,
                                                                SLOT( slotColorSelected( const QColor& ) ), file, "blah" ) );
    file->insertItem( "Plain + Default", KoColorPanel::createColorPopup( KoColorPanel::Plain, Qt::green, this,
                                                                         SLOT( slotColorSelected( const QColor& ) ), file, "blah" ) );
    file->insertItem( "Plain", KoColorPanel::createColorPopup( KoColorPanel::Plain, QColor(), this,
                                                               SLOT( slotColorSelected( const QColor& ) ), file, "blah" ) );
    file->insertSeparator();
    file->insertItem( "Default Colors", this, SLOT( defaultColors() ), Qt::CTRL+Qt::Key_D );
    file->insertItem( "Insert Random Color", this, SLOT( insertRandomColor() ), Qt::CTRL+Qt::Key_R );
    file->insertSeparator();
    file->insertItem( "Clear", this, SLOT( clearColors() ), Qt::CTRL+Qt::Key_C );
    file->insertSeparator();
    file->insertItem( "&Quit", qApp, SLOT( closeAllWindows() ), Qt::CTRL+Qt::Key_Q );

    KToolBar* toolBar = new KToolBar( this );
    addDockWindow( toolBar );
    ( void ) new KoToolButton( "color_fill", 1, toolBar, "funky button", "Fill Color" );
}

void TopLevel::insertRandomColor()
{
    panel->insertColor( qRgb( rand() % 256, rand() % 256, rand() % 256 ) );
}

void TopLevel::defaultColors()
{
    panel->insertDefaultColors();
}

void TopLevel::clearColors()
{
    panel->clear();
}

void TopLevel::slotColorSelected( const QColor& color )
{
    kDebug() << "#### selected: " << color.name() << endl;
}


int main( int argc, char ** argv )
{
    srand( time( 0 ) );

    KApplication a( argc, argv, "KColorAction Test" );
    TopLevel *toplevel = new TopLevel( 0, "coloractiontest" );
    a.setMainWidget( toplevel );
    toplevel->show();
    return a.exec();
}

#include <coloraction_test.moc>
