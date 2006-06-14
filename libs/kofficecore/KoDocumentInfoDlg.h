/* This file is part of the KDE project
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>
                 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#include <koffice_export.h>
#include <kpagedialog.h>

class KoDocumentInfo;

/**
 * @short The dialog that shows information about the document
 * @author Simon Hausmann <hausmann@kde.org>
 * @author Martin Pfeiffer <hubipete@gmx.net>
 * @see KoDocumentInfo
 *
 * This dialog is invoked by KoMainWindow and shows the content
 * of the given KoDocumentInfo class. It consists of several pages,
 * one showing general information about the document and an other
 * showing information about the author.
 * This dialog implements only things that are stored in the OASIS
 * meta.xml file and therefore available through the KoDocumentInfo
 * class.
 * The widgets shown in the tabs are koDocumentInfoAboutWidget and
 * koDocumentInfoAuthorWidget. This class here is derived from
 * KDialogBase and uses it in the TabbedMode.
 */

class KOFFICECORE_EXPORT KoDocumentInfoDlg : public KPageDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor
     * @param parent a pointer to the parent widget
     * @param docInfo a pointer to the shown KoDocumentInfo
     */
    KoDocumentInfoDlg( QWidget *parent, KoDocumentInfo* docInfo );

    /** The destructor */
    virtual ~KoDocumentInfoDlg();

  public slots:
    /** Connected to the applyClicked() signal */
    void slotApply();

  private slots:
    /** Connected with clicked() from pbReset - Reset parts of the metadata */
    void slotResetMetaData();
    /** Connected with clicked() from pbDelete - Delete all author metadata */
    void slotDeleteAuthorInfo();
    /** Connected with clicked() from pbLoadKABC - Load metadata from KABC */
    void slotLoadFromKABC();

  private:
    /** Sets up the aboutWidget and fills the widgets with content */
    void initAboutTab();
    /** Sets up the authorWidget and fills the widgets with content */
    void initAuthorTab();
    /** Saves the changed data back to the KoDocumentInfo class */
    void saveAboutData();
    /** Saves the changed data back to the KoDocumentInfo class */
    void saveAuthorData();

    class KoDocumentInfoDlgPrivate;
    KoDocumentInfoDlgPrivate *d;
};

#endif
