/*
 *  kis_layerview.h - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *                1999 Michael Koch    <koch@kde.org>
 *                2000 Matthias Elter  <elter@kde.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __kis_layerview_h__
#define __kis_layerview_h__

#include <qdialog.h>
#include <qgridview.h>
#include <qrect.h>

#define CELLWIDTH   200
#define CELLHEIGHT  40
#define MAXROWS	    8

class KisDoc;
class KisView;
class QPopupMenu;
class QLineEdit;
class QPixmap;
class IntegerWidget;
class LayerTable;
class KisLayer;
class QHBox;
class KoFrameButton;

class KisLayerView : public QWidget
{
	Q_OBJECT

		public:
	KisLayerView( KisDoc* doc, QWidget* _parent = 0,  const char* _name = 0 );
	~KisLayerView();

	void showScrollBars();
	LayerTable *layerTable() { return layertable; }
	QHBox *getFrame() { return frame; };

 private:
	void initGUI();
	LayerTable *layertable;
	QHBox *frame;
	QHBox *buttons;

	KoFrameButton *pbAddLayer;
	KoFrameButton *pbRemoveLayer;
	KoFrameButton *pbUp;
	KoFrameButton *pbDown;
};

class LayerTable : public QGridView
{
	Q_OBJECT

		typedef QGridView super;

 public:

	enum action { VISIBLE, SELECTION, LINKING, PROPERTIES, ADDLAYER, REMOVELAYER, ADDMASK, REMOVEMASK, UPPERLAYER, LOWERLAYER, FRONTLAYER, BACKLAYER, LEVEL };

	// these are not used
	LayerTable(QWidget* _parent = 0, const char* _name = 0 );
	LayerTable(KisDoc* doc, QWidget* _parent = 0, const char* name = 0 );

	// this one is used because it keeps a reference to the LayerView
	LayerTable(KisDoc* doc, QWidget* _parent = 0,
		   KisLayerView *_layerview = 0, const char* name = 0 );

	void updateTable();
	void updateAllCells();
	void update_contextmenu( int _index );

	void swapLayers( int a, int b );
	void selectLayer( int _index );
	void slotInverseVisibility( int _index );
	void slotInverseLinking( int _index );
	void slotProperties();

	virtual QSize sizeHint() const;

	public slots:

		void slotMenuAction( int );
	void slotAddLayer();
	void slotRemoveLayer();
	void slotRaiseLayer();
	void slotLowerLayer();
	void slotFrontLayer();
	void slotBackgroundLayer();
	void slotAboutToShow();

	void slotDocUpdated();

 protected:

	virtual void paintCell( QPainter*, int _row, int _col );
	virtual void mousePressEvent( QMouseEvent* _event );
	virtual void mouseDoubleClickEvent( QMouseEvent* _event );

 private:

	void init(KisDoc* doc);

	KisDoc* m_doc;
	KisView* m_view;
	KisLayerView* pLayerView;

	int m_items, m_selected;
	QPopupMenu* m_contextmenu;
	QPixmap *mVisibleIcon, *mNovisibleIcon;
	QPixmap *mLinkedIcon,  *mUnlinkedIcon;
	QRect mVisibleRect, mLinkedRect, mPreviewRect;
};

class LayerPropertyDialog : QDialog
{
	Q_OBJECT

		public:

	static bool editProperties( KisLayer& _layer );

 protected:

	LayerPropertyDialog( QString _layername, uchar _opacity,
			     QWidget *_parent, const char *_name );

	QLineEdit *m_name;
	IntegerWidget *m_opacity;
};

#endif // __kis_layerview_h__

