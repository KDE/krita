/*
 *  SPDX-FileCopyrightText: 2018 Michael Zhou <simeirxh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDLGPALETTEEDITOR_H
#define KISDLGPALETTEEDITOR_H

#include <QDialog>
#include <QPointer>
#include <QPair>
#include <QScopedPointer>
#include <QHash>
#include <QSet>

#include <KoColorSet.h>

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
    void slotColCountChanged(int);

    void slotAccepted();

private:
    QString oldNameFromNewName(const QString &newName) const;

private:
    QScopedPointer<Ui_WdgDlgPaletteEditor> m_ui;
    QScopedPointer<QAction> m_actAddGroup;
    QScopedPointer<QAction> m_actDelGroup;
    QScopedPointer<QAction> m_actRenGroup;
    QScopedPointer<KisPaletteEditor> m_paletteEditor;
    QSharedPointer<KoColorSet> m_colorSet;
    QString m_currentGroupOriginalName;

    QPalette m_normalPalette;
    QPalette m_warnPalette;
};

#endif // KISKisDlgPaletteEditor_H
