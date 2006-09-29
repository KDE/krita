/* This file is part of the KDE project
   Copyright (C) 2005 Peter Simonsson <psn@linux.se>

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
#include "KoDetailsPane.h"

#include <QCheckBox>
#include <QLabel>
#include <QFile>
#include <QImage>
#include <QRect>
#include <QBrush>
#include <QPainter>
#include <QEvent>
#include <QSplitter>
#include <QPixmap>
#include <QStandardItemModel>

#include <kinstance.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kconfig.h>
#include <kurl.h>
#include <kfileitem.h>
#include <kio/previewjob.h>
#include <kdebug.h>
#include <ktextbrowser.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include "KoTemplates.h"


////////////////////////////////////
// class KoDetailsPane
///////////////////////////////////

class KoDetailsPanePrivate
{
  public:
    KoDetailsPanePrivate() :
      m_instance(0)
    {
    }

    KInstance* m_instance;
    QStandardItemModel* m_model;
};

KoDetailsPane::KoDetailsPane(QWidget* parent, KInstance* _instance, const QString& header)
  : QWidget(parent), Ui_KoDetailsPaneBase()
{
  d = new KoDetailsPanePrivate;
  d->m_instance = _instance;
  d->m_model = new QStandardItemModel;
  d->m_model->setHorizontalHeaderItem(0, new QStandardItem(header));

  setupUi(this);

  m_previewLabel->installEventFilter(this);
  m_documentList->installEventFilter(this);
  m_documentList->setIconSize(QSize(64, 64));
  m_documentList->setModel(d->m_model);

  changePalette();

  connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(changePalette()));

  connect(m_documentList->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
          this, SLOT(selectionChanged(const QModelIndex&)));
  connect(m_documentList, SIGNAL(doubleClicked(const QModelIndex&)),
          this, SLOT(openFile(const QModelIndex&)));
  connect(m_openButton, SIGNAL(clicked()), this, SLOT(openFile()));
}

KoDetailsPane::~KoDetailsPane()
{
  delete d;
}

KInstance* KoDetailsPane::instance()
{
  return d->m_instance;
}

bool KoDetailsPane::eventFilter(QObject* watched, QEvent* e)
{
  if(watched == m_previewLabel) {
    if(e->type() == QEvent::MouseButtonDblClick) {
      openFile();
    }
  }

  if(watched == m_documentList) {
    if((e->type() == QEvent::Resize) && isVisible()) {
      emit splitterResized(this, m_splitter->sizes());
    }

    if((e->type() == QEvent::KeyPress)) {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);

      if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
        openFile();
      }
    }
  }

  return false;
}

void KoDetailsPane::resizeSplitter(KoDetailsPane* sender, const QList<int>& sizes)
{
  if(sender == this)
  return;

  m_splitter->setSizes(sizes);
}

void KoDetailsPane::openFile()
{
  QModelIndex index = m_documentList->selectionModel()->currentIndex();
  openFile(index);
}

void KoDetailsPane::changePalette()
{
  QPalette p = kapp ? kapp->palette() : palette();
  p.setBrush(QColorGroup::Base, p.brush(QPalette::Normal, QColorGroup::Background));
  p.setColor(QColorGroup::Text, p.color(QPalette::Normal, QColorGroup::Foreground));
  m_detailsLabel->setPalette(p);
}

QStandardItemModel* KoDetailsPane::model() const
{
  return d->m_model;
}

#include "KoDetailsPane.moc"
