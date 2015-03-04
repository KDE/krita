/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOPACONFIGUREDIALOG_H
#define KOPACONFIGUREDIALOG_H

#include <kpagedialog.h>

class KoPAView;
class KoConfigDocumentPage;
class KoConfigGridPage;
class KoConfigMiscPage;
class KoConfigAuthorPage;

class KoPAConfigureDialog : public KPageDialog
{
    Q_OBJECT

public:
    explicit KoPAConfigureDialog(KoPAView *parent);

Q_SIGNALS:
    /// Emitted when the configuration may have changed.
    void changed();

public Q_SLOTS:
    void slotApply();
    void slotDefault();

private:
    KoConfigGridPage *m_gridPage;
    KoConfigDocumentPage *m_docPage;
    KoConfigMiscPage *m_miscPage;
    KoConfigAuthorPage *m_authorPage;
};

#endif // KOPACONFIGUREDIALOG_H
