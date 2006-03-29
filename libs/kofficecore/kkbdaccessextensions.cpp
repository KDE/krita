/* This file is part of the KDE project
   Copyright (C) 2005, Gary Cramblitt <garycramblitt@comcast.net>

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

// Qt includes
#include <qsplitter.h>
#include <q3dockwindow.h>
#include <q3dockarea.h>
#include <qevent.h>
#include <qcursor.h>
#include <qobject.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <kxmlguifactory.h>
//Added by qt3to4:
#include <Q3PtrList>
#include <QKeyEvent>
#include <Q3Frame>
#include <Q3ValueList>
#include <QMouseEvent>

// KDE includes
#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kmainwindow.h>
#include <kaction.h>
#include <kdebug.h>
#include <q3tl.h>

// KKbdAccessExtensions includes
#include "kkbdaccessextensions.h"
// TODO: See eventFilter method.
//#include "kkbdaccessextensions.moc"

class KPanelKbdSizerIcon : public QCursor
{
    public:
        KPanelKbdSizerIcon() :
            QCursor(Qt::SizeAllCursor),
            isActive(false)
        {
            currentPos = QPoint(-1, -1);
        }

        ~KPanelKbdSizerIcon()
        {
            hide();
        }

        void show(const QPoint p) {
            if (!isActive) {
                originalPos = QCursor::pos();
                kapp->setOverrideCursor(*this);
                isActive = true;
            }
            if (p != pos())
                setPos(p);
            currentPos = p;
        }

        void hide() {
            if (isActive) {
                kapp->restoreOverrideCursor();
                QCursor::setPos(originalPos);
            }
            isActive = false;
        }

        void setShape(Qt::CursorShape shayp)
        {
            if (shayp != shape()) {
                // Must restore and override to get the icon to refresh.
                if (isActive) kapp->restoreOverrideCursor();
                QCursor::setShape(shayp);
                if (isActive) kapp->setOverrideCursor(*this);
            }
        }

        // Return the difference between a position and where icon is supposed to be.
        QSize delta(const QPoint p)
        {
            QPoint d = p - currentPos;
            return QSize(d.x(), d.y());
        }

        // Return the difference between where the icon is currently positioned and where
        // it is supposed to be.
        QSize delta() { return delta(pos()); }

        // True if the sizing icon is visible.
        bool isActive;

    private:
        // Icon's current position.
        QPoint currentPos;
        // Mouse cursor's original position when icon is shown.
        QPoint originalPos;
};

class KKbdAccessExtensionsPrivate
{
    public:
        KKbdAccessExtensionsPrivate() :
            fwdAction(0),
            revAction(0),
            accessKeysAction(0),
            panel(0),
            handleNdx(0),
            icon(0),
            stepSize(10),
            accessKeyLabels(0) {};

        ~KKbdAccessExtensionsPrivate()
        {
            delete icon;
            // TODO: This crashes, but should delete in the event that KMainWindow is not deleted.
            if (accessKeyLabels) {
                accessKeyLabels->setAutoDelete(false);
                delete accessKeyLabels;
            }
        }

        // Action that starts panel sizing (defaults to F8), forward and reverse;
        KAction* fwdAction;
        KAction* revAction;

        // Action that starts access keys.
        KAction* accessKeysAction;

        // The splitter or dockwindow currently being sized.  If 0, sizing is not in progress.
        QWidget* panel;

        // Index of current handle of the panel.  When panel is a QDockWindow:
        //      1 = size horizontally
        //      2 = size vertically
        uint handleNdx;

        // Sizing icon.
        KPanelKbdSizerIcon* icon;

        // Sizing increment.
        int stepSize;

        // List of the access key QLabels.  If not 0, access keys are onscreen.
        Q3PtrList<QLabel>* accessKeyLabels;

        // Pointer to the KMainWindow.
        KMainWindow* mainWindow;
};

KKbdAccessExtensions::KKbdAccessExtensions(KMainWindow* parent, const char* name) :
    QObject(parent, name)
{
    // kDebug() << "KKbdAccessExtensions::KKbdAccessExtensions: running." << endl;
    d = new KKbdAccessExtensionsPrivate;
    d->mainWindow = parent;
    d->fwdAction = new KAction(i18n("Resize Panel Forward"), KShortcut("F8"),
        0, 0, parent->actionCollection(), "resize_panel_forward");
    d->revAction = new KAction(i18n("Resize Panel Reverse"), KShortcut("Shift+F8"),
        0, 0, parent->actionCollection(), "resize_panel_reverse");
    d->accessKeysAction = new KAction(i18n("Access Keys"), KShortcut("Alt+F8"),
        0, 0, parent->actionCollection(), "access_keys");
    // "Disable" the shortcuts so we can see them in eventFilter.
    d->fwdAction->setEnabled(false);
    d->revAction->setEnabled(false);
    d->accessKeysAction->setEnabled(false);
    d->icon = new KPanelKbdSizerIcon();
    kapp->installEventFilter(this);
}

KKbdAccessExtensions::~KKbdAccessExtensions()
{
    kapp->removeEventFilter(this);
    if (d->panel) exitSizing();
    delete d;
}

int KKbdAccessExtensions::stepSize() const { return d->stepSize; }

void KKbdAccessExtensions::setStepSize(int s) { d->stepSize = s; }

bool KKbdAccessExtensions::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() == QEvent::KeyPress ) {
        // TODO: This permits only a single-key shortcut.  For example, Alt+S,R would not work.
        // If user configures a multi-key shortcut, it is undefined what will happen here.
        // It would be better to handle these as KShortcut activate() signals, but the problem
        // is that once a QDockWindow is undocked and has focus, the KShortcut activate() signals
        // don't fire anymore.
        KShortcut fwdSc = d->fwdAction->shortcut();
        KShortcut revSc = d->revAction->shortcut();
        KShortcut accessKeysSc = d->accessKeysAction->shortcut();
        QKeyEvent* kev = dynamic_cast<QKeyEvent *>(e);
        KKey k = KKey(kev);
        KShortcut sc = KShortcut(k);
        // kDebug() << "KKbdAccessExtensions::eventFilter: Key press " << sc << endl;
        if (!d->accessKeyLabels) {
            if (sc == fwdSc) {
                nextHandle();
                return true;
            }
            if (sc == revSc) {
                prevHandle();
                return true;
            }
        }
        if (d->panel) {
            if (k == KKey(Qt::Key_Escape))
                exitSizing();
            else
                resizePanelFromKey(kev->key(), kev->state());
            // Eat the key.
            return true;
        }
        if (sc == accessKeysSc && !d->panel) {
            if (d->accessKeyLabels) {
                delete d->accessKeyLabels;
                d->accessKeyLabels = 0;
            } else
                displayAccessKeys();
            return true;
        }
        if (d->accessKeyLabels) {
            if (k == KKey(Qt::Key_Escape)) {
                delete d->accessKeyLabels;
                d->accessKeyLabels = 0;
            } else
                handleAccessKey(kev);
            return true;
        }
        return false;
    }
    else if (d->icon->isActive && e->type() == QEvent::MouseButtonPress) {
        exitSizing();
        return true;
    }
    else if (d->accessKeyLabels && e->type() == QEvent::MouseButtonPress) {
        delete d->accessKeyLabels;
        d->accessKeyLabels = 0;
        return true;
    }
/*    else if (e->type() == QEvent::MouseMove && d->icon->isActive) {
        // Lock mouse cursor down.
        showIcon();
        dynamic_cast<QMouseEvent *>(e)->accept();
        return true;
    }*/
    else if (e->type() == QEvent::MouseMove && d->icon->isActive && d->panel) {
        // Resize according to mouse movement.
        QMouseEvent* me = dynamic_cast<QMouseEvent *>(e);
        QSize s = d->icon->delta();
        int dx = s.width();
        int dy = s.height();
        resizePanel(dx, dy, me->state());
        me->accept();
        showIcon();
        return true;
    }
    else if (e->type() == QEvent::Resize && d->panel && o == d->panel) {
        // TODO: This doesn't always work.
        showIcon();
    }
    return false;
}

QWidgetList* KKbdAccessExtensions::getAllPanels()
{
    QWidgetList allWidgets = kapp->allWidgets();
    QWidgetList* allPanels = new QWidgetList;
    foreach ( QWidget* widget, allWidgets ) {
        if (widget->isVisible()) {
            if (qobject_cast<QSplitter*>( widget )) {
                // Only size QSplitters with at least two handles (there is always one hidden).
                if (dynamic_cast<QSplitter *>(widget)->sizes().count() >= 2)
                    allPanels->append(widget);
            } else if (qobject_cast<Q3DockWindow*>( widget )) {
                if (dynamic_cast<Q3DockWindow *>(widget)->isResizeEnabled()) {
                    // kDebug() << "KKbdAccessExtensions::getAllPanels: QDockWindow = " << widget->name() << endl;
                    allPanels->append(widget);
                }
            }
        }
    }
    return allPanels;
}

void KKbdAccessExtensions::nextHandle()
{
    QWidget* panel = d->panel;
    // See if current panel has another handle.  If not, find next panel.
    if (panel) {
        bool advance = true;
        d->handleNdx++;
        if (qobject_cast<QSplitter*>( panel ))
            advance = (d->handleNdx >= dynamic_cast<QSplitter *>(panel)->sizes().count());
        else
            // Undocked windows have only one "handle" (center).
            advance = (d->handleNdx > 2 || !dynamic_cast<Q3DockWindow *>(panel)->area());
        if (advance) {
            QWidgetList* allWidgets = getAllPanels();
            int index = allWidgets->indexOf(panel);
            panel = 0;
            if (index > -1)
                panel = allWidgets->at(index+1);
            delete allWidgets;
            d->handleNdx = 1;
        }
    } else {
        // Find first panel.
        QWidgetList* allWidgets = getAllPanels();
        panel = allWidgets->first();
        delete allWidgets;
        d->handleNdx = 1;
    }
    d->panel = panel;
    if (panel)
        showIcon();
    else
        exitSizing();
}

void KKbdAccessExtensions::prevHandle()
{
    QWidget* panel = d->panel;
    // See if current panel has another handle.  If not, find previous panel.
    if (panel) {
        bool rewind = true;
        d->handleNdx--;
        rewind = (d->handleNdx < 1);
        if (rewind) {
            QWidgetList* allWidgets = getAllPanels();
            int index = allWidgets->indexOf(panel);
            panel = 0;
            if (index > 0)
                panel = allWidgets->at(index-1);
            delete allWidgets;
            if (panel) {
                if (qobject_cast<QSplitter*>( panel ))
                    d->handleNdx = dynamic_cast<QSplitter *>(panel)->sizes().count() - 1;
                else {
                    if (dynamic_cast<Q3DockWindow *>(panel)->area())
                        d->handleNdx = 2;
                    else
                        d->handleNdx = 1;
                }
            }
        }
    } else {
        // Find last panel.
        QWidgetList* allWidgets = getAllPanels();
        panel = allWidgets->last();
        delete allWidgets;
        if (panel) {
            if (qobject_cast<QSplitter*>( panel ))
                d->handleNdx = dynamic_cast<QSplitter *>(panel)->sizes().count() - 1;
            else {
                if (dynamic_cast<Q3DockWindow *>(panel)->area())
                    d->handleNdx = 2;
                else
                    d->handleNdx = 1;
            }
        }
    }
    d->panel = panel;
    if (panel)
        showIcon();
    else
        exitSizing();
}

void KKbdAccessExtensions::exitSizing()
{
    // kDebug() << "KKbdAccessExtensions::exiting sizing mode." << endl;
    hideIcon();
    d->handleNdx = 0;
    d->panel = 0;
}

void KKbdAccessExtensions::showIcon()
{
    if (!d->panel) return;
    QPoint p;
    // kDebug() << "KKbdAccessExtensions::showIcon: topLevelWidget = " << d->panel->topLevelWidget()->name() << endl;
    if (qobject_cast<QSplitter*>( d->panel )) {
        QSplitter* splitter = dynamic_cast<QSplitter *>(d->panel);
        int handleNdx = d->handleNdx - 1;
        Q3ValueList<int> sizes = splitter->sizes();
        // kDebug() << "KKbdAccessExtensions::showIcon: sizes = " << sizes << endl;
        if (splitter->orientation() == Qt::Horizontal) {
            d->icon->setShape(Qt::SizeHorCursor);
            p.setX(sizes[handleNdx] + (splitter->handleWidth() / 2));
            p.setY(splitter->height() / 2);
        } else {
            d->icon->setShape(Qt::SizeVerCursor);
            p.setX(splitter->width() / 2);
            p.setY(sizes[handleNdx] + (splitter->handleWidth() / 2));
        }
        // kDebug() << "KKbdAccessExtensions::showIcon: p = " << p << endl;
        p = splitter->mapToGlobal(p);
        // kDebug() << "KKbdAccessExtensions::showIcon: mapToGlobal = " << p << endl;
    } else {
        Q3DockWindow* dockWindow = dynamic_cast<Q3DockWindow *>(d->panel);
        p = dockWindow->pos();
        if (dockWindow->area()) {
            // kDebug() << "KKbdAccessExtensions::showIcon: pos = " << p << " of window = " << dockWindow->parentWidget()->name() << endl;
            p = dockWindow->parentWidget()->mapTo(dockWindow->topLevelWidget(), p);
            // kDebug() << "KKbdAccessExtensions::showIcon: mapTo = " << p << " of window = " << dockWindow->topLevelWidget()->name() << endl;
            // TODO: How to get the handle width?
            if (d->handleNdx == 1) {
                d->icon->setShape(Qt::SizeHorCursor);
                if (dockWindow->area()->orientation() == Qt::Vertical) {
                    if (dockWindow->area()->handlePosition() == Q3DockArea::Normal)
                        // Handle is to the right of the dock window.
                        p.setX(p.x() + dockWindow->width());
                        // else Handle is to the left of the dock window.
                } else
                    // Handle is to the right of the dock window.
                    p.setX(p.x() + dockWindow->width());
                p.setY(p.y() + (dockWindow->height() / 2));
            } else {
                d->icon->setShape(Qt::SizeVerCursor);
                p.setX(p.x() + (dockWindow->width() / 2));
                if (dockWindow->area()->orientation() == Qt::Vertical)
                    // Handle is below the dock window.
                    p.setY(p.y() + dockWindow->height());
                else {
                    if (dockWindow->area()->handlePosition() == Q3DockArea::Normal)
                        // Handle is below the dock window.
                        p.setY(p.y() + dockWindow->height());
                        // else Handle is above the dock window.
                }
            }
            p = dockWindow->topLevelWidget()->mapToGlobal(p);
        } else {
            d->icon->setShape(Qt::SizeAllCursor);
            p = QPoint(dockWindow->width() / 2, dockWindow->height() / 2);
            p = dockWindow->mapToGlobal(p);       // Undocked.  Position in center of window.
        }
    }
    // kDebug() << "KKbdAccessExtensions::showIcon: show(p) = " << p << endl;
    d->icon->show(p);
}

void KKbdAccessExtensions::hideIcon()
{
    d->icon->hide();
}

void KKbdAccessExtensions::resizePanel(int dx, int dy, int state)
{
    int adj = dx + dy;
    if (adj == 0) return;
    // kDebug() << "KKbdAccessExtensions::resizePanel: panel = " << d->panel->name() << endl;
    if (qobject_cast<QSplitter*>( d->panel )) {
        QSplitter* splitter = dynamic_cast<QSplitter *>(d->panel);
        int handleNdx = d->handleNdx - 1;
        Q3ValueList<int> sizes = splitter->sizes();
        // kDebug() << "KKbdAccessExtensions::resizePanel: before sizes = " << sizes << endl;
        sizes[handleNdx] = sizes[handleNdx] + adj;
        // kDebug() << "KKbdAccessExtensions::resizePanel: setSizes = " << sizes << endl;
        splitter->setSizes(sizes);
        QApplication::postEvent(splitter, new QEvent(QEvent::LayoutRequest));
    } else {
        // TODO: How to get the handle width?
        Q3DockWindow* dockWindow = dynamic_cast<Q3DockWindow *>(d->panel);
        if (dockWindow->area()) {
            // kDebug() << "KKbdAccessExtensions::resizePanel: fixedExtent = " << dockWindow->fixedExtent() << endl;
            QSize fe = dockWindow->fixedExtent();
            if (d->handleNdx == 1) {
                // When vertically oriented and dock area is on right side of screen, pressing
                // left arrow increases size.
                if (dockWindow->area()->orientation() == Qt::Vertical &&
                    dockWindow->area()->handlePosition() == Q3DockArea::Reverse) adj = -adj;
                int w = fe.width();
                if (w < 0) w = dockWindow->width();
                w = w + adj;
                if (w > 0 ) dockWindow->setFixedExtentWidth(w);
            } else {
                // When horizontally oriented and dock area is at bottom of screen,
                // pressing up arrow increases size.
                if (dockWindow->area()->orientation() == Qt::Horizontal &&
                    dockWindow->area()->handlePosition() == Q3DockArea::Reverse) adj = -adj;
                int h = fe.height();
                if (h < 0) h = dockWindow->height();
                h = h + adj;
                if (h > 0) dockWindow->setFixedExtentHeight(h);
            }
            dockWindow->updateGeometry();
            QApplication::postEvent(dockWindow->area(), new QEvent(QEvent::LayoutRequest));
            // kDebug() << "KKbdAccessExtensions::resizePanel: fixedExtent = " << dockWindow->fixedExtent() << endl;
        } else {
            if (state == Qt::ShiftModifier) {
                QSize s = dockWindow->size();
                s.setWidth(s.width() + dx);
                s.setHeight(s.height() + dy);
                dockWindow->resize(s);
            } else {
                QPoint p = dockWindow->pos();
                p.setX(p.x() + dx);
                p.setY(p.y() + dy);
                dockWindow->move(p);
            }
        }
    }
}

void KKbdAccessExtensions::resizePanelFromKey(int key, int state)
{
    // kDebug() << "KPanelKdbSizer::resizePanelFromKey: key = " << key << " state = " << state << endl;
    if (!d->panel) return;
    int dx = 0;
    int dy = 0;
    int stepSize = d->stepSize;
    switch (key) {
        case Qt::Key_Left:      dx = -stepSize;     break;
        case Qt::Key_Right:     dx = stepSize;      break;
        case Qt::Key_Up:        dy = -stepSize;     break;
        case Qt::Key_Down:      dy = stepSize;      break;
        case Qt::Key_PageUp:     dy = -5 * stepSize; break;
        case Qt::Key_PageDown:      dy = 5 * stepSize;  break;
    }
    int adj = dx + dy;
    // kDebug() << "KKbdAccessExtensions::resizePanelFromKey: adj = " << adj << endl;
    if (adj != 0)
        resizePanel(dx, dy, state);
    else {
        if (key == Qt::Key_Enter && qobject_cast<Q3DockWindow*>( d->panel )) {
            Q3DockWindow* dockWindow = dynamic_cast<Q3DockWindow *>(d->panel);
            if (dockWindow->area())
                dockWindow->undock();
            else
                dockWindow->dock();
        }
    }
    showIcon();
}

void KKbdAccessExtensions::displayAccessKeys()
{
    // Build a list of valid access keys that don't collide with shortcuts.
    QString availableAccessKeys = "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890";
    QList<KXMLGUIClient*> allClients = d->mainWindow->factory()->clients();
    foreach ( KXMLGUIClient* client, allClients )
    {
        QList<KAction*> actions = client->actionCollection()->actions();
        for (int j = 0; j < (int)actions.count(); j++) {
            KAction* action = actions[j];
            KShortcut sc = action->shortcut();
            for (int i = 0; i < (int)sc.count(); i++) {
                KKeySequence seq = sc.seq(i);
                if (seq.count() == 1) {
                    QString s = seq.toString();
                    if (availableAccessKeys.contains(s))
                        availableAccessKeys.remove(s);
                }
            }
        }
    }
    // Find all visible, focusable widgets and create a QLabel for each.  Don't exceed
    // available list of access keys.
    QWidgetList allWidgets = kapp->allWidgets();
    int accessCount = 0;
    int maxAccessCount = availableAccessKeys.length();
    int overlap = 20;
    QPoint prevGlobalPos = QPoint(-overlap, -overlap);
    foreach ( QWidget* widget, allWidgets ) {
        if (accessCount >= maxAccessCount)
            break;
        if (widget->isVisible() && widget->focusPolicy() != Qt::NoFocus ) {
            QRect r = widget->rect();
            QPoint p(r.x(), r.y());
            // Don't display an access key if within overlap pixels of previous one.
            QPoint globalPos = widget->mapToGlobal(p);
            QPoint diffPos = globalPos - prevGlobalPos;
            if (diffPos.manhattanLength() > overlap) {
                accessCount++;
                QLabel* lab=new QLabel(widget, "", widget, 0, Qt::WDestructiveClose);
                lab->setPalette(QToolTip::palette());
                lab->setLineWidth(2);
                lab->setFrameStyle(Q3Frame::Box | Q3Frame::Plain);
                lab->setMargin(3);
                lab->adjustSize();
                lab->move(p);
                if (!d->accessKeyLabels) {
                    d->accessKeyLabels = new Q3PtrList<QLabel>;
                    d->accessKeyLabels->setAutoDelete(true);
                }
                d->accessKeyLabels->append(lab);
                prevGlobalPos = globalPos;
            }
        }
    }
    if (accessCount > 0) {
        // Sort the access keys from left to right and down the screen.
        Q3ValueList<KSortedLabel> sortedLabels;
        for (int i = 0; i < accessCount; i++)
            sortedLabels.append(KSortedLabel(d->accessKeyLabels->at(i)));
        qHeapSort( sortedLabels );
        // Assign access key labels.
        for (int i = 0; i < accessCount; i++) {
            QLabel* lab = sortedLabels[i].label();
            QChar s = availableAccessKeys[i];
            lab->setText(s);
            lab->adjustSize();
            lab->show();
        }
    }
}

// Handling of the HTML accesskey attribute.
bool KKbdAccessExtensions::handleAccessKey( const QKeyEvent* ev )
{
// Qt interprets the keyevent also with the modifiers, and ev->text() matches that,
// but this code must act as if the modifiers weren't pressed
    if (!d->accessKeyLabels) return false;
    QChar c;
    if( ev->key() >= Qt::Key_A && ev->key() <= Qt::Key_Z )
        c = 'A' + ev->key() - Qt::Key_A;
    else if( ev->key() >= Qt::Key_0 && ev->key() <= Qt::Key_9 )
        c = '0' + ev->key() - Qt::Key_0;
    else {
        // TODO fake XKeyEvent and XLookupString ?
        // This below seems to work e.g. for eacute though.
        if( ev->text().length() == 1 )
            c = ev->text()[ 0 ];
    }
    if( c.isNull())
        return false;

    QLabel* lab = d->accessKeyLabels->first();
    while (lab) {
        if (lab->text() == c) {
            lab->buddy()->setFocus();
            delete d->accessKeyLabels;
            d->accessKeyLabels = 0;
            return true;
        }
        lab = d->accessKeyLabels->next();
    }
    return false;
}

KSortedLabel::KSortedLabel(QLabel* l) :
    m_l(l) { }

KSortedLabel::KSortedLabel() :
    m_l(0) { }

bool KSortedLabel::operator<( const KSortedLabel& l ) const
{
    QPoint p1 = m_l->mapToGlobal(m_l->pos());
    QPoint p2 = l.label()->mapToGlobal(l.label()->pos());
    return (p1.y() < p2.y() || (p1.y() == p2.y() && p1.x() < p2.x()));
}
