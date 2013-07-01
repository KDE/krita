/* This file is part of the KDE project
 * Copyright (C) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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


#ifndef COLORSET_CHOOSER_H
#define COLORSET_CHOOSER_H

#include <QWidget>

class KoColorSet;
class KLineEdit;
class KoResourceItemChooser;
class KisView2;
class KoResource;

class ColorSetChooser : public QWidget
{
    Q_OBJECT
public:
    ColorSetChooser(QWidget* parent = 0);
    virtual ~ColorSetChooser();

signals:
    void paletteSelected(KoColorSet* colorSet);

private slots:
    void resourceSelected(KoResource* resource);
    
private:
    KoResourceItemChooser * m_itemChooser;
};

#endif // COLORSET_CHOOSER_H
