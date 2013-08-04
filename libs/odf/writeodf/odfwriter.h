#ifndef ODFWRITER_H
#define ODFWRITER_H

#include <KoXmlWriter.h>
#include <QtCore/QUrl>
#include <QtCore/QDate>
#include <QtCore/QStringList>

class OdfWriter {
private:
    OdfWriter* child;
    OdfWriter* parent;
    void operator=(const OdfWriter&);
protected:
    mutable KoXmlWriter* xml;
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
    // ideally, the copy constructor would never be called
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
    void addAttribute(const char* name, const QStringList& /*value*/) {
        Q_ASSERT(!child);
        xml->addAttribute(name, "");
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
};
#endif
