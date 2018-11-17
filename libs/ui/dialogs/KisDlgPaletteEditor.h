/*
 *  Copyright (c) 2018 Michael Zhou <simeirxh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
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

#ifndef KISDLGPALETTEEDITOR_H
#define KISDLGPALETTEEDITOR_H

#include <QDialog>
#include <QPointer>
#include <QPair>
#include <QScopedPointer>
#include <QHash>
#include <QSet>
#include <QButtonGroup>

#include "kritaui_export.h"

class QAction;

class KoColorSet;
class KisPaletteModel;
class KisSwatchGroup;
class KoDialog;
class KisViewManager;

class KisPaletteEditor;
class Ui_WdgDlgPaletteEditor;

/**
 * @brief The KisDlgPaletteEditor class
 * a dialog used by the palette docker to make modifications to a palette.
 * it automatically uploads all changes into the resource server when
 * the change is accepted
 */
class KRITAUI_EXPORT KisDlgPaletteEditor : public QDialog
{
    Q_OBJECT
public:
    explicit KisDlgPaletteEditor();
    ~KisDlgPaletteEditor();

public:
    void setPaletteModel(KisPaletteModel *);
    KoColorSetSP palette() const { return m_colorSet; }
    void setView(KisViewManager *);

private Q_SLOTS:
    void slotDelGroup();
    void slotAddGroup();
    void slotRenGroup();

    void slotGroupChosen(const QString &groupName);

    void slotRowCountChanged(int);
    void slotSetGlobal();

    void slotNameChanged();
    void slotFilenameChanged(const QString &newFilename);
    void slotFilenameInputFinished();
    void slotColCountChanged(int);

    void slotAccepted();

private:
    QString oldNameFromNewName(const QString &newName) const;

private:
    QScopedPointer<Ui_WdgDlgPaletteEditor> m_ui;
    QScopedPointer<QAction> m_actAddGroup;
    QScopedPointer<QAction> m_actDelGroup;
    QScopedPointer<QAction> m_actRenGroup;
    QScopedPointer<QButtonGroup> m_globalButtons;
    QScopedPointer<KisPaletteEditor> m_paletteEditor;
    QSharedPointer<KoColorSet> m_colorSet;
    QString m_currentGroupOriginalName;

    QPalette m_normalPalette;
    QPalette m_warnPalette;
};

#endif // KISKisDlgPaletteEditor_H
