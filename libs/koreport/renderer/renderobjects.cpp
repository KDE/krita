/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "renderobjects.h"


//
// ORODocument
//
ORODocument::ORODocument(const QString & pTitle)
        : m_title(pTitle)
{
}

ORODocument::~ORODocument()
{
    while (!m_pages.isEmpty()) {
        OROPage * p = m_pages.takeFirst();
        p->m_document = 0;
        delete p;
    }
}

void ORODocument::setTitle(const QString & pTitle)
{
    m_title = pTitle;
}

OROPage* ORODocument::page(int pnum)
{
    return m_pages.at(pnum);
}

void ORODocument::addPage(OROPage* p)
{
    if (p == 0)
        return;

    // check that this page is not already in another document

    p->m_document = this;
    m_pages.append(p);
}

OROSection* ORODocument::section(int pnum)
{
    return m_sections.at(pnum);
}

void ORODocument::addSection(OROSection* s)
{
    if (s == 0)
        return;

    // check that this page is not already in another document

    s->m_document = this;
    m_sections.append(s);
}

void ORODocument::setPageOptions(const ReportPageOptions & options)
{
    m_pageOptions = options;
}

//
// OROPage
//
OROPage::OROPage(ORODocument * pDocument)
        : m_document(pDocument)
{

}

OROPage::~OROPage()
{
    if (m_document) {
        m_document->m_pages.removeAt(page());
        m_document = 0;
    }

    while (!m_primitives.isEmpty()) {
        OROPrimitive* p = m_primitives.takeFirst();
        p->m_page = 0;
        delete p;
    }
}

int OROPage::page() const
{
    if (m_document) {
        for (int i = 0; i < m_document->m_pages.size(); i++) {
            if (m_document->m_pages.at(i) == this)
                return i;
        }
    }
    return -1;
}

OROPrimitive* OROPage::primitive(int idx)
{
    return m_primitives.at(idx);
}

void OROPage::addPrimitive(OROPrimitive* p, bool atBeginning)
{
    if (p == 0)
        return;

    // check that this primitve is not already in another page

    p->m_page = this;
    if (atBeginning) {
        m_primitives.prepend(p);
    } else {
        m_primitives.append(p);
    }
}

//
// OROSection
//
OROSection::OROSection(ORODocument * pDocument)
        : m_document(pDocument)
{
    m_height = 0;
    m_backgroundColor = Qt::white;
}

OROSection::~OROSection()
{
    if (m_document) {
        m_document->m_sections.removeAt(row());
        m_document = 0;
    }

    while (!m_primitives.isEmpty()) {
        OROPrimitive* p = m_primitives.takeFirst();
        delete p;
    }
}

long OROSection::row() const
{
    return m_row;
}

OROPrimitive* OROSection::primitive(int idx)
{
    return m_primitives.at(idx);
}

void OROSection::addPrimitive(OROPrimitive* p)
{
    if (p == 0)
        return;

    m_primitives.append(p);
}

void OROSection::setHeight(int h)
{
    m_height = h;
}

int OROSection::height()
{
    return m_height;
}

void OROSection::setBackgroundColor(const QColor &c)
{
    m_backgroundColor = c;
}

QColor OROSection::backgroundColor()
{
    return m_backgroundColor;
}

void OROSection::sortPrimatives(Sort s)
{
    if (s == SortX) {
        qSort(m_primitives.begin(), m_primitives.end(), xLessThan);
    }
}

bool OROSection::xLessThan(OROPrimitive* s1, OROPrimitive* s2)
{
    return s1->position().x() < s2->position().x();
}

//
// OROPrimitive
//
OROPrimitive::OROPrimitive(int pType)
        : m_type(pType)
{
    m_page = 0;
}

OROPrimitive::~OROPrimitive()
{
    if (m_page) {
        m_page->m_primitives.removeAt(m_page->m_primitives.indexOf(this));
        m_page = 0;
    }
}

void OROPrimitive::setPosition(const QPointF & p)
{
    m_position = p;
}

//
// OROTextBox
//
const int OROTextBox::TextBox = 1;
OROTextBox::OROTextBox()
        : OROPrimitive(OROTextBox::TextBox)
{
    m_flags = 0;

    m_lineStyle.lineColor = Qt::black;
    m_lineStyle.weight = 0;
    m_lineStyle.style = Qt::NoPen;
}

OROTextBox::~OROTextBox()
{
}

void OROTextBox::setSize(const QSizeF & s)
{
    m_size = s;
}

void OROTextBox::setText(const QString & s)
{
    m_text = s;
}

void OROTextBox::setTextStyle(const KRTextStyleData & ts)
{
    m_textStyle = ts;
}

void OROTextBox::setLineStyle(const KRLineStyleData & ls)
{
    m_lineStyle = ls;
}

void OROTextBox::setFont(const QFont & f)
{
    m_textStyle.font = f;
}

void OROTextBox::setFlags(int f)
{
    m_flags = f;
}

OROPrimitive* OROTextBox::clone()
{
    OROTextBox *theClone = new OROTextBox();
    theClone->setSize(m_size);
    theClone->setPosition(m_position);
    theClone->setText(m_text);
    theClone->setTextStyle(m_textStyle);
    theClone->setLineStyle(m_lineStyle);
    theClone->setFlags(m_alignment);
    return theClone;
}


//
// OROLine
//
const int OROLine::Line = 2;

OROLine::OROLine()
        : OROPrimitive(OROLine::Line)
{

}

OROLine::~OROLine()
{
}

void OROLine::setStartPoint(const QPointF & p)
{
    setPosition(p);
}

void OROLine::setEndPoint(const QPointF & p)
{
    m_endPoint = p;
}

void OROLine::setLineStyle(const KRLineStyleData& ls)
{
    m_lineStyle = ls;
}


OROPrimitive* OROLine::clone()
{
    OROLine *theClone = new OROLine();
    theClone->setStartPoint(m_position);
    theClone->setEndPoint(m_endPoint);
    theClone->setLineStyle(m_lineStyle);
    return theClone;
}

//
// OROImage
//
const int OROImage::Image = 3;

OROImage::OROImage()
        : OROPrimitive(OROImage::Image)
{
    m_scaled = false;
    m_transformFlags = Qt::FastTransformation;
    m_aspectFlags = Qt::IgnoreAspectRatio;
}

OROImage::~OROImage()
{
}

void OROImage::setImage(const QImage & img)
{
    m_image = img;
}

void OROImage::setSize(const QSizeF & sz)
{
    m_size = sz;
}

void OROImage::setScaled(bool b)
{
    m_scaled = b;
}

void OROImage::setTransformationMode(int tm)
{
    m_transformFlags = tm;
}

void OROImage::setAspectRatioMode(int arm)
{
    m_aspectFlags = arm;
}

OROPrimitive* OROImage::clone()
{
    OROImage *theClone = new OROImage();
    theClone->setSize(m_size);
    theClone->setPosition(m_position);
    theClone->setImage(m_image);
    theClone->setScaled(m_scaled);
    theClone->setTransformationMode(m_transformFlags);
    theClone->setAspectRatioMode(m_aspectFlags);
    return theClone;
}

//
// OROPicture
//
const int OROPicture::Picture = 6;

OROPicture::OROPicture()
        : OROPrimitive(OROPicture::Picture)
{

}

OROPicture::~OROPicture()
{
}

void OROPicture::setSize(const QSizeF & sz)
{
    m_size = sz;
}

OROPrimitive* OROPicture::clone()
{
    OROPicture *theClone = new OROPicture();
    theClone->setSize(m_size);
    theClone->setPosition(m_position);
    theClone->setPicture(m_picture);
    return theClone;
}

//
// ORORect
//
const int ORORect::Rect = 4;

ORORect::ORORect()
        : OROPrimitive(ORORect::Rect)
{
}

ORORect::~ORORect()
{
}

void ORORect::setSize(const QSizeF & s)
{
    m_size = s;
}

void ORORect::setRect(const QRectF & r)
{
    setPosition(r.topLeft());
    setSize(r.size());
}

void ORORect::setPen(const QPen & p)
{
    m_pen = p;
}

void ORORect::setBrush(const QBrush & b)
{
    m_brush = b;
}

OROPrimitive* ORORect::clone()
{
    ORORect *theClone = new ORORect();
    theClone->setSize(m_size);
    theClone->setPosition(m_position);
    theClone->setPen(m_pen);
    theClone->setBrush(m_brush);
    return theClone;
}
//
// OROEllipse
//
const int OROEllipse::Ellipse = 5;

OROEllipse::OROEllipse()
        : OROPrimitive(OROEllipse::Ellipse)
{
}

OROEllipse::~OROEllipse()
{
}

void OROEllipse::setSize(const QSizeF & s)
{
    m_size = s;
}

void OROEllipse::setRect(const QRectF & r)
{
    setPosition(r.topLeft());
    setSize(r.size());
}

void OROEllipse::setPen(const QPen & p)
{
    m_pen = p;
}

void OROEllipse::setBrush(const QBrush & b)
{
    m_brush = b;
}

OROPrimitive* OROEllipse::clone()
{
    OROEllipse *theClone = new OROEllipse();
    theClone->setSize(m_size);
    theClone->setPosition(m_position);
    theClone->setPen(m_pen);
    theClone->setBrush(m_brush);
    return theClone;
}

const int OROCheck::Check = 7;

OROCheck::OROCheck()
        : OROPrimitive(OROCheck::Check)
{

}

OROCheck::~OROCheck()
{

}

OROPrimitive* OROCheck::clone()
{
    OROCheck *theClone = new OROCheck();
    theClone->setSize(m_size);
    theClone->setPosition(m_position);
    theClone->setLineStyle(m_lineStyle);
    theClone->setForegroundColor(m_fgColor);
    theClone->setValue(m_value);
    return theClone;
}
