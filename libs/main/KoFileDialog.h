/* This file is part of the KDE project
   Copyright (C) 2002-2004 David Faure <faure@kde.org>
   Copyright (C) 2002-2004 Clarence Dang <dang@kde.org>

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

#ifndef KOFILEDIALOG_H
#define KOFILEDIALOG_H

#include <kfiledialog.h>
#include <QVector>

/**
 * Extension to KFileDialog in order to add special entries to the filter combo,
 * like "save as koffice-1.1", "save as dir" etc.
 * @note Used only when saving!
 *
 * This class is INTERNAL to KoMainWindow, do NOT use in your application.
 * Binary compatibility is not guaranteed.
 */
class KoFileDialog : public KFileDialog
{
    Q_OBJECT
public:
    KoFileDialog(const QString& startDir, QWidget *parent);
    void setSpecialMimeFilter(QStringList& mimeFilter,
                              const QString& currentFormat, const int specialOutputFlag,
                              const QString& nativeFormat,
                              int supportedSpecialFormats);
    int specialEntrySelected();

private slots:
    void slotChangedfilter(int index);

private:
    QVector<int> m_specialFormats;
};

#endif /* KOFILEDIALOG_H */
