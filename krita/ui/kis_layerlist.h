/*
  Copyright (c) 2005 Gábor Lehel <illissius@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KIS_LAYERLIST_H
#define KIS_LAYERLIST_H

#include <qimage.h>
#include "layerlist.h"

class KoPartSelectAction;
class KoDocumentEntry;
class KisLayer;

class KisLayerList: public LayerList
{
    Q_OBJECT
    typedef LayerList super;

signals:
    void requestNewObjectLayer( LayerItem *parent, LayerItem *after, const KoDocumentEntry &entry );
    void requestNewObjectLayer( int parentID, int afterID, const KoDocumentEntry &entry );
    void requestNewAdjustmentLayer( LayerItem *parent, LayerItem *after );
    void requestNewAdjustmentLayer( int parentID, int afterID );

public:
    KisLayerList( QWidget *parent = 0, const char *name = 0 );

    virtual void constructMenu( LayerItem *layer );
    virtual void menuActivated( int id, LayerItem *layer );

    KoPartSelectAction *partLayerAction() const { return m_partLayerAction; }

private:
    KoPartSelectAction *m_partLayerAction;
};

class KisLayerItem: public LayerItem
{
    typedef LayerItem super;

public:
    KisLayerItem( LayerList* parent, KisLayer* layer );
    KisLayerItem( LayerItem* parent, KisLayer* layer );

    KisLayer* layer() const;

    void sync();

    /// returns whether any preview was retrieved
    bool updatePreview();

    virtual QString tooltip() const;
    virtual QImage tooltipPreview() const;

    //virtual void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );

private:
    void init();
    QImage m_preview;
    KisLayer *m_layer;
};

#endif
