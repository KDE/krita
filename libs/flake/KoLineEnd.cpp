/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2005-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2006 Peter Simonsson <psn@linux.se>
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <t.zachmann@zagge.de>

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

#include "KoLineEnd.h" 

#include <QPixmap>
#include <QIcon>
#include <QSvgRenderer>
#include <QPainter>

//qRegisterMetaType<KoLineEnd>("KoLineEnd");

KoLineEnd::KoLineEnd(QString name, QString path, QString view)
{
  m_name = name;
  m_path = path;
  m_view = view;
}

KoLineEnd::KoLineEnd()
{
}

KoLineEnd::KoLineEnd(const KoLineEnd &lineEnd)
{
    m_name = lineEnd.m_name;
    m_path = lineEnd.m_path;
    m_view = lineEnd.m_view;
    m_svg = lineEnd.m_svg;
}

KoLineEnd::~KoLineEnd()
{
}

QByteArray KoLineEnd::generateSVG(QSize size, QString comment)
{
    QByteArray str("<?xml version=\"1.0\" standalone=\"no\"?>\
    <!--");
    str.append(comment.toUtf8());
    str.append("-->\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\
    <svg width=\"");
    str.append(QString::number(size.width()).toUtf8());
    str.append("px\" height=\"");
    str.append(QString::number(size.height()).toUtf8());
    str.append("px\" viewBox=\"");
    str.append(m_view.toUtf8());
    str.append("\" preserveAspectRatio=\"none\" xmlns=\"http://www.w3.org/2000/svg\">\
    <path fill=\"black\"  d=\"");
    str.append(m_path.toUtf8());
    str.append("\" />\
    </svg>");
    m_svg = str;
    return str;
}

QByteArray KoLineEnd::getSVG(){
    return m_svg;
}

QIcon KoLineEnd::drawIcon(QSize size, int proportion)
{
    QSvgRenderer endLineRenderer;
    QPixmap endLinePixmap(size);
    endLinePixmap.fill(QColor(Qt::transparent));
    QPainter endLinePainter(&endLinePixmap);

    // Convert path to SVG
    endLineRenderer.load(generateSVG(QSize(size.width()-6, size.height()-6)));
    endLineRenderer.render(&endLinePainter, QRectF(proportion, proportion, size.width()-(proportion*2), size.height()-(proportion*2)));

    // return QIcon
    return QIcon (endLinePixmap);
}

QString KoLineEnd::name()
{
  return m_name;
}

QString KoLineEnd::path()
{
  return m_path;
}

QString KoLineEnd::viewBox()
{
  return m_view;
}

#include "KoLineEnd.moc"
