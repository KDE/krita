/*
   Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>

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
#ifndef _KO_TOOLBOX_H_
#define _KO_TOOLBOX_H_

#include <qtoolbutton.h>
#include <q3ptrvector.h>
#include <q3toolbar.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3PtrList>
#include <koffice_export.h>
#include <ktoolbar.h>

class QWidget;
class KAction;
class KMainWindow;
class KDualColorButton;
class Q3GridLayout;
class ToolArea;


/**
 * KActionBox is a kind of super-specialized toolbox that can order
 * tools according to type and priority.
 *
 * This is to a large extent a port of the Karbon vtoolbox -- with
 * which it should be merged one day. However, it doesn't depend on a
 * tool-like class, it aggregates actions.
 */

class KOFFICEUI_EXPORT KoToolBox : public KToolBar {

    Q_OBJECT

public:

    KoToolBox( KMainWindow *mainWin, const char* name, KInstance* instance, int numberOfTooltypes);
    virtual ~KoToolBox();

    // Called by the toolcontroller for each tool. For every category,
    // there is a separate list, and the tool is categorized correctly.
    // The tool is not yet added to the widgets; call setupTools()
    // to do that. We don't store the tool.
    void registerTool(KAction * tool, int toolType, quint32 priority);

    // Called when all tools have been added by the tool controller
    void setupTools();

public slots:

    virtual void setOrientation ( Qt::Orientation o );
    void slotButtonPressed( int id );
    void slotPressButton( int id );

    // Enables or disables all buttons and the corresponding actions.
    void enableTools(bool enable);

    void slotSetTool(const QString & toolname);

private:

    QToolButton * createButton(QWidget * parent, const QIcon& icon, QString tooltip);


private:
    quint32 m_numberOfButtons;

    Q3ButtonGroup * m_buttonGroup; // The invisible group of all toolbuttons, so only one can be active at a given time

    Q3PtrList<ToolArea> m_toolBoxes; // For every ToolArea

    typedef QMap< int, KAction*> ToolList; // The priority ordered list of tools for a certain tooltype

    Q3PtrList<ToolList> m_tools;
    Q3PtrList<KAction> m_idToActionMap; // Map the buttongroup id's to actions for easy activating.
    KInstance* m_instance;
};


class ToolArea : public QWidget {

public:
    ToolArea(QWidget *parent);
    ~ToolArea();

    void  setOrientation ( Qt::Orientation o );
    void  add(QWidget *button);

    QWidget* getNextParent();

private:
    Q3PtrList<QWidget>  m_children;
    Q3BoxLayout        *m_layout;

    QWidget           *m_leftRow;
    Q3BoxLayout        *m_leftLayout;

    QWidget           *m_rightRow;
    Q3BoxLayout        *m_rightLayout;

    bool               m_left;
};


#endif // _KO_TOOLBOX_H_
