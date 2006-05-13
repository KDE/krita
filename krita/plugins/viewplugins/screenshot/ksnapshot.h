// -*- c++ -*-
/* 
 * (c) Richard J. Moore 1997-2002
 * (c) Matthias Ettrich 2000
 * (c) Aaron J. Seigo 2002
 * (c) Nadeem Hasan 2003
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

#ifndef KSNAPSHOT_H
#define KSNAPSHOT_H

#include <QLabel>
#include <QPixmap>
#include <QTimer>
//Added by qt3to4:
#include <QMouseEvent>
#include <QEvent>

#include <dcopclient.h>
#include <kglobalsettings.h>
#include <kdialogbase.h>
#include <kurl.h>

class RegionGrabber;
class KSnapshotWidget;

class KSnapshotThumb : public QLabel
{
    Q_OBJECT

public:
    KSnapshotThumb(QWidget *parent, const char *name = 0)
        : QLabel(parent, name)
        {
            setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }
    virtual ~KSnapshotThumb() {}

signals:
    void startDrag();

protected:
    void mousePressEvent(QMouseEvent * e)
        {
            mClickPt = e->pos();
        }

    void mouseMoveEvent(QMouseEvent * e)
        {
            if (mClickPt != QPoint(0, 0) &&
                (e->pos() - mClickPt).manhattanLength() > KGlobalSettings::dndEventDelay())
            {
                mClickPt = QPoint(0, 0);
                emit startDrag();
            }
        }

    void mouseReleaseEvent(QMouseEvent * /*e*/)
        {
            mClickPt = QPoint(0, 0);
        }

    QPoint mClickPt;
};

class KSnapshot : public KDialogBase
{
    typedef KDialogBase super;
    Q_OBJECT

public:
    KSnapshot(QWidget *parent= 0, const char *name= 0);
    ~KSnapshot();

    enum CaptureMode { FullScreen=0, WindowUnderCursor=1, Region=2 };

    bool save( const QString &filename );
    bool save( const KUrl& url );

    QString url() const { return filename.url(); }

signals:
    void screenGrabbed();

protected slots:
    void slotGrab();
    void slotCopy();
    void slotPrint();
    void slotMovePointer( int x, int y );

    void setTime(int newTime);
    void setURL(const QString &newURL);
    void setGrabMode( int m );
    void exit();

    void slotOk();


protected:
    void reject() { close(); }
    bool eventFilter( QObject*, QEvent* );
    
private slots:
    void grabTimerDone();
    void slotDragSnapshot();
    void slotRegionGrabbed( const QPixmap & );

private:
    void updatePreview();
    void performGrab();
    void autoincFilename();

    QPixmap snapshot;
    QTimer grabTimer;
    QWidget* grabber;
    KUrl filename;
    KSnapshotWidget *mainWidget;
    RegionGrabber *rgnGrab;
    bool modified;
    bool haveXShape;
};

#endif // KSNAPSHOT_H

