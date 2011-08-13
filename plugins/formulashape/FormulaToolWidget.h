/* This file is part of the KDE project
   Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>

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

#ifndef FORMULATOOLWIDGET_H
#define FORMULATOOLWIDGET_H

#include <QTabWidget>
#include <QMenu>
#include "ui_FormulaToolWidget.h"

class KoFormulaTool;
class QTableWidgetItem;

/**
 * @short A widget providing options for the FormulaTool
 *
 * The FormulaToolOptions widget provides the combobox - listwidget combination
 * which is used to select templates for inserting in the formula. Further the
 * widget provides two buttons - save and load formula. For saving and loading it
 * uses the loadOdf() and saveOdf() methods of KoFormulaShape.
 */
class FormulaToolWidget : public QTabWidget, Ui::mainTabWidget {
Q_OBJECT
public:
    /// Standart constructor
    explicit FormulaToolWidget( KoFormulaTool* tool, QWidget* parent = 0 );

    /// Standart destructor
    ~FormulaToolWidget();

    /// Set the KoFormulaTool @p tool this options widget belongs to
    void setFormulaTool( KoFormulaTool* tool );

public slots:
    void insertSymbol(QTableWidgetItem* item);

private:
    void setupButton(QToolButton* button, QMenu& menu, const QString& text, QList<QString>, int length=8);

    static QList<QString> symbolsInRange(int start, int length);
    
private:
    /// The KoFormulaTool this options widget belongs to
    KoFormulaTool* m_tool;

    QMenu m_scriptsMenu;
    QMenu m_fractionMenu;
    QMenu m_tableMenu;
    QMenu m_fenceMenu;
    QMenu m_rootMenu;
    QMenu m_arrowMenu;
    QMenu m_greekMenu;
    QMenu m_miscMenu;
    QMenu m_relationMenu;
    QMenu m_operatorMenu;
    QMenu m_alterTableMenu;
    QMenu m_rowMenu;
};

#endif // FORMULATOOLWIDGET_H
