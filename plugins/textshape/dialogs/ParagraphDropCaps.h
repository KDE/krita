/* This file is part of the KDE project

   Copyright (C)  2011 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>
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

#ifndef PARAGRAPHDROPCAPS_H
#define PARAGRAPHDROPCAPS_H
#include "ui_ParagraphDropCaps.h"

#include "KoUnit.h"
#include <QWidget>

class KoParagraphStyle;

namespace Ui {
    class ParagraphDropCaps;
}

class ParagraphDropCaps : public QWidget
{
    Q_OBJECT

public:
  ParagraphDropCaps(QWidget *parent = 0);

  void setDisplay(KoParagraphStyle *style);
  void save(KoParagraphStyle *style);

  void setUnit(const KoUnit &unit);

signals:
  void parStyleChanged();

private slots:
  void dropCapsStateChanged();
  void paragraphDistanceChanged(qreal distance);
  void dropsLineSpanChanged(int lineSpan);
  void dropedCharacterCountChanged(int count);

private:
    Ui::ParagraphDropCaps widget;

    bool m_dropCapsInherited;
    bool m_capsDistanceInherited;
    bool m_capsLengthInherited;
    bool m_capsLinesInherited;
};

#endif // PARAGRAPHDROPCAPS_H
