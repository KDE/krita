/*
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2005-2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef _KO_TOOLBOX_H_
#define _KO_TOOLBOX_H_

#include <QList>
#include <QMap>
#include <QDockWidget>
#include <koffice_export.h>
#include <kinstance.h>

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
 * The ToolBox is a container for tool buttons which are themselves divided into sections.
 * Adding buttons using addButton() will allow you to show those buttons.  You should connect
 * the button to your handling method yourself.
 * The unique property of this toolbox is that it can be shown horizontal as well as vertical,
 * rotating in a smart way to show the buttons optimally.
 */
class KOFFICEUI_EXPORT KoToolBox : public QDockWidget {
    Q_OBJECT
public:
    /// constructor
    KoToolBox();
    ~KoToolBox();

    /**
     * Add a button to the toolbox.
     * The buttons should all be added before the fist showing since adding will not really add
     * them to the UI until setup() is called.
     * @param button the new button.  Please make sure you connect to the button yourself.
     * @param section the section in which this button will be shown.  Each section will be its own
     *        widget.
     * @param priority the priority in the section. Lowest value means it will be shown first.
     * @param buttonGroupId if passed this will allow you to use setActiveTool() to trigger
     *      this button
     * @see setup()
     */
    void addButton(QAbstractButton *button, const QString &section, int priority, int buttonGroupId=-1);

    /**
     * Setup the toolbox by adding the buttons in the right configuration to the ui.
     * You should only call this method one time, and you should call it prior to showing.
     */
    void setup();

    /**
     * For a button added to this toolbox you can regiter a visibility-code by which that
     * button will be known.  Using setButtonsVisible() you can then show only the buttons
     * you want visible.
     * @param button the previously registred button
     * @param code that we recognize this button by, does not have to be unique.
     */
    void setVisibilityCode(QAbstractButton *button, const QString &code);

public slots:
    /**
     * Using the buttongroup id passed in addButton() you can set the new active button.
     * If the id does not resolve to a visible button, this call is ignored.
     * @param id an id to identify the button to activate.
     */
    void setActiveTool(int id);

    /**
     * Show only the dynamic buttons that have a code from parameter codes.
     * The toolbox allows buttons to be optionally registred with a visibilityCode. This code
     * can be passed here and all buttons that have that code are shown. All buttons that
     * have another visibility code registred are hidden.
     * @param codes a list of all the codes to show.
     */
    void setButtonsVisible(const QList<QString> &codes);

private:
    QButtonGroup *m_buttonGroup;
    QBoxLayout* m_layout;
    QList<ToolArea *> m_toolBoxes;
    QMap<QAbstractButton*, QString> m_visibilityCodes;

    // Section,  [prio, button]
    QMap<QString, QMultiMap<int, QAbstractButton*> > m_buttons;
    QMap<QString, ToolArea*> m_toolAreas;

    void showEvent(QShowEvent *event);

#if 0

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
#endif
};

/// \internal
class ToolArea : public QWidget {

public:
    ToolArea(QWidget *parent);
    ~ToolArea();

    void setOrientation (Qt::Orientation orientation);
    void add(QWidget *button);

    QWidget* getNextParent();

private:
    QList<QWidget *> m_children;
    QBoxLayout *m_layout;

    QWidget *m_leftRow;
    QWidget *m_rightRow;
    QBoxLayout *m_leftLayout;
    QBoxLayout *m_rightLayout;

    bool m_left;
};


#endif // _KO_TOOLBOX_H_
