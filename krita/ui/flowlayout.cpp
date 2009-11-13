/****************************************************************************
 **
 ** Copyright (C) 2004-2008 Trolltech ASA. All rights reserved.
 **
 ** This file is part of the documentation of the Qt Toolkit.
 **
 ** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

 #include <QtGui>

 #include "flowlayout.h"

 FlowLayout::FlowLayout(QWidget *parent, int margin, int spacing)
     : QLayout(parent)
 {
     setMargin(margin);
     setSpacing(spacing);
 }

 FlowLayout::FlowLayout(int spacing)
 {
     setSpacing(spacing);
 }

 FlowLayout::~FlowLayout()
 {
     QLayoutItem *item;
     while ((item = takeAt(0)))
         delete item;
 }

 void FlowLayout::addItem(QLayoutItem *item)
 {
     itemList.append(item);
 }

 int FlowLayout::count() const
 {
     return itemList.size();
 }

 QLayoutItem *FlowLayout::itemAt(int index) const
 {
     return itemList.value(index);
 }

 QLayoutItem *FlowLayout::takeAt(int index)
 {
     if (index >= 0 && index < itemList.size())
         return itemList.takeAt(index);
     else
         return 0;
 }

 Qt::Orientations FlowLayout::expandingDirections() const
 {
     return 0;
 }

 bool FlowLayout::hasHeightForWidth() const
 {
     return true;
 }

 int FlowLayout::heightForWidth(int width) const
 {
     int height = doLayout(QRect(0, 0, width, 0), true);
     return height;
 }

 void FlowLayout::setGeometry(const QRect &rect)
 {
     QLayout::setGeometry(rect);
     doLayout(rect, false);
 }

 QSize FlowLayout::sizeHint() const
 {
     return minimumSize();
 }

 QSize FlowLayout::minimumSize() const
 {
     QSize size;
     QLayoutItem *item;
     foreach (item, itemList)
         size = size.expandedTo(item->minimumSize());

     size += QSize(2*margin(), 2*margin());
     return size;
 }

 int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
 {
     int x = rect.x();
     int y = rect.y();
     int lineHeight = 0;

     QLayoutItem *item;
     foreach (item, itemList) {
         int nextX = x + item->sizeHint().width() + spacing();
         if (nextX - spacing() > rect.right() && lineHeight > 0) {
             x = rect.x();
             y = y + lineHeight + spacing();
             nextX = x + item->sizeHint().width() + spacing();
             lineHeight = 0;
         }

         if (!testOnly)
             item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

         x = nextX;
         lineHeight = qMax(lineHeight, item->sizeHint().height());
     }
     return y + lineHeight - rect.y();
 }
