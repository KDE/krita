/* This file is part of the KDE project
   Copyright (C)  2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#ifndef __KoImportStyleDia__
#define __KoImportStyleDia__

#include <kdialog.h>
#include <QStringList>
#include <KoStyleCollection.h>
class QLineEdit;
class Q3ListBox;
class QPushButton;

class KOTEXT_EXPORT KoImportStyleDia : public KDialog
{
    Q_OBJECT
public:
    /// @param currentCollection collection of styles already present in the document
    /// (this is used to avoid conflicts in the style names and displayNames)
    KoImportStyleDia( KoStyleCollection* currentCollection, QWidget *parent, const char *name );
    ~KoImportStyleDia();

    const KoStyleCollection& importedStyles() const { return m_styleList; }

protected slots:
    virtual void slotOk();
    void slotLoadFile();

protected:
    /**  Open file dialog and load the list of styles from the selected doc.
     */
    virtual void loadFile()=0;

    void clear();
    QString generateStyleName( const QString & templateName ) const;
    QString generateStyleDisplayName( const QString & templateName ) const;

    // @return collection of styles already present in the document
    const KoStyleCollection* currentCollection() const { return m_currentCollection; }

    void initList();

    // used by subclasses, hmm...
    Q3ListBox *m_listStyleName;
    KoStyleCollection m_styleList;

private:
    void generateStyleList();
    void updateFollowingStyle( KoParagStyle* removedStyle );

    KoStyleCollection* m_currentCollection; // Styles already present in the document
};

#endif
