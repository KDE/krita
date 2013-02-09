/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifndef KOTEXTWRITER_P_H
#define KOTEXTWRITER_P_H

#include <QMap>
#include <QTextDocument>
#include <QTextTable>
#include <QTextCursor>
#include <QStack>
#include <QTextTableCellFormat>
#include <QBuffer>
#include <QUuid>
#include <QXmlStreamReader>

#include "KoInlineObject.h"
#include "KoAnchorInlineObject.h"
#include "KoShape.h"
#include "KoVariable.h"
#include "KoInlineTextObjectManager.h"
#include "styles/KoStyleManager.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"
#include "styles/KoTableCellStyle.h"
#include "styles/KoTableStyle.h"
#include "KoTextBlockData.h"
#include "KoTextDocument.h"
#include "KoTextInlineRdf.h"
#include "KoSection.h"

#include "KoTextMeta.h"
#include "KoBookmark.h"

#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <KoTableColumnAndRowStyleManager.h>
#include <KoTableColumnStyle.h>

#include <opendocument/KoTextSharedSavingData.h>

#include <KoXmlWriter.h>
#include <KoTableOfContentsGeneratorInfo.h>
#include <KoBibliographyInfo.h>
#include <KoTextWriter.h>
#include <KoTableRowStyle.h>

#ifdef SHOULD_BUILD_RDF
#include <Soprano/Soprano>
#endif

#include "KoDocumentRdfBase.h"

/**
 * XXX: Apidox!
 */
class TagInformation
{
    public:
        TagInformation():tagName(0), attributeList()
        {
        }

        void setTagName(const char *tagName)
        {
            this->tagName = tagName;
        }

        void addAttribute(const QString& attributeName, const QString& attributeValue)
        {
            attributeList.push_back(QPair<QString,QString>(attributeName, attributeValue));
        }

        void addAttribute(const QString& attributeName, int value)
        {
            addAttribute(attributeName, QString::number(value));
        }

        void clear()
        {
            tagName = NULL;
            attributeList.clear();
        }

        const char *name() const
        {
            return tagName;
        }

        const QVector<QPair<QString, QString> >& attributes() const
        {
            return attributeList;
        }

    private:
        const char *tagName;
        QVector<QPair<QString, QString> > attributeList;
};


/**
 * XXX: Apidox!
 */
class KoTextWriter::Private
{
public:

    explicit Private(KoShapeSavingContext &context);

    ~Private() {}

    void writeBlocks(QTextDocument *document, int from, int to,
                     QHash<QTextList *, QString> &listStyles,
                     QTextTable *currentTable = 0,
                     QTextList *currentList = 0);
    QHash<QTextList *, QString> saveListStyles(QTextBlock block, int to);

private:

    enum ElementType {
        Span,
        ParagraphOrHeader,
        ListItem,
        List,
        NumberedParagraph,
        Table,
        TableRow,
        TableColumn,
        TableCell
    };

    void openTagRegion(KoTextWriter::Private::ElementType elementType, TagInformation &tagInformation);
    void closeTagRegion();

    QString saveParagraphStyle(const QTextBlock &block);
    QString saveParagraphStyle(const QTextBlockFormat &blockFormat, const QTextCharFormat &charFormat);
    QString saveCharacterStyle(const QTextCharFormat &charFormat, const QTextCharFormat &blockCharFormat);
    QString saveTableStyle(const QTextTable &table);
    QString saveTableColumnStyle(const KoTableColumnStyle &columnStyle, int columnNumber, const QString &tableStyleName);
    QString saveTableRowStyle(const KoTableRowStyle &rowStyle, int rowNumber, const QString &tableStyleName);
    QString saveTableCellStyle(const QTextTableCellFormat &cellFormat, int columnNumber, const QString &tableStyleName);

    void saveParagraph(const QTextBlock &block, int from, int to);
    void saveTable(QTextTable *table, QHash<QTextList *, QString> &listStyles);
    QTextBlock& saveList(QTextBlock &block, QHash<QTextList *, QString> &listStyles, int level, QTextTable *currentTable);
    void saveTableOfContents(QTextDocument *document, QHash<QTextList *, QString> &listStyles, QTextBlock toc);
    void saveBibliography(QTextDocument *document, QHash<QTextList *, QString> &listStyles, QTextBlock bib);
    void saveInlineRdf(KoTextInlineRdf *rdf, TagInformation *tagInfos);

    void addNameSpaceDefinitions(QString &generatedXmlString);

    // Common methods
    void writeAttributes(QTextStream &outputXmlStream, KoXmlElement &element);
    void writeNode(QTextStream &outputXmlStream, KoXmlNode &node, bool writeOnlyChildren = false);

    QString createXmlId();

public:

    KoDocumentRdfBase *rdfData;
    KoTextSharedSavingData *sharedData;
    KoStyleManager *styleManager;
    QTextDocument *document;
    int globalFrom; // to and from positions, relevant for finding matching bookmarks etc
    int globalTo;

private:

    KoXmlWriter *writer;

    QStack<const char *> openedTagStack;

    KoShapeSavingContext &context;

    // Things like bookmarks need to be properly turn down during a cut and paste operation
    // when their end markeris not included in the selection. However, when recursing into
    // e.g. the QTextDocument of a table, we need have a clean slate. Hence, a stack of stacks.
    QStack< QStack<KoInlineObject*> *> pairedInlineObjectsStackStack;
    QStack<KoInlineObject*> *currentPairedInlineObjectsStack;

    QMap<KoList *, QString> listXmlIds;

    QMap<KoList *, QString> numberedParagraphListIds;

    int splitEndBlockNumber;
};

#endif // KOTEXTWRITER_P_H
