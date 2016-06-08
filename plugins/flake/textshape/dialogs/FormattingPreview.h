/* This file is part of the KDE project
   Copyright (C)  2008 Pierre Stirnweiss <pstirnweiss@googlemail.com>
   Copyright (C)  2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>

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

#ifndef FORMATTINGPREVIEW_H
#define FORMATTINGPREVIEW_H

#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>

#include <QFont>
#include <QFrame>
#include <QWidget>

class QString;
class KoStyleThumbnailer;

class FormattingPreview : public QFrame
{
    Q_OBJECT

public:
    explicit FormattingPreview(QWidget *parent = 0);
    ~FormattingPreview();

public Q_SLOTS:
    ///Character properties
    void setCharacterStyle(const KoCharacterStyle *style);
    void setParagraphStyle(const KoParagraphStyle *style);

    void setText(const QString &sampleText);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QString m_sampleText;

    KoCharacterStyle *m_characterStyle;
    KoParagraphStyle *m_paragraphStyle;
    KoStyleThumbnailer *m_thumbnailer;
    bool m_previewLayoutRequired;
};

#endif //FORMATTINGPREVIEW_H
