/*
 *  Copyright (c) 2016 Michael Abrahams <miabraha@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

