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

#include <QList>
#include <QMap>
#include <koffice_export.h>
#include <ktoolbar.h>

class QButtonGroup;
class QToolButton;
class QGridLayout;
class QBoxLayout;
class QAbstractButton;
class QWidget;

class KAction;
class KMainWindow;
class ToolArea;


/**
 * KoToolBox is a kind of super-specialized toolbox that can order
 * tools according to type and priority.
 *
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
    void slotButtonPressed( QAbstractButton * );

    // Enables or disables all buttons and the corresponding actions.
    void enableTools(bool enable);

    void slotSetTool(const QString & toolname);

private:

    QToolButton * createButton(QWidget * parent, const QIcon& icon, QString tooltip);

private:

    QButtonGroup * m_buttonGroup; // The invisible group of all toolbuttons, so only one can be active at a given time

    QList<ToolArea *> m_toolBoxes; // For every ToolArea

    typedef QMap< int, KAction*> ToolList; // The priority ordered list of tools for a certain tooltype

    QList<ToolList *> m_tools;
    QMap<QAbstractButton *, KAction *> m_actionMap; // Map the buttongroup id's to actions for easy activating.
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
    QList<QWidget *>  m_children;
    QBoxLayout        *m_layout;

    QWidget           *m_leftRow;
    QBoxLayout        *m_leftLayout;

    QWidget           *m_rightRow;
    QBoxLayout        *m_rightLayout;

    bool               m_left;
};


#endif // _KO_TOOLBOX_H_
