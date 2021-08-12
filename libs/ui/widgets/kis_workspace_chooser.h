/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */


#ifndef KIS_WORKSPACE_CHOOSER_H
#define KIS_WORKSPACE_CHOOSER_H

#include <QWidget>
#include <KoResource.h>

class QLineEdit;
class QPushButton;
class QGridLayout;
class KisResourceItemChooser;
class KisViewManager;

class KisWorkspaceChooser : public QWidget
{
    Q_OBJECT
public:
    KisWorkspaceChooser(KisViewManager * view, QWidget* parent = 0);
    ~KisWorkspaceChooser() override;

private Q_SLOTS:
    void slotSaveWorkspace();
    void slotUpdateWorkspaceSaveButton();
    void workspaceSelected( KoResourceSP resource );

    void slotSaveWindowLayout();
    void slotUpdateWindowLayoutSaveButton();
    void windowLayoutSelected( KoResourceSP resource );

private:
    struct ChooserWidgets
    {
        KisResourceItemChooser *itemChooser;
        QLineEdit *nameEdit;
        QPushButton *saveButton;
    };

    KisViewManager *m_view;

    QGridLayout* m_layout;
    ChooserWidgets m_workspaceWidgets;
    ChooserWidgets m_windowLayoutWidgets;

    ChooserWidgets createChooserWidgets(const QString &resourceType, const QString &title);
};

#endif // KIS_WORKSPACE_CHOOSER_H
