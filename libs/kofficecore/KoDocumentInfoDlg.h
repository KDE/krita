/* This file is part of the KDE project
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>

   $Id$

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

#ifndef __koDocumentInfoDlg_h__
#define __koDocumentInfoDlg_h__

#include <kpropertiesdialog.h>
#include <koffice_export.h>

class KDialogBase;
class KoDocumentInfo;
class KoDocumentInfoAuthor;
class KoDocumentInfoAbout;
class KoDocumentInfoUserMetadata;
class KArchiveEntry;

class KOFFICECORE_EXPORT KoDocumentInfoDlg : public QObject
{
  Q_OBJECT
public:
  KoDocumentInfoDlg( KoDocumentInfo *docInfo, QWidget *parent = 0, const char *name = 0,
		     KDialogBase *dialog = 0 );
  virtual ~KoDocumentInfoDlg();

  int exec();
  KDialogBase *dialog() const;

  void save();

signals:
  void changed();

private slots:
  void loadFromKABC();
  void deleteInfo();
  void resetMetaData();

private:
  void addAuthorPage( KoDocumentInfoAuthor *authorInfo );
  void addAboutPage( KoDocumentInfoAbout *aboutInfo );
  void addUserMetadataPage( KoDocumentInfoUserMetadata *userMetadataInfo );

  void save( KoDocumentInfoAuthor *authorInfo );
  void save( KoDocumentInfoAbout *aboutInfo );
  void save( KoDocumentInfoUserMetadata *userMetadataInfo );

  class KoDocumentInfoDlgPrivate;
  KoDocumentInfoDlgPrivate *d;
};

class KOFFICECORE_EXPORT KoDocumentInfoPropsPage : public KPropsDlgPlugin
{
  Q_OBJECT
public:
  KoDocumentInfoPropsPage( KPropertiesDialog *props, const char *name = 0,
                           const QStringList & = QStringList() );
  virtual ~KoDocumentInfoPropsPage();

  virtual void applyChanges();

private:
  void copy( const QString &path, const KArchiveEntry *entry );
  class KoDocumentInfoPropsPagePrivate;
  KoDocumentInfoPropsPagePrivate *d;
};

#endif
