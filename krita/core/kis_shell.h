/*
 *  kis_shell.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __kimageshop_shell_h__
#define __kimageshop_shell_h__

#include <koMainWindow.h>

class KisShell : public KoMainWindow
{
  Q_OBJECT

public:

    KisShell( const char* name = 0 );
    ~KisShell();

    QString nativeFormatPattern() const { return "*.kis"; }
    QString nativeFormatName() const;

public slots:

    virtual void slotFileNew();
    
    //virtual void slotFileNewDocument();
    //virtual void slotFileAddNewImage();
    //virtual void slotFileRemoveCurrentImage();

    virtual void slotFilePrint();
    virtual void slotFileClose();

    virtual void statusMsg( const QString& );

protected:

    virtual bool openDocument( const KURL & url );

private:

    QLabel *m_pMessageLabel;
};

#endif // __kimageshop_shell_h__
