/* This file is part of the KDE project
   Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>
                 2009 Jeremias Epperlein <jeeree@web.de>

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
   Boston, MA 02110-1301, USA.
*/

#include "FormulaToolWidget.h"
#include "KoFormulaTool.h"
#include "KoFormulaShape.h"
#include "ElementFactory.h"
#include "BasicElement.h"
#include <QWidgetAction>
#include <QTableWidget>

#include <KAction>

#include <KMessageBox>
#include <QMenu>
#include <kdebug.h>
#include "FormulaCursor.h"

FormulaToolWidget::FormulaToolWidget( KoFormulaTool* tool, QWidget* parent )
                  : QTabWidget( parent )
{
    m_tool = tool;
    setupUi( this );
    // setup the element insert menus
    m_fractionMenu.addAction( m_tool->action( "insert_fraction" ) );
    m_fractionMenu.addAction( m_tool->action( "insert_bevelled_fraction" ) );
    
    
    m_fenceMenu.addAction( m_tool->action( "insert_fence" ) );
    m_fenceMenu.addAction( m_tool->action( "insert_enclosed" ) );
    

    m_tableMenu.addAction( m_tool->action( "insert_33table" ) );
    m_tableMenu.addAction( m_tool->action( "insert_21table" ) );

    m_rootMenu.addAction( m_tool->action( "insert_root" ) );
    m_rootMenu.addAction( m_tool->action( "insert_sqrt" ) );

    m_scriptsMenu.addAction( m_tool->action( "insert_subscript" ) );
    m_scriptsMenu.addAction( m_tool->action( "insert_supscript" ) );
    m_scriptsMenu.addAction( m_tool->action( "insert_subsupscript" ) );
    m_scriptsMenu.addAction( m_tool->action( "insert_underscript" ) );
    m_scriptsMenu.addAction( m_tool->action( "insert_overscript" ) );
    m_scriptsMenu.addAction( m_tool->action( "insert_underoverscript" ) );
    

    m_alterTableMenu.addAction( m_tool->action( "insert_row") );
    m_alterTableMenu.addAction( m_tool->action( "insert_column") );
    m_alterTableMenu.addAction( m_tool->action( "remove_row") );
    m_alterTableMenu.addAction( m_tool->action( "remove_column") );

    // assign menus to toolbuttons
    buttonFence->setMenu( &m_fenceMenu );
    buttonFence->setDefaultAction( m_tool->action( "insert_fence" ) );
    buttonRoot->setMenu( &m_rootMenu );
    buttonRoot->setDefaultAction( m_tool->action( "insert_sqrt" ) );
    buttonFraction->setMenu( &m_fractionMenu );
    buttonFraction->setDefaultAction(m_tool->action("insert_fraction"));
    buttonTable->setMenu( &m_tableMenu );
    buttonTable->setDefaultAction(m_tool->action( "insert_33table"));
    buttonScript->setMenu( &m_scriptsMenu );
    buttonScript->setDefaultAction(m_tool->action( "insert_subscript"));

    buttonAlterTable->setMenu(&m_alterTableMenu);
    buttonAlterTable->setDefaultAction(m_tool->action("insert_row"));
    // setup the buttons for symbol insertion
    buttonArrows->setText(QChar(0x2190));
    setupButton(buttonArrows,m_arrowMenu,i18n("Arrows"), symbolsInRange(0x2190,0x21FF));
    buttonGreek->setText(QChar(0x03B2));
    setupButton(buttonGreek,m_greekMenu,i18n("Greek"), symbolsInRange(0x0391,0x03A1)
                                                     <<symbolsInRange(0x03A3,0x03A9)
                                                     <<symbolsInRange(0x03B1,0x03C9));
    buttonRelation->setText(QChar(0x2265));
    setupButton(buttonRelation,m_relationMenu,i18n("Relations"), symbolsInRange(0x223C,0x2292)
                                                               <<symbolsInRange(0x2AAE,0x2ABA));
    buttonOperators->setText(QChar(0x2211));
    setupButton(buttonOperators,m_operatorMenu,i18n("Operators"), symbolsInRange(0x220F,0x2219)
                                                               <<symbolsInRange(0x2227,0x2233)
                                                               <<symbolsInRange(0x2207,0x2208));
    buttonMisc->setText(QChar(0x211A));
    setupButton(buttonMisc,m_miscMenu,i18n("Miscellaneous"), symbolsInRange(0x2200,0x2205)
                                                                   <<symbolsInRange(0x221F,0x2222));

    buttonRow->hide();
    connect( buttonLoad, SIGNAL( clicked() ), m_tool, SLOT( loadFormula() ) );
    connect( buttonSave, SIGNAL( clicked() ), m_tool, SLOT( saveFormula() ) );
    connect( buttonAlterTable, SIGNAL( triggered( QAction* ) ), m_tool, SLOT( changeTable(QAction*)));
}

FormulaToolWidget::~FormulaToolWidget()
{}


void FormulaToolWidget::setFormulaTool( KoFormulaTool* tool )
{
    m_tool = tool;
}


void FormulaToolWidget::insertSymbol ( QTableWidgetItem* item )
{
    m_tool->insertSymbol(item->text());
}


void FormulaToolWidget::setupButton ( QToolButton* button, QMenu& menu, const QString& text, QList<QString> list, int length)
{
    QWidgetAction *widgetaction=new QWidgetAction(button);
    QTableWidget* table= new QTableWidget(list.length()/length,length,button);
    for (int i=0; i<list.length();i++) {
        QTableWidgetItem *newItem = new QTableWidgetItem(list[i]);
        newItem->setFlags(Qt::ItemIsEnabled);
        table->setItem(i/length,i%length, newItem);
    }
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    table->setShowGrid(false);
    table->setFixedSize(table->horizontalHeader()->length(), table->verticalHeader()->length());
    button->setToolTip(text);
    //TODO: that is a little bit hackish
//     connect( table,SIGNAL( itemActivated(QTableWidgetItem*)),
//              table, SIGNAL( itemClicked(QTableWidgetItem*)));
    connect( table,SIGNAL( itemClicked(QTableWidgetItem*)),
             this, SLOT( insertSymbol(QTableWidgetItem*)));
    connect( table,SIGNAL( itemClicked(QTableWidgetItem*)),
             &menu, SLOT(hide()));
    button->setPopupMode(QToolButton::InstantPopup);
    button->setMenu(&menu);
    
    widgetaction->setDefaultWidget(table);
    menu.addAction(widgetaction);
}

QList< QString > FormulaToolWidget::symbolsInRange ( int first, int last )
{
    QList<QString> list;
    for (int i=first;i<=last;++i) {
        list.append(QChar(i));
    }
    return list;
}


#include "FormulaToolWidget.moc"
