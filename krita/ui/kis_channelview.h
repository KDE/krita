/*
 *  kis_channelview.h - part of Krayon
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *                1999 Michael Koch    <koch@kde.org>
 *                2000 Matthias Elter  <elter@kde.org>
 *                2001 John Califf     <jcaliff@compuzone.net>
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

#ifndef __kis_channelview_h__
#define __kis_channelview_h__

#define CELLWIDTH   200
#define CELLHEIGHT  40
#define MAXROWS	    8

#include <qdialog.h>
#include <qgridview.h>
#include <qrect.h>
#include <kdialogbase.h>

class KisDoc;
class KisView;
class QPopupMenu;
class QLineEdit;
class QPixmap;
class IntegerWidget;
class ChannelTable;
class KisChannel;
class QHBox;
class QToolButton;

class KisChannelView : public QWidget
{
    Q_OBJECT

public:
    KisChannelView( KisDoc* doc, QWidget* _parent = 0,  const char* _name = 0 );
    ~KisChannelView();
    void showScrollBars();
    ChannelTable *channelTable() { return channeltable; }
    QHBox *getFrame() const { return frame; };

private:
    void initGUI();
    ChannelTable *channeltable;
    QHBox *frame;
    QHBox *buttons;

    QToolButton * pbAddChannel;
    QToolButton * pbRemoveChannel;
    QToolButton * pbUp;
    QToolButton * pbDown;
};

class ChannelTable : public QGridView {
    Q_OBJECT

    typedef QGridView super;

public:

    enum action { VISIBLE, ADDCHANNEL, REMOVECHANNEL, RAISECHANNEL, LOWERCHANNEL };

    ChannelTable( QWidget *_parent = 0, const char *_name = 0 );
    ChannelTable( KisDoc *_doc, QWidget *_parent = 0, const char *_name = 0 );

    // this one is used because it keeps a reference to the ChannelView
    ChannelTable(KisDoc* doc, QWidget* _parent = 0,
        KisChannelView *_channelview = 0, const char* name = 0 );

    void updateTable();
    void updateAllCells();
    void update_contextmenu( int _index );

    void selectChannel( int _index );
    void slotInverseVisibility( int _index );
    void slotProperties();
    virtual QSize sizeHint() const;

public slots:

    void slotMenuAction( int );
    void slotAddChannel();
    void slotRemoveChannel();

protected:

    virtual void paintCell( QPainter*, int _row, int _col );
    virtual void mousePressEvent( QMouseEvent *_event );

private:

    void init(KisDoc* doc);

    KisDoc* m_doc;
    KisChannelView* pChannelView;

    int m_items, m_selected;
    QPopupMenu* m_contextmenu;
    QPixmap *mVisibleIcon, *mNovisibleIcon;
    QPixmap *mLinkedIcon, *mUnlinkedIcon;
    QRect mVisibleRect, mLinkedRect, mPreviewRect;
};

class ChannelPropertyDialog : public KDialogBase
{
    Q_OBJECT

public:

    static bool editProperties( KisChannel& _channel );

protected:

    ChannelPropertyDialog( QString _channelname, uchar _opacity,
    QWidget *_parent, const char *_name );

    QLineEdit *m_name;
    IntegerWidget *m_opacity;
};

#endif


