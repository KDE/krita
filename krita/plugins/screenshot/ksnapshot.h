// -*- c++ -*-

#ifndef KSNAPSHOT_H
#define KSNAPSHOT_H

#include <qlabel.h>
#include <qpixmap.h>
#include <qtimer.h>

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
            setAlignment(AlignHCenter | AlignVCenter);
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
    bool save( const KURL& url );

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
    KURL filename;
    KSnapshotWidget *mainWidget;
    RegionGrabber *rgnGrab;
    bool modified;
    bool haveXShape;
};

#endif // KSNAPSHOT_H

