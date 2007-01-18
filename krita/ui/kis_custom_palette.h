/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#ifndef KIS_CUSTOM_PALETTE_H_
#define KIS_CUSTOM_PALETTE_H_

#include <QObject>

#include "ui_wdgcustompalette.h"


class KisResource;
class KisView2;
class KisPalette;
class KisResourceServerBase;
class KisResourceMediator;

class KisWdgCustomPalette : public QWidget, public Ui::KisWdgCustomPalette
{
    Q_OBJECT

    public:
        KisWdgCustomPalette( QWidget *parent, const char *name) : QWidget(parent) { setObjectName(name); setupUi(this); }
};

class KisCustomPalette : public KisWdgCustomPalette
{
    Q_OBJECT
public:
    KisCustomPalette( QList<KisPalette*> &palettes, QWidget *parent, const char* name, const QString& caption, KisView2* view);
    virtual ~KisCustomPalette();
    void setResourceServer(KisResourceServerBase* server) { m_server = server; }
    void setPalette(KisPalette* p);

signals:
    void activatedResource(KisResource *);

private slots:
    void slotAddPalette();
    void slotAddNew();
    void slotRemoveCurrent();
    void slotPaletteChanged( QListWidgetItem * current, QListWidgetItem * previous );

private:
    bool m_ownPalette;
    bool m_editMode;
    KisView2* m_view;
    KisPalette* m_palette;
    KisResourceMediator* m_mediator;
    KisResourceServerBase* m_server;
    QMap<QListWidgetItem*, KisPalette*> m_palettes;
};


#endif // KIS_CUSTOM_PALETTE_H_
