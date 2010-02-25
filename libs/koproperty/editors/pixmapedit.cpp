/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2005-2008 Jarosław Staniek <staniek@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include "pixmapedit.h"
#include "utils.h"
#include "koproperty/Property.h"
#include "koproperty/EditorDataModel.h"

#include <QLayout>
#include <QPainter>
#include <QLabel>
#include <QCursor>
#include <QFont>
#include <QImage>
#include <q3filedialog.h>
#include <QToolTip>
#include <QApplication>
#include <QDesktopWidget>
//Added by qt3to4:
#include <QPixmap>
#include <QEvent>
#include <QKeyEvent>
#include <QFrame>
#include <QResizeEvent>
#include <QMouseEvent>

#include <KDebug>
#include <KImageIO>
#include <KPushButton>
#include <KFileDialog>
#include <KLocale>

/* KDE4:
#ifdef Q_WS_WIN
#include <win32_utils.h>
#include <krecentdirs.h>
#endif*/

using namespace KoProperty;

PixmapEdit::PixmapEdit(Property *prop, QWidget *parent)
        : QWidget(parent)
        , m_property(prop)
{
    setBackgroundRole(QPalette::Base);

    QHBoxLayout *lyr = new QHBoxLayout(this);
    lyr->setContentsMargins(0,0,0,0);

    m_edit = new QLabel(this);
    lyr->addWidget(m_edit);
    m_edit->setContentsMargins(0, 1, 0, 0);
    m_edit->setToolTip(i18n("Click to show image preview"));
    m_edit->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
//    m_edit->setMinimumHeight(5);
    m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_edit->setBackgroundRole(QPalette::Base);
    m_edit->setMouseTracking(true);
    m_edit->installEventFilter(this);

    m_button = new KPushButton(i18nc("Three dots for 'Insert image from file' button", "..."), this);
    lyr->addWidget(m_button);
    Utils::setupDotDotDotButton(m_button, i18n("Insert image from file"),
        i18n("Inserts image from file"));

    m_popup = new QLabel(0, Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    m_popup->setBackgroundRole(QPalette::Base);
    m_popup->setFrameStyle(QFrame::Plain | QFrame::Box);
    m_popup->setMargin(2);
    m_popup->setLineWidth(1);
    m_popup->hide();

    setFocusProxy(m_edit);
    connect(m_button, SIGNAL(clicked()), this, SLOT(selectPixmap()));
}

PixmapEdit::~PixmapEdit()
{
    delete m_popup;
}

QVariant PixmapEdit::value() const
{
    return m_pixmap;
}

void PixmapEdit::setValue(const QVariant &value)
{
    m_pixmap = value.value<QPixmap>();
    if (m_pixmap.isNull() || (m_pixmap.height() <= height())) {
//        m_edit->setPixmap(m_pixmap);
        m_previewPixmap = m_pixmap;
    } else {
        QImage img(m_pixmap.toImage());
        const QSize sz(size() - QSize(0,1));
        if (!QRect(QPoint(0, 0), sz).contains(m_pixmap.rect())) {
            img = img.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_previewPixmap = QPixmap::fromImage(img);//preview pixmap is a bit larger
        } else {
            m_previewPixmap = m_pixmap;
//            img = img.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
//        const QPixmap pm( QPixmap::fromImage(img) );
//        m_edit->setPixmap(pm);
    }
//    if (emitChange)
//        emit valueChanged(this);
}

/*
void
PixmapEdit::drawViewer(QPainter *p, const QColorGroup &, const QRect &r, const QVariant &value)
{
    QRect r2(r);
    r2.setHeight(r2.height() + 1);
    p->setClipRect(r2);
    p->setClipping(true);
    p->eraseRect(r2);
    if (value.value<QPixmap>().isNull())
        return;
    if (m_recentlyPainted != value) {
        m_recentlyPainted = value;
        m_scaledPixmap = value.value<QPixmap>();
        if (m_scaledPixmap.height() > r2.height() || m_scaledPixmap.width() > r2.width()) { //scale down
            QImage img(m_scaledPixmap.toImage());
            img = img.scaled(r2.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_scaledPixmap = QPixmap::fromImage(img);
        }
    }
    p->drawPixmap(r2.topLeft().x(),
                  r2.topLeft().y() + (r2.height() - m_scaledPixmap.height()) / 2, m_scaledPixmap);
}*/

QString PixmapEdit::selectPixmapFileName()
{
    /*#ifdef PURE_QT
      QString url = QFileDialog::getOpenFileName();
      if (!url.isEmpty()) {
        m_edit->setPixmap(QPixmap(url));
        emit valueChanged(this);
      }
    #endif*/
    const QString caption(i18n("Insert Image From File (for \"%1\" property)", m_property->caption()));
    /*KDE4:
    #ifdef Q_WS_WIN
      QString recentDir;
      QString fileName = Q3FileDialog::getOpenFileName(
        KFileDialog::getStartURL(":lastVisitedImagePath", recentDir).path(),
        convertKFileDialogFilterToQFileDialogFilter(KImageIO::pattern(KImageIO::Reading)),
        this, 0, caption);
    #else*/
    const KUrl url(KFileDialog::getImageOpenUrl(
                 KUrl(":lastVisitedImagePath"), this, caption));
    QString fileName = url.isLocalFile() ? url.toLocalFile() : url.prettyUrl();

    //! @todo download the file if remote, then set fileName properly
//#endif
    return fileName;
}

void PixmapEdit::selectPixmap()
{
    const QString fileName(selectPixmapFileName());
    if (fileName.isEmpty())
        return;

    QPixmap pm;
    if (!pm.load(fileName)) {
//! @todo err msg
        return;
    }
    setValue(pm);

    /* KDE4:
    #ifdef Q_WS_WIN
      //save last visited path
      KUrl url(fileName);
      if (url.isLocalFile())
        KRecentDirs::add(":lastVisitedImagePath", url.directory());
    #endif
    */
}

/*
void
PixmapEdit::resizeEvent(QResizeEvent *e)
{
    Widget::resizeEvent(e);
    m_edit->move(0, 0);
    m_edit->resize(e->size() - QSize(m_button->width(), -1));
    m_button->move(m_edit->width(), 0);
    m_button->setFixedSize(m_button->width(), height());
}*/

bool
PixmapEdit::eventFilter(QObject *o, QEvent *ev)
{
    if (o == m_edit) {
        if (ev->type() == QEvent::MouseButtonPress && static_cast<QMouseEvent*>(ev)->button() == Qt::LeftButton) {
            if (m_previewPixmap.height() <= m_edit->height()
                    && m_previewPixmap.width() <= m_edit->width())
                return false;

            m_popup->setPixmap(m_previewPixmap.isNull() ? m_pixmap : m_previewPixmap);
            m_popup->resize(m_previewPixmap.size() + QSize(2*3, 2*3));
            QPoint pos = QCursor::pos() + QPoint(3, 15);
            const QRect screenRect = QApplication::desktop()->availableGeometry(this);
            if ((pos.x() + m_popup->width()) > screenRect.width())
                pos.setX(screenRect.width() - m_popup->width());
            if ((pos.y() + m_popup->height()) > screenRect.height())
                pos.setY(mapToGlobal(QPoint(0, 0)).y() - m_popup->height());
            m_popup->move(pos);
            m_popup->show();
        } else if (ev->type() == QEvent::MouseButtonRelease || ev->type() == QEvent::Hide) {
            if (m_popup->isVisible()) {
                m_popup->hide();
            }
        } else if (ev->type() == QEvent::KeyPress) {
            QKeyEvent* e = static_cast<QKeyEvent*>(ev);
            if ((e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Space) || (e->key() == Qt::Key_Return)) {
                m_button->animateClick();
                return true;
            }
        }
    }
    return QWidget::eventFilter(o, ev);
}

/*
void
PixmapEdit::setReadOnlyInternal(bool readOnly)
{
    m_button->setEnabled(!readOnly);
}*/

//-----------------------

PixmapDelegate::PixmapDelegate()
{
//    options.removeBorders = false;
}

QWidget* PixmapDelegate::createEditor( int type, QWidget *parent, 
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    const EditorDataModel *editorModel
        = dynamic_cast<const EditorDataModel*>(index.model());
    Property *property = editorModel->propertyForItem(index);
    PixmapEdit *pe = new PixmapEdit(property, parent);
    return pe;
}

void PixmapDelegate::paint( QPainter * painter, 
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QPixmap pm( index.data(Qt::EditRole).value<QPixmap>() );
    if (pm.isNull())
        return;
    painter->save();
    if (pm.height() > option.rect.height() || pm.width() > option.rect.width()) { //scale down
        QImage img(pm.toImage());
        img = img.scaled(option.rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pm = QPixmap::fromImage(img);
    }
/*todo    if (m_recentlyPainted != value) {
        m_recentlyPainted = value;
        m_scaledPixmap = value.value<QPixmap>();
        if (m_scaledPixmap.height() > r2.height() || m_scaledPixmap.width() > r2.width()) { //scale down
            QImage img(m_scaledPixmap.toImage());
            img = img.scaled(r2.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_scaledPixmap = QPixmap::fromImage(img);
        }
    }*/
    painter->drawPixmap(option.rect.topLeft().x(),
                  option.rect.topLeft().y() + (option.rect.height() - pm.height()) / 2, pm);
    painter->restore();
}

#include "pixmapedit.moc"
