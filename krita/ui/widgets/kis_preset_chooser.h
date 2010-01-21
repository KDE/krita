/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#ifndef KIS_ITEM_CHOOSER_H_
#define KIS_ITEM_CHOOSER_H_

#include <QWidget>
#include <krita_export.h>

class KoResourceItemChooser;
class KoResource;

/**
 * A special type of item chooser that can contain extra widgets that show
 * more information about the currently selected item. Reimplement update()
 * to extract that information and fill the appropriate widgets.
 */
class KRITAUI_EXPORT KisPresetChooser : public QWidget
{

    Q_OBJECT

public:

    KisPresetChooser(QWidget *parent = 0, const char *name = 0);
    virtual ~KisPresetChooser();

signals:
    void resourceSelected( KoResource * resource );

private:
    KoResourceItemChooser *m_chooser;
};

#endif // KIS_ITEM_CHOOSER_H_

