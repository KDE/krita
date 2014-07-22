/* This file is part of the KDE project
 * Copyright (C) 2011 Ganesh Paramasivam <ganesh@crystalfab.com> 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __FORMAT_CHANGE_INFORMATION_H__
#define __FORMAT_CHANGE_INFORMATION_H__

#include <QTextCharFormat>
#include <QTextBlockFormat>

class KoFormatChangeInformation {
    public:
        typedef enum {
            eTextStyleChange = 0,
            eParagraphStyleChange,
            eListItemNumberingChange
        }FormatChangeType;

    KoFormatChangeInformation::FormatChangeType formatType();

    protected:
        explicit KoFormatChangeInformation(KoFormatChangeInformation::FormatChangeType formatChangeType);

    private:
        KoFormatChangeInformation::FormatChangeType formatChangeType;
};

class KoTextStyleChangeInformation:public KoFormatChangeInformation {
    public:
        explicit KoTextStyleChangeInformation(KoFormatChangeInformation::FormatChangeType formatChangeType = KoFormatChangeInformation::eTextStyleChange);
        void setPreviousCharFormat(QTextCharFormat &oldFormat);
        QTextCharFormat& previousCharFormat();
    private:
        QTextCharFormat previousTextCharFormat;
};

class KoParagraphStyleChangeInformation:public KoTextStyleChangeInformation {
    public:
        KoParagraphStyleChangeInformation();
        void setPreviousBlockFormat(QTextBlockFormat &oldFormat);
        QTextBlockFormat& previousBlockFormat();
    private:
        QTextBlockFormat previousTextBlockFormat;
};

class KoListItemNumChangeInformation:public KoFormatChangeInformation {
    public:
        typedef enum {
            eNumberingRestarted = 0,
            eRestartRemoved
        }ListItemNumChangeType;
        explicit KoListItemNumChangeInformation(KoListItemNumChangeInformation::ListItemNumChangeType eSubType);
        void setPreviousStartNumber(int oldRestartNumber);
        KoListItemNumChangeInformation::ListItemNumChangeType listItemNumChangeType();
        int previousStartNumber();
    private:
        int oldStartNumber;
        KoListItemNumChangeInformation::ListItemNumChangeType eSubType;
};
#endif
