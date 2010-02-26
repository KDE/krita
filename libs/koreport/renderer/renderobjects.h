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
#ifndef __RENDEROBJECTS_H__
#define __RENDEROBJECTS_H__

#include <QString>
#include <QList>
#include <QPointF>
#include <QSizeF>
#include <QFont>
#include <QImage>
#include <QPen>
#include <QBrush>
#include <krobjectdata.h>
#include <reportpageoptions.h>
#include <QPicture>
#include <krreportdata.h>

class ORODocument;
class OROPage;
class OROPrimitive;
class OROTextBox;
class OROLine;
class OROImage;
class OROSection;

//
// ORODocument
// This object is a single document containing one or more OROPage elements
//
class ORODocument
{
    friend class OROPage;
    friend class OROSection;
public:
    ORODocument(const QString & = QString());
    virtual ~ORODocument();

    QString title() const {
        return m_title;
    };
    void setTitle(const QString &);

    int pages() const {
        return m_pages.count();
    };
    OROPage* page(int);
    void addPage(OROPage*);

    int sections() const {
        return m_sections.count();
    };
    OROSection* section(int);
    void addSection(OROSection*);

    void setPageOptions(const ReportPageOptions &);
    ReportPageOptions pageOptions() const {
        return m_pageOptions;
    };

protected:
    QString m_title;
    QList<OROPage*> m_pages;
    QList<OROSection*> m_sections;
    ReportPageOptions m_pageOptions;
};

//
// OROPage
// This object is a single page in a document and may contain zero or more
// OROPrimitive objects all of which represent some form of mark to made on
// a page.
//
class OROPage
{
    friend class ORODocument;
    friend class OROPrimitive;

public:
    OROPage(ORODocument * = 0);
    virtual ~OROPage();

    ORODocument* document() const {
        return m_document;
    };
    int page() const; // returns this pages current page number

    int primitives() const {
        return m_primitives.count();
    };
    OROPrimitive* primitive(int);
    void addPrimitive(OROPrimitive*, bool = false);

protected:
    ORODocument * m_document;
    QList<OROPrimitive*> m_primitives;
};
//
// OROSection
// This object is a single row in a document and may contain zero or more
// OROPrimitives
//
class OROSection
{
    friend class ORODocument;
    friend class OROPrimitive;

public:
    enum Sort {
        SortX = 1,
        SortY,
        SortZ
    };

    OROSection(ORODocument * = 0);
    virtual ~OROSection();

    void setHeight(int);
    int height();

    void setBackgroundColor(const QColor&L);
    QColor backgroundColor();

    ORODocument* document() const {
        return m_document;
    };
    long row() const; // returns this pages current page number

    void setType(KRSectionData::Section t) {
        m_type = t;
    }
    KRSectionData::Section type() {
        return m_type;
    }

    int primitives() const {
        return m_primitives.count();
    };
    OROPrimitive* primitive(int);
    void addPrimitive(OROPrimitive*);

    void sortPrimatives(Sort);
protected:
    ORODocument * m_document;
    QList<OROPrimitive*> m_primitives;
    long m_row;
    int m_height;
    KRSectionData::Section m_type;
    QColor m_backgroundColor;

private:
    static bool xLessThan(OROPrimitive* s1, OROPrimitive* s2);
};


//
// OROPrimitive
// This object represents the basic primitive with a position and type.
// Other primitives are subclasses with a defined type and any additional
// information they require to define that primitive.
//
class OROPrimitive
{
    friend class OROPage;

public:
    virtual ~OROPrimitive();

    // Returns the type of the primitive which should be
    // set by the subclass
    int type() const {
        return m_type;
    };
    OROPage * page() const {
        return m_page;
    };

    QPointF position() const {
        return m_position;
    };
    void setPosition(const QPointF &);
    virtual OROPrimitive* clone() = 0;
protected:
    OROPrimitive(int);

    OROPage * m_page;
    int m_type;
    QPointF m_position;
    QSizeF m_size;
};

//
// OROTextBox
// This is a text box primitive it defines a box region and text that will
// be rendered inside that region. It also contains information for font
// and positioning of the text.
//
class OROTextBox : public OROPrimitive
{
public:
    OROTextBox();
    virtual ~OROTextBox();

    QSizeF size() const {
        return m_size;
    };
    void setSize(const QSizeF &);

    QString text() const {
        return m_text;
    };
    void setText(const QString &);

    KRTextStyleData textStyle() const {
        return m_textStyle;
    }
    void setTextStyle(const KRTextStyleData&);

    KRLineStyleData lineStyle() const {
        return m_lineStyle;
    }
    void setLineStyle(const KRLineStyleData&);

    void setFont(const QFont &);

    int flags() const {
        return m_flags;
    };
    void setFlags(int);

    static const int TextBox;

    virtual OROPrimitive* clone();

protected:
    QSizeF m_size;
    QString m_text;
    KRTextStyleData m_textStyle;
    KRLineStyleData m_lineStyle;
    Qt::Alignment m_alignment;
    int m_flags; // Qt::AlignmentFlag and Qt::TextFlag OR'd
};

//
// OROLine
// This primitive defines a line with a width/weight.
//
class OROLine : public OROPrimitive
{
public:
    OROLine();
    virtual ~OROLine();

    QPointF startPoint() const {
        return position();
    };
    void setStartPoint(const QPointF &);

    QPointF endPoint() const {
        return m_endPoint;
    };
    void setEndPoint(const QPointF &);

    KRLineStyleData lineStyle() const {
        return m_lineStyle;
    };
    void setLineStyle(const KRLineStyleData&);

    static const int Line;
    virtual OROPrimitive* clone();
protected:
    QPointF m_endPoint;
    KRLineStyleData m_lineStyle;
};

//
// OROImage
// This primitive defines an image
//
class OROImage: public OROPrimitive
{
public:
    OROImage();
    virtual ~OROImage();

    QImage image() const {
        return m_image;
    };
    void setImage(const QImage &);

    QSizeF size() const {
        return m_size;
    };
    void setSize(const QSizeF &);

    bool scaled() const {
        return m_scaled;
    };
    void setScaled(bool);

    int transformationMode() const {
        return m_transformFlags;
    };
    void setTransformationMode(int);

    int aspectRatioMode() const {
        return m_aspectFlags;
    };
    void setAspectRatioMode(int);

    static const int Image;
    virtual OROPrimitive* clone();

protected:
    QImage m_image;
    QSizeF m_size;
    bool m_scaled;
    int m_transformFlags;
    int m_aspectFlags;
};

class OROPicture: public OROPrimitive
{
public:
    OROPicture();
    virtual ~OROPicture();

    void setPicture(const QPicture& p) {
        m_picture = p;
    }
    QPicture* picture() {
        return &m_picture;
    };

    QSizeF size() const {
        return m_size;
    };
    void setSize(const QSizeF &);

    static const int Picture;
    virtual OROPrimitive* clone();
protected:
    QPicture m_picture;
    QSizeF m_size;

};
//
// ORORect
// This primitive defines a drawn rectangle
//
class ORORect: public OROPrimitive
{
public:
    ORORect();
    virtual ~ORORect();

    QSizeF size() const {
        return m_size;
    }
    void setSize(const QSizeF &);

    QRectF rect() const {
        return QRectF(m_position, m_size);
    };
    void setRect(const QRectF &);

    QPen pen() const {
        return m_pen;
    };
    void setPen(const QPen &);

    QBrush brush() const {
        return m_brush;
    };
    void setBrush(const QBrush &);

    static const int Rect;
    virtual OROPrimitive* clone();
protected:
    QSizeF m_size;
    QPen m_pen;
    QBrush m_brush;
};

//
// ORORect
// This primitive defines a drawn rectangle
//
class OROEllipse: public OROPrimitive
{
public:
    OROEllipse();
    virtual ~OROEllipse();

    QSizeF size() const {
        return m_size;
    }
    void setSize(const QSizeF &);

    QRectF rect() const {
        return QRectF(m_position, m_size);
    };
    void setRect(const QRectF &);

    QPen pen() const {
        return m_pen;
    };
    void setPen(const QPen &);

    QBrush brush() const {
        return m_brush;
    };
    void setBrush(const QBrush &);

    static const int Ellipse;
    virtual OROPrimitive* clone();

protected:
    QSizeF m_size;
    QPen m_pen;
    QBrush m_brush;
};

class OROCheck : public OROPrimitive
{
public:
    OROCheck();
    virtual ~OROCheck();
    virtual OROPrimitive* clone();
    static const int Check;

    void setCheckType(const QString& t) {
        if (t == "Cross" || t == "Tick" || t == "Dot") {
            m_checkType = t;
        } else {
            m_checkType = "Cross";
        }
    }

    QString checkType() {
        return m_checkType;
    };

    QSizeF size() const {
        return m_size;
    }
    void setSize(const QSizeF &s) {
        m_size = s;
    }

    void setValue(bool v) {
        m_value = v;
    }
    bool value() {
        return m_value;
    }

    void setLineStyle(const KRLineStyleData& ls) {
        m_lineStyle = ls;
    }

    KRLineStyleData lineStyle() {
        return m_lineStyle;
    }
    void setForegroundColor(const QColor& fg) {
        m_fgColor = fg;
    }
    QColor foregroundColor() {
        return m_fgColor;
    }

protected:
    QSizeF m_size;
    QString m_checkType;
    bool m_value;
    KRLineStyleData m_lineStyle;
    QColor m_fgColor;


};

#endif // __RENDEROBJECTS_H__
