/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Michael Zhou <simeirxh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPALETTELISTWIDGET_H
#define KISPALETTELISTWIDGET_H

#include <QString>
#include <QWidget>
#include <ui_WdgPaletteListWidget.h>

#include "kritawidgets_export.h"

#include <KoColorSet.h>

class KoResource;


struct KisPaletteChooserPrivate;

class KRITAWIDGETS_EXPORT KisPaletteChooser : public QWidget
{
    Q_OBJECT
public:
    explicit KisPaletteChooser(QWidget *parent = nullptr);
    virtual ~KisPaletteChooser();
    void setCurrentItem(KoResourceSP currentResource);

Q_SIGNALS:
    void sigPaletteSelected(KoColorSetSP);
    void sigAddPalette();
    void sigRemovePalette(KoColorSetSP);
    void sigImportPalette();
    void sigExportPalette(KoColorSetSP);

public Q_SLOTS:

private /* methods */:
    QString newPaletteFileName();

public Q_SLOTS:
    void paletteSelected(KoResourceSP);

private Q_SLOTS:
    void slotAdd();
    void slotRemove();
    void slotImport();
    void slotExport();

private:
    QScopedPointer<Ui_WdgPaletteListWidget> m_ui;
    QScopedPointer<KisPaletteChooserPrivate> m_d;
};

#endif // KISPALETTELISTWIDGET_H
