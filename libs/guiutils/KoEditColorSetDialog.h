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

#include "ui_KoEditColorSet.h"

#include <KDialog>

#include <koguiutils_export.h>

class QGridLayout;
class QScrollArea;
class KoColorPatch;
class KoColorSet;

class KoEditColorSet : public QWidget
{
    Q_OBJECT
public:
    KoEditColorSet(const QList<KoColorSet *> &palettes, const QString &activePalette, QWidget *parent = 0);
    virtual ~KoEditColorSet();
    KoColorSet *activeColorSet();

private slots:
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
};

class KOGUIUTILS_EXPORT KoEditColorSetDialog : public KDialog
{
    Q_OBJECT

public:
    KoEditColorSetDialog(const QList<KoColorSet *> &palettes, const QString &activePalette, QWidget *parent = 0);
    KoColorSet *activeColorSet();

    /**
     * Destructor
     */
    virtual ~KoEditColorSetDialog();

private:
    KoEditColorSet *ui;
};

#endif

