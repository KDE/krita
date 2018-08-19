/* This file is part of the KDE project
 * Copyright (C) 2011 Sven Langkamp <sven.langkamp@gmail.com>
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


#ifndef KIS_WORKSPACE_CHOOSER_H
#define KIS_WORKSPACE_CHOOSER_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QGridLayout;
class KoResourceItemChooser;
class KisViewManager;
class KoResource;
class KoAbstractResourceServerAdapter;

class KisWorkspaceChooser : public QWidget
{
    Q_OBJECT
public:
    KisWorkspaceChooser(KisViewManager * view, QWidget* parent = 0);
    ~KisWorkspaceChooser() override;

private Q_SLOTS:
    void slotSaveWorkspace();
    void workspaceSelected( KoResource * resource );

    void slotSaveWindowLayout();
    void windowLayoutSelected( KoResource * resource );

private:
    struct ChooserWidgets
    {
        KoResourceItemChooser *itemChooser;
        QLineEdit *nameEdit;
        QPushButton *saveButton;
    };

    KisViewManager *m_view;

    QGridLayout* m_layout;
    ChooserWidgets m_workspaceWidgets;
    ChooserWidgets m_windowLayoutWidgets;

    ChooserWidgets createChooserWidgets(QSharedPointer<KoAbstractResourceServerAdapter> adapter, const QString &title);
};

#endif // KIS_WORKSPACE_CHOOSER_H
