/*
 *  SPDX-FileCopyrightText: 2016 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QHBoxLayout>
#include <QString>
#include <QHash>

class KisShortcutsDialog;
class QPushButton;
class QComboBox;

class KShortcutSchemesEditor: public QHBoxLayout
{
    Q_OBJECT
public:
    KShortcutSchemesEditor(KisShortcutsDialog *parent);

    /** @return the currently selected scheme in the editor (may differ from current app's scheme.*/
    QString currentScheme();

private Q_SLOTS:
    void newScheme();
    void deleteScheme();
    void importShortcutsScheme();
    void exportShortcutsScheme();
    void loadCustomShortcuts();
    void saveCustomShortcuts();
    // void saveAsDefaultsForScheme();  //Not implemented

Q_SIGNALS:
    void shortcutsSchemeChanged(const QString &);

protected:
    void updateDeleteButton();

private:
    QPushButton *m_newScheme;
    QPushButton *m_deleteScheme;
    QPushButton *m_exportScheme;
    QComboBox *m_schemesList;

    KisShortcutsDialog *m_dialog;
    QHash<QString, QString> m_schemeFileLocations;
};

