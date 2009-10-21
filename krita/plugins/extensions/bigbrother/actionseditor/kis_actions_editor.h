/* This file is part of the KDE project
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_ACTIONS_EDITOR_H_
#define _KIS_ACTIONS_EDITOR_H_

#include <QWidget>

class KisMacro;
class KisMacroModel;
class KisRecordedAction;

class QModelIndex;
class QGridLayout;

namespace Ui
{
class ActionsEditor;
}

class KisActionsEditor : public QWidget
{
    Q_OBJECT
public:
    KisActionsEditor(QWidget* parent);
    ~KisActionsEditor();
    void setMacro(KisMacro*);
private slots:
    void slotActionActivated(const QModelIndex&);
    void slotBtnDelete();
    void slotBtnDuplicate();
    void slotBtnRaise();
    void slotBtnLower();
private:
    void setCurrentAction(KisRecordedAction* _action);
private:
    QWidget* m_currentEditor;
    Ui::ActionsEditor* m_form;
    KisMacro* m_macro;
    KisMacroModel* m_model;
    QGridLayout* m_widgetLayout;
};

#endif
