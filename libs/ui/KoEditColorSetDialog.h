/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#ifndef KOEDITCOLORSET_H
#define KOEDITCOLORSET_H

#include <ui_KoEditColorSet.h>

#include <KoDialog.h>

#include "kritawidgets_export.h"

class QGridLayout;
class QScrollArea;
class KoColorPatch;
class KoColorSet;

class KoEditColorSetWidget : public QWidget
{
    Q_OBJECT
public:
    KoEditColorSetWidget(const QList<KoColorSet *> &palettes, const QString &activePalette, QWidget *parent = 0);
    ~KoEditColorSetWidget() override;

    /**
     * Return the active color set. The caller takes ownership of that color set.
     */
    KoColorSet *activeColorSet();

private Q_SLOTS:
    void setActiveColorSet(int index);
    void setTextLabel(KoColorPatch *patch);
    void addColor();
    void removeColor();
    void open();
    void save();

private:
    Ui::KoEditColorSet widget;
    QList<KoColorSet *> m_colorSets;
    QGridLayout *m_gridLayout;
    QScrollArea *m_scrollArea;
    KoColorSet *m_activeColorSet;
    KoColorPatch *m_activePatch;
    uint m_initialColorSetCount;
    bool m_activeColorSetRequested;
};

/**
 * A dialog for editing palettes/color sets in an application. Example use of this dialog is in text color toolbar,
 * the toolbar brings a set of colors from one palette, and a button brings this dialog for editing palettes.
 * This dialog is able to:
 * - Set active palette from a combobox
 * - Add/remove color from a palette
 * - Open new palette from a gimp palette file (.gpl)
 * - Save changes to the file
 * @see KoColorSetWidget
 */
class KRITAWIDGETS_EXPORT KoEditColorSetDialog : public KoDialog
{
    Q_OBJECT

public:
    /**
     * Constructs a KoEditColorSetDialog.
     * @param palettes all available palettes that are going to be edited.
     * @param activePalette name of the palette which will be activated after this dialog is shown.
     * @param parent the parent widget
     */
    KoEditColorSetDialog(const QList<KoColorSet *> &palettes, const QString &activePalette, QWidget *parent = 0);

    /**
     * Returns the last active color set.
     * The caller takes ownership of that color set.
     * @return the last active KoColorSet in the dialog before the user press OK
     */
    KoColorSet *activeColorSet();

    /**
     * Destructor
     */
    ~KoEditColorSetDialog() override;

private:
    KoEditColorSetWidget *ui;
};

#endif

