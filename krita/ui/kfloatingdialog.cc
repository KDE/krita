/*
 *  kfloatingdialog.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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

#include <qcursor.h>
#include <qpainter.h>
#include <qrect.h>

#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kapplication.h>

#include "kfloatingdialog.h"
#include "kis_factory.h"

KFloatingDialog::KFloatingDialog(QWidget *parent, const char* name)
    : QFrame(parent, name)
{
    setFocusPolicy(QWidget::StrongFocus);
    setMouseTracking(true);
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(FRAMEBORDER);

    m_pParent = parent;
    m_shaded = false;
    m_dragging = false;
    m_resizing = false;
    m_cursor = false;

    m_pBase = 0L;

    if (m_pParent)
    {
        m_docked = true;
        m_dockedPos = pos();
    }
    else
    {
        m_docked = false;
        m_dockedPos = QPoint(0,0);
    }

    m_pCloseButton = 0L;
    m_pMinButton  = 0L;
    m_pDockButton  = 0L;

#if 0
    // setup title buttons
    m_pCloseButton = new QPushButton(this);
    QPixmap close_pm( locate("kis_pics", "close.png", KisFactory::global()));
    m_pCloseButton->setPixmap(close_pm);
    m_pCloseButton->setGeometry(width()-FRAMEBORDER-13, FRAMEBORDER+1, 12, 12);
    connect(m_pCloseButton, SIGNAL(clicked()), this, SLOT(slotClose()));

    m_pMinButton = new QPushButton(this);
    QPixmap min_pm( locate("kis_pics", "minimize.png", KisFactory::global()));
    m_pMinButton->setPixmap(min_pm);
    m_pMinButton->setGeometry(width()-FRAMEBORDER-26, FRAMEBORDER+1, 12, 12);
    connect(m_pMinButton, SIGNAL(clicked()), this, SLOT(slotMinimize()));

    m_pDockButton = new QPushButton(this);
    QPixmap dock_pm( locate("kis_pics", "dock.png", KisFactory::global()));
    m_pDockButton->setPixmap(dock_pm);
    m_pDockButton->setGeometry(width()-FRAMEBORDER-39, FRAMEBORDER+1, 12, 12);
    connect(m_pDockButton, SIGNAL(clicked()), this, SLOT(slotDock()));

    // read config
    readSettings();
#endif
}


KFloatingDialog::~KFloatingDialog()
{
    if (m_pCloseButton)  delete m_pCloseButton;
    if (m_pDockButton)   delete m_pDockButton;
    if (m_pMinButton)    delete m_pMinButton;
}


/*
    read config settings - this is all cosmetic
*/

void KFloatingDialog::readSettings()
{
#if 0
    // query kwmrc for the titlebar look
    KConfig* config = new KConfig("kwmrc", true);

    config->setGroup("WM");

    m_activeBlend = config->readColorEntry("activeBlend" , &(Qt::black));
    m_inactiveBlend
        = config->readColorEntry("inactiveBlend" ,
            &palette().normal().background());

    config->setGroup("General");
    QString key = config->readEntry("TitlebarLook");
    m_titleLook = gradient;
    m_gradientType = KPixmapEffect::HorizontalGradient;

    if( key == "shadedHorizontal")
        m_gradientType = KPixmapEffect::HorizontalGradient;
    else if( key == "shadedVertical")
        m_gradientType = KPixmapEffect::VerticalGradient;
    else if( key == "shadedDiagonal")
        m_gradientType = KPixmapEffect::DiagonalGradient;
    else if( key == "shadedCrossDiagonal")
        m_gradientType = KPixmapEffect::CrossDiagonalGradient;
    else if( key == "shadedRectangle")
        m_gradientType = KPixmapEffect::RectangleGradient;
    else if( key == "shadedElliptic")
        m_gradientType = KPixmapEffect::EllipticGradient;
    else if( key == "shadedPyramid")
        m_gradientType = KPixmapEffect::PyramidGradient;
    else if( key == "shadedPipeCross")
        m_gradientType = KPixmapEffect::PipeCrossGradient;
    else if( key == "plain")
        m_titleLook = plain;
    else if( key == "pixmap")
        m_titleLook = pixmap;

    if (m_titleLook == pixmap )
    {
        m_pActivePm = new QPixmap;
        m_pInactivePm = new QPixmap;
        KIconLoader iconLoader("kwm");
        *(m_pActivePm) = BarIcon("activetitlebar");
        *(m_pInactivePm) = BarIcon("inactivetitlebar");

        if (m_pInactivePm->size() == QSize(0,0))
	        *m_pInactivePm = *m_pActivePm;

        if (m_pActivePm->size() == QSize(0,0))
		    m_titleLook = plain;
    }
#endif
}


void KFloatingDialog::setBaseWidget(QWidget *w)
{
    m_pBase = w;
    setBaseSize( m_pBase->sizeHint() );
}


void KFloatingDialog::slotClose()
{
    hide();
    emit sigClosed();
}


void KFloatingDialog::slotDock()
{
    if (m_docked) // docked -> undock
        setDocked(false);
    else          // undocked -> dock
        setDocked(true);
}

void KFloatingDialog::slotMinimize()
{
    if (!m_docked) showMinimized();
}


void KFloatingDialog::setShaded(bool value)
{
    if (m_shaded == value)
        return;

    m_shaded = value;

    if (m_shaded)
    {
        m_unshadedHeight = height();
        resize(width(), TITLE_HEIGHT);
    }
    else
        resize(width(), m_unshadedHeight);
}


void KFloatingDialog::setDocked(bool value)
{
    if (m_docked == value)
        return;

    m_docked = value;

    // set docked - back to original parent
    if (m_docked)
    {
        if (!m_pParent)
		{
		    m_docked = false;
		    return;
		}
        reparent(m_pParent, 0, m_dockedPos, true);
    }
    // set free floating - keep on top of parent window
    else
    {
        m_dockedPos = pos();
        reparent(0, WStyle_StaysOnTop, mapToGlobal(QPoint(0,0)), true);
        setActiveWindow();
    }
}


void KFloatingDialog::focusInEvent( QFocusEvent *e )
{
    QFrame::focusInEvent( e );
    repaint( false );
}

void KFloatingDialog::focusOutEvent( QFocusEvent *e )
{
    QFrame::focusOutEvent( e );
    repaint( false );
}


void KFloatingDialog::paintEvent(QPaintEvent *e)
{
    if (!isVisible())
        return;

#if 0

    QRect r(FRAMEBORDER, FRAMEBORDER, _width(), GRADIENT_HEIGHT);
    QPainter p;

    p.begin(this);
    p.setClipRect(r);
    p.setClipping(true);

    // pixmap
    if (m_titleLook == pixmap)
    {
        QPixmap *pm = hasFocus() ? m_pActivePm : m_pInactivePm;

        for (int x = r.x(); x < r.x() + r.width(); x+=pm->width())
		    p.drawPixmap(x, r.y(), *pm);
    }
    // gradient
    else if (m_titleLook == gradient)
    {
        QPixmap* pm = 0;

        if (hasFocus())
		{
		    if (m_activeShadePm.size() != r.size())
			{
			  m_activeShadePm.resize(r.width(), r.height());
			  KPixmapEffect::gradient(m_activeShadePm,
                KGlobalSettings::activeTitleColor(),
				m_activeBlend, m_gradientType);
			}
		    pm = &m_activeShadePm;
		}
        else
		{
		    if (m_inactiveShadePm.size() != r.size())
			{
			    m_inactiveShadePm.resize(r.width(), r.height());
			    KPixmapEffect::gradient(m_inactiveShadePm,
                    KGlobalSettings::inactiveTitleColor(),
					m_inactiveBlend, m_gradientType);
			}
		    pm = &m_inactiveShadePm;
		}
        p.drawPixmap(r.x(), r.y(), *pm);
    }
    // plain
    else
    {
      p.setBackgroundColor(hasFocus() ? KGlobalSettings::activeTitleColor()
						   : KGlobalSettings::inactiveTitleColor());
      p.eraseRect(r);
    }

    // paint caption
    p.setPen(hasFocus() ?
        KGlobalSettings::activeTextColor() : KGlobalSettings::inactiveTextColor());

    /* FIXME: we need a global KIS config class that provides
    for example a KIS-global small font p.setFont(tinyFont); */

    // adjust cliprect (don't draw caption text under the buttons)
    r.setRight(r.width() - 41);
    p.setClipRect(r);
    p.drawText(r.x(),
    r.y() + (r.height()-p.fontMetrics().height())/2+p.fontMetrics().ascent(),
	QString(" ")+caption()+" ");

    // TODO: Should we add title animation? ;-)

    p.setClipping(false);
    p.end();

#endif

    QFrame::paintEvent(e);
}

void KFloatingDialog::mouseDoubleClickEvent (QMouseEvent *e)
{
#if 0
    if (e->button() & LeftButton)
    {
        QRect title(0,0, width(), TITLE_HEIGHT);
        if(!title.contains(e->pos()))
		    return;

        if (m_shaded)
		    setShaded(false);
        else
		    setShaded(true);
    }
    else
        QFrame::mouseDoubleClickEvent (e);
#endif

    QFrame::mouseDoubleClickEvent (e);
}


void KFloatingDialog::mousePressEvent(QMouseEvent *e)
{
    raise();

    if(!m_docked)  setActiveWindow();

#if 0
    if (e->button() & LeftButton)
    {
        QPoint pos = e->pos();
        QRect title(0, 0, width(), TITLE_HEIGHT);

        if(bottomRect().contains(pos))
		{
		    m_resizing = true;
		    m_resizeMode = vertical;
		}
        else if(rightRect().contains(pos))
		{
		    m_resizing = true;
		    m_resizeMode = horizontal;
		}
        else if(lowerRightRect().contains(pos))
		{
		    m_resizing = true;
		    m_resizeMode = diagonal;
		}
        else if(title.contains(pos))
		    m_dragging = true;

        if(m_dragging)
		    m_pos = e->pos();

        if(m_resizing)
		    m_oldSize = QPoint(width(), height());
    }
    else
        QFrame::mousePressEvent(e);
#endif

        QFrame::mousePressEvent(e);
}

void KFloatingDialog::mouseMoveEvent(QMouseEvent *e)
{
#if 0
    if (bottomRect().contains(e->pos()) && !m_shaded)
    {
        if (!QApplication::overrideCursor()
        || QApplication::overrideCursor()->shape() != SizeVerCursor)
		{
		    if (m_cursor)
			    QApplication::restoreOverrideCursor();
		    QApplication::setOverrideCursor(sizeVerCursor);
		}
        m_cursor = true;
    }
    else if (rightRect().contains(e->pos()))
    {
        if (!QApplication::overrideCursor()
        || QApplication::overrideCursor()->shape() != SizeHorCursor)
		{
		    if (m_cursor)
			    QApplication::restoreOverrideCursor();
		    QApplication::setOverrideCursor(sizeHorCursor);
		}
        m_cursor = true;
    }
    else if (lowerRightRect().contains(e->pos()) && !m_shaded)
    {
        if (!QApplication::overrideCursor()
        || QApplication::overrideCursor()->shape() != SizeFDiagCursor)
		{
		    if (m_cursor)
			    QApplication::restoreOverrideCursor();
		    QApplication::setOverrideCursor(sizeFDiagCursor);
		}
        m_cursor = true;
    }
    else if (m_cursor)
    {
        QApplication::restoreOverrideCursor();
        m_cursor = false;
    }

    if (m_dragging)
    {
        if(m_pParent && m_docked)
		{
		    QPoint newPos = m_pParent->mapFromGlobal(QCursor::pos()) - m_pos;
		    if (newPos.x() < 0) newPos.setX(0);
		    if (newPos.y() < 0) newPos.setY(0);
		    move(newPos);
		    kdDebug()"move to x :"<<newPos.x()<<" y "<< newPos.y()<<endl;
		}
        else
		{
		  QPoint newPos = (QCursor::pos() - m_pos);
		  //KWM::move(winId(), newPos); //jwc
		}
    }
    else if (m_resizing)
    {
        QPoint cursorPos;

        if(m_pParent && m_docked)
		    cursorPos = m_pParent->mapFromGlobal(QCursor::pos());
        else
		    cursorPos = (QCursor::pos());

        // pos() does not work here
        QPoint newSize = cursorPos -
            QPoint(geometry().left(), geometry().top());

        if (m_resizeMode == vertical)
		    newSize.setX(m_oldSize.x());
        else if (m_resizeMode == horizontal)
		    newSize.setY(m_oldSize.y());

        if (newSize.x() < MIN_WIDTH)
		    newSize.setX(MIN_WIDTH);

        if (newSize.y() < MIN_HEIGHT)
		    newSize.setY(MIN_HEIGHT);

        if(m_shaded)
		    newSize.setY(height());

        if (m_pParent && m_docked)
		    resize(newSize.x(), newSize.y());
        else
            resize(newSize.x(), newSize.y());

        // jwc - no KWM anymore
        /*----
        else
            KWM::setGeometry(winId(), QRect(geometry().left(), geometry().top(),
            newSize.x(), newSize.y()));
        ---*/

        m_oldSize = QPoint(width(), height());
        kdDebug()<<"resize to w "<< newSize.x()<<" h "<< newSize.y()<<endl;
    }
    else
        QFrame::mouseMoveEvent(e);
#endif

        QFrame::mouseMoveEvent(e);
}


void KFloatingDialog::mouseReleaseEvent(QMouseEvent *e)
{
#if 0
    if (e->button() & LeftButton)
    {
        m_dragging = false;
        m_resizing = false;
    }
    else
        QFrame::mouseReleaseEvent(e);
#endif

    QFrame::mouseReleaseEvent(e);
}


void KFloatingDialog::resizeEvent(QResizeEvent *)
{
#if 0
    m_pCloseButton->setGeometry(width()-FRAMEBORDER-13, FRAMEBORDER+1, 12, 12);
    m_pMinButton->setGeometry(width()-FRAMEBORDER-26, FRAMEBORDER+1, 12, 12);
    m_pDockButton->setGeometry(width()-FRAMEBORDER-39, FRAMEBORDER+1, 12, 12);

    if (m_pBase)
        m_pBase->setGeometry(_left(), _top(), _width(), _height());
#endif
}

void  KFloatingDialog::leaveEvent(QEvent *)
{
    if (m_cursor)
    {
        m_cursor = false;
        QApplication::restoreOverrideCursor();
    }
}

#include "kfloatingdialog.moc"
