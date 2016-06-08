#ifndef TABLEOFCONTENTSPREVIEW_H
#define TABLEOFCONTENTSPREVIEW_H
/* This file is part of the KDE project
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include <KoZoomHandler.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextRangeManager.h>

#include <QFrame>
#include <QPixmap>

class TextShape;
class KoTableOfContentsGeneratorInfo;
class KoStyleManager;

class TableOfContentsPreview : public QFrame
{
    Q_OBJECT
public:
    explicit TableOfContentsPreview(QWidget *parent = 0);
    ~TableOfContentsPreview();
    void setStyleManager(KoStyleManager *styleManager);
    /// sets the size of the generated preview pixmap if not set then it takes the widget's size
    void setPreviewSize(const QSize &size);
    QPixmap previewPixmap();

protected:
    void paintEvent(QPaintEvent *event);

Q_SIGNALS:
    void pixmapGenerated();
public Q_SLOTS:
    void updatePreview(KoTableOfContentsGeneratorInfo *info);

private Q_SLOTS:
    void finishedPreviewLayout();

private:
    TextShape *m_textShape;
    QPixmap *m_pm;
    KoZoomHandler m_zoomHandler;
    KoStyleManager *m_styleManager;
    KoInlineTextObjectManager m_itom;
    KoTextRangeManager m_tlm;
    QSize m_previewPixSize;

    void deleteTextShape();

};

#endif // TABLEOFCONTENTSPREVIEW_H
