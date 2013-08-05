/* This file is part of the KDE project
   Copyright (C) 2013 Jos van den Oever <jos@vandenoever.info>

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
   Boston, MA 02110-1301, USA.
*/
#ifndef ODFWRITER_H
#define ODFWRITER_H

#include <KoXmlWriter.h>
#include <QUrl>
#include <QDate>
#include <QStringList>

class Duration {
private:
    QTime m_time;
public:
    explicit Duration(const QTime& t) :m_time(t) {}
    QString toString() const {
        return m_time.toString("'PT'hh'H'mm'M'ss'S'");
    }
};

class OdfWriter {
private:
    void operator=(const OdfWriter&);
protected:
    OdfWriter(KoXmlWriter* xml_, const char* tag, bool indent) :child(0), parent(0), xml(xml_) {
        xml->startElement(tag, indent);
    }
    OdfWriter(OdfWriter* p, const char* tag, bool indent) :child(0), parent(p), xml(parent->xml) {
        if (parent->child) {
            parent->child->end();
        }
        parent->child = this;
        xml->startElement(tag, indent);
    }
    ~OdfWriter() {
        end();
    }
    void endChild() {
        if (child) {
            child->parent = 0;
            child->end();
            child = 0;
        }
    }
    // in c++11, we would use a move constructor instead of a copy constructor
    OdfWriter(const OdfWriter&o) :child(o.child), parent(o.parent), xml(o.xml) {
        // disable o and make the parent refer to this new copy
        o.xml = 0;
        if (parent && parent->child == &o) {
            parent->child = this;
        }
    }
public:
    void end() {
        if (xml) {
            endChild();
            xml->endElement();
            if (parent) {
                parent->child = 0;
            }
            xml = 0;
        }
    }
    void addTextNode(const QString& str) {
        endChild();
        xml->addTextNode(str);
    }
    void addAttribute(const char* name, const char* value) {
        Q_ASSERT(!child);
        xml->addAttribute(name, value);
    }
    void addAttribute(const char* name, const QString& value) {
        Q_ASSERT(!child);
        xml->addAttribute(name, value);
    }
    void addAttribute(const char* name, quint64 value) {
        Q_ASSERT(!child);
        xml->addAttribute(name, QString::number(value));
    }
    void addAttribute(const char* name, const QUrl& value) {
        Q_ASSERT(!child);
        xml->addAttribute(name, value.toString());
    }
    void addAttribute(const char* name, const QDate& value) {
        Q_ASSERT(!child);
        xml->addAttribute(name, value.toString(Qt::ISODate));
    }
    void addAttribute(const char* name, const QTime& value) {
        Q_ASSERT(!child);
        xml->addAttribute(name, value.toString(Qt::ISODate));
    }
    void addAttribute(const char* name, const QDateTime& value) {
        Q_ASSERT(!child);
        xml->addAttribute(name, value.toString(Qt::ISODate));
    }
    void addAttribute(const char* name, const QStringList& value) {
        Q_ASSERT(!child);
        xml->addAttribute(name, value.join(QChar(' ')));
    }
    void addAttribute(const char* name, const Duration& value) {
        Q_ASSERT(!child);
        xml->addAttribute(name, value.toString());
    }
    void addProcessingInstruction(const char* cstr) {
        endChild();
        xml->addProcessingInstruction(cstr);
    }
    template <class T>
    void addCompleteElement(T cstr) {
        endChild();
        xml->addCompleteElement(cstr);
    }
private:
    OdfWriter* child;
    OdfWriter* parent;
protected:
    mutable KoXmlWriter* xml;
};
#endif
