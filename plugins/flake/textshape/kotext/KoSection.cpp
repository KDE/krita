/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoSection.h"

#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoTextSharedLoadingData.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoSectionStyle.h>
#include <KoSectionModel.h>
#include <KoSectionEnd.h>
#include <KoTextDocument.h>
#include <KoTextInlineRdf.h>

#include <QTextBlock>

#include "TextDebug.h"

class KoSectionPrivate
{
public:
    explicit KoSectionPrivate(const QTextCursor &cursor, const QString &_name, KoSection *_parent)
        : document(cursor.block().document())
        , name(_name)
        , sectionStyle(0)
        , boundingCursorStart(cursor)
        , boundingCursorEnd(cursor)
        , parent(_parent)
        , inlineRdf(0)
    {
    }

    const QTextDocument *document;

    QString condition;
    QString display;
    QString name;
    QString text_protected;
    QString protection_key;
    QString protection_key_digest_algorithm;
    QString style_name;
    KoSectionStyle *sectionStyle;

    QScopedPointer<KoSectionEnd> sectionEnd; ///< pointer to the corresponding section end
    int level; ///< level of the section in document, root sections have 0 level
    QTextCursor boundingCursorStart; ///< This cursor points to the start of the section
    QTextCursor boundingCursorEnd; ///< This cursor points to the end of the section (excluding paragraph symbol)

    // Boundings explanation:
    //
    // |S|e|c|t|i|o|n|...|t|e|x|t|P|
    // ^                         ^
    // |--- Start                |-- End

    QVector<KoSection *> children; ///< List of the section's childrens
    KoSection *parent; ///< Parent of the section

    KoTextInlineRdf *inlineRdf; ///< Handling associated RDF
};

KoSection::KoSection(const QTextCursor &cursor, const QString &name, KoSection *parent)
    : d_ptr(new KoSectionPrivate(cursor, name, parent))
{
    Q_D(KoSection);

    d->boundingCursorStart.setKeepPositionOnInsert(true); // Start cursor should stay on place
    d->boundingCursorEnd.setKeepPositionOnInsert(false); // and end one should move forward

    if (parent) {
        d->level = parent->level() + 1;
    } else {
        d->level = 0;
    }
}

KoSection::~KoSection()
{
    // Here scoped pointer will delete sectionEnd
}

QString KoSection::name() const
{
    Q_D(const KoSection);
    return d->name;
}

QPair<int, int> KoSection::bounds() const
{
    Q_D(const KoSection);
    return QPair<int, int>(
        d->boundingCursorStart.position(),
        d->boundingCursorEnd.position()
    );
}

int KoSection::level() const
{
    Q_D(const KoSection);
    return d->level;
}

bool KoSection::loadOdf(const KoXmlElement &element, KoTextSharedLoadingData *sharedData, bool stylesDotXml)
{
    Q_D(KoSection);
    // check whether we really are a section
    if (element.namespaceURI() == KoXmlNS::text && element.localName() == "section") {
        // get all the attributes
        d->condition = element.attributeNS(KoXmlNS::text, "condition");
        d->display = element.attributeNS(KoXmlNS::text, "display");

        if (d->display == "condition" && d->condition.isEmpty()) {
            warnText << "Section display is set to \"condition\", but condition is empty.";
        }

        QString newName = element.attributeNS(KoXmlNS::text, "name");
        if (!KoTextDocument(d->document).sectionModel()->setName(this, newName)) {
            warnText << "Section name \"" << newName
                << "\" must be unique or is invalid. Resetting it to " << name();
        }

        d->text_protected = element.attributeNS(KoXmlNS::text, "text-protected");
        d->protection_key = element.attributeNS(KoXmlNS::text, "protection-key");
        d->protection_key_digest_algorithm = element.attributeNS(KoXmlNS::text, "protection-key-algorithm");
        d->style_name = element.attributeNS(KoXmlNS::text, "style-name", "");

        if (!d->style_name.isEmpty()) {
            d->sectionStyle = sharedData->sectionStyle(d->style_name, stylesDotXml);
        }

        // lets handle associated xml:id
        if (element.hasAttribute("id")) {
            KoTextInlineRdf* inlineRdf = new KoTextInlineRdf(const_cast<QTextDocument *>(d->document), this);
            if (inlineRdf->loadOdf(element)) {
                d->inlineRdf = inlineRdf;
            } else {
                delete inlineRdf;
                inlineRdf = 0;
            }
        }

        return true;
    }
    return false;
}

void KoSection::saveOdf(KoShapeSavingContext &context) const
{
    Q_D(const KoSection);
    KoXmlWriter *writer = &context.xmlWriter();
    Q_ASSERT(writer);
    writer->startElement("text:section", false);

    if (!d->condition.isEmpty()) writer->addAttribute("text:condition", d->condition);
    if (!d->display.isEmpty()) writer->addAttribute("text:display", d->condition);
    if (!d->name.isEmpty()) writer->addAttribute("text:name", d->name);
    if (!d->text_protected.isEmpty()) writer->addAttribute("text:text-protected", d->text_protected);
    if (!d->protection_key.isEmpty()) writer->addAttribute("text:protection-key", d->protection_key);
    if (!d->protection_key_digest_algorithm.isEmpty()) {
        writer->addAttribute("text:protection-key-digest-algorithm", d->protection_key_digest_algorithm);
    }
    if (!d->style_name.isEmpty()) writer->addAttribute("text:style-name", d->style_name);

    if (d->inlineRdf) {
        d->inlineRdf->saveOdf(context, writer);
    }
}

void KoSection::setSectionEnd(KoSectionEnd* sectionEnd)
{
    Q_D(KoSection);
    d->sectionEnd.reset(sectionEnd);
}

void KoSection::setName(const QString &name)
{
    Q_D(KoSection);
    d->name = name;
}

void KoSection::setLevel(int level)
{
    Q_D(KoSection);
    d->level = level;
}

void KoSection::setKeepEndBound(bool state)
{
    Q_D(KoSection);
    d->boundingCursorEnd.setKeepPositionOnInsert(state);
}

KoSection *KoSection::parent() const
{
    Q_D(const KoSection);
    return d->parent;
}

QVector<KoSection *> KoSection::children() const
{
    Q_D(const KoSection);
    return d->children;
}

void KoSection::insertChild(int childIdx, KoSection *section)
{
    Q_D(KoSection);
    d->children.insert(childIdx, section);
}

void KoSection::removeChild(int childIdx)
{
    Q_D(KoSection);
    d->children.remove(childIdx);
}

KoTextInlineRdf *KoSection::inlineRdf() const
{
    Q_D(const KoSection);
    return d->inlineRdf;
}

void KoSection::setInlineRdf(KoTextInlineRdf *inlineRdf)
{
    Q_D(KoSection);
    d->inlineRdf = inlineRdf;
}
