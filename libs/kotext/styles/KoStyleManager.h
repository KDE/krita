/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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

#ifndef KOSTYLEMANAGER_H
#define KOSTYLEMANAGER_H

#include "kotext_export.h"

#include <QObject>
#include <QMetaType>

class QTextDocument;
class KoCharacterStyle;
class KoParagraphStyle;
class KoListStyle;
class KoTableStyle;
class KoTableColumnStyle;
class KoTableRowStyle;
class KoTableCellStyle;
class KoSectionStyle;
class KoXmlWriter;
class ChangeFollower;
class KoGenStyles;
class KoTextShapeData;

/**
 * Manages all character, paragraph, table and table cell styles for any number
 * of documents.
 */
class KOTEXT_EXPORT KoStyleManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a new style manager.
     * @param parent pass a parent to use qobject memory management
     */
    explicit KoStyleManager(QObject *parent = 0);

    /**
     * Destructor.
     */
    virtual ~KoStyleManager();

    // load is not needed as it is done in KoTextSharedLoadingData

    /**
     * Save document styles
     */
    void saveOdf(KoGenStyles& mainStyles);

    /**
     * Save the default-style styles
     */
    void saveOdfDefaultStyles(KoGenStyles &mainStyles);

    /**
     * Add a new style, automatically giving it a new styleId.
     */
    void add(KoCharacterStyle *style);
    /**
     * Add a new style, automatically giving it a new styleId.
     */
    void add(KoParagraphStyle *style);
    /**
     * Add a new list style, automatically giving it a new styleId.
     */
    void add(KoListStyle *style);
    /**
     * Add a new table style, automatically giving it a new styleId.
     */
    void add(KoTableStyle *style);
    /**
     * Add a new table column style, automatically giving it a new styleId.
     */
    void add(KoTableColumnStyle *style);
    /**
     * Add a new table row style, automatically giving it a new styleId.
     */
    void add(KoTableRowStyle *style);
    /**
     * Add a new table cell style, automatically giving it a new styleId.
     */
    void add(KoTableCellStyle *style);
    /**
     * Add a new sewction style, automatically giving it a new styleId.
     */
    void add(KoSectionStyle *style);

    /**
     * Remove a style.
     */
    void remove(KoCharacterStyle *style);
    /**
     * Remove a style.
     */
    void remove(KoParagraphStyle *style);
    /**
     * Remove a list style.
     */
    void remove(KoListStyle *style);
    /**
     * Remove a table style.
     */
    void remove(KoTableStyle *style);
    /**
     * Remove a table column style.
     */
    void remove(KoTableColumnStyle *style);
    /**
     * Remove a table row style.
     */
    void remove(KoTableRowStyle *style);
    /**
     * Remove a table cell style.
     */
    void remove(KoTableCellStyle *style);
    /**
     * Remove a section style.
     */
    void remove(KoSectionStyle *style);

    /**
     * Add a document for which the styles will be applied.
     * Whenever a style is changed (signified by a alteredStyle() call) all
     * registered documents will be updated to reflect that change.
     */
    void add(QTextDocument *document);
    /**
     * Remove a previously registered document.
     */
    void remove(QTextDocument *document);

    /**
     * Return a characterStyle by its id.
     * From documents you can retrieve the id out of each QTextCharFormat
     * by requesting the KoCharacterStyle::StyleId property.
     * @param id the unique Id to search for.
     * @see KoCharacterStyle::styleId()
     */
    KoCharacterStyle *characterStyle(int id) const;

    /**
     * Return a paragraphStyle by its id.
     * From documents you can retrieve the id out of each QTextBlockFormat
     * by requesting the KoParagraphStyle::StyleId property.
     * @param id the unique Id to search for.
     * @see KoParagraphStyle::styleId()
     */
    KoParagraphStyle *paragraphStyle(int id) const;

    /**
     * Return a list style by its id.
     */
    KoListStyle *listStyle(int id) const;

    /**
     * Return a tableStyle by its id.
     * From documents you can retrieve the id out of each QTextTableFormat
     * by requesting the KoTableStyle::StyleId property.
     * @param id the unique Id to search for.
     * @see KoTableStyle::styleId()
     */
    KoTableStyle *tableStyle(int id) const;

    /**
     * Return a tableColumnStyle by its id.
     * From documents you can retrieve the id out of the KoTableRowandColumnStyleManager
     * @param id the unique Id to search for.
     * @see KoTableColumnStyle::styleId()
     */
    KoTableColumnStyle *tableColumnStyle(int id) const;

    /**
     * Return a tableRowStyle by its id.
     * From documents you can retrieve the id out of the KoTableRowandColumnStyleManager
     * @param id the unique Id to search for.
     * @see KoTableRowStyle::styleId()
     */
    KoTableRowStyle *tableRowStyle(int id) const;

    /**
     * Return a tableCellStyle by its id.
     * From documents you can retrieve the id out of each QTextTableCellFormat
     * by requesting the KoTableCellStyle::StyleId property.
     * @param id the unique Id to search for.
     * @see KoTableCellStyle::styleId()
     */
    KoTableCellStyle *tableCellStyle(int id) const;

    /**
     * Return a sectionStyle by its id.
     * From documents you can retrieve the id out of each QTextFrameFormat
     * by requesting the KoSectionStyle::StyleId property.
     * @param id the unique Id to search for.
     * @see KoSectionStyle::styleId()
     */
    KoSectionStyle *sectionStyle(int id) const;

    /**
     * Return the first characterStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see characterStyle(id);
     */
    KoCharacterStyle *characterStyle(const QString &name) const;

    /**
     * Return the first paragraphStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see paragraphStyle(id);
     */
    KoParagraphStyle *paragraphStyle(const QString &name) const;

    /**
     * Returns the first  listStyle ith the param use-visible-name.
     */
    KoListStyle *listStyle(const QString &name) const;

    /**
     * Return the first tableStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see tableStyle(id);
     */
    KoTableStyle *tableStyle(const QString &name) const;

    /**
     * Return the first tableColumnStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see tableColumnStyle(id);
     */
    KoTableColumnStyle *tableColumnStyle(const QString &name) const;

    /**
     * Return the first tableRowStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see tableRowStyle(id);
     */
    KoTableRowStyle *tableRowStyle(const QString &name) const;

    /**
     * Return the first tableCellStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see tableCellStyle(id);
     */
    KoTableCellStyle *tableCellStyle(const QString &name) const;

    /**
     * Return the first sectionStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see sectionStyle(id);
     */
    KoSectionStyle *sectionStyle(const QString &name) const;

     /**
     * Return the default paragraph style that will always be present in each
     * document. You can alter the style, but you can never delete it.
     * The default is suppost to stay invisible to the user and its called
     * i18n("[No Paragraph Style]") for that reason. Applications should not
     * show this style in their document-level configure dialogs.
     */
    KoParagraphStyle *defaultParagraphStyle() const;

    /**
     * Returns the default list style to be used for lists, headers, paragraphs
     * that do not specify a list-style
     */
    KoListStyle *defaultListStyle() const;

    /**
     * Sets the outline style to be used for headers that are not specified as lists
     */
    void setOutlineStyle(KoListStyle *listStyle);

    /**
     * Returns the outline style to be used for headers that are not specified as lists
     */
    KoListStyle *outlineStyle() const;

    /// return all the characterStyles registered.
    QList<KoCharacterStyle*> characterStyles() const;

    /// return all the paragraphStyles registered.
    QList<KoParagraphStyle*> paragraphStyles() const;

    /// return all the listStyles registered.
    QList<KoListStyle*> listStyles() const;

    /// return all the tableStyles registered.
    QList<KoTableStyle*> tableStyles() const;

    /// return all the tableColumnStyles registered.
    QList<KoTableColumnStyle*> tableColumnStyles() const;

    /// return all the tableRowStyles registered.
    QList<KoTableRowStyle*> tableRowStyles() const;

    /// return all the tableCellStyles registered.
    QList<KoTableCellStyle*> tableCellStyles() const;

    /// return all the sectionStyles registered.
    QList<KoSectionStyle*> sectionStyles() const;

signals:
    void styleAdded(KoParagraphStyle*);
    void styleAdded(KoCharacterStyle*);
    void styleAdded(KoListStyle*);
    void styleAdded(KoTableStyle*);
    void styleAdded(KoTableColumnStyle*);
    void styleAdded(KoTableRowStyle*);
    void styleAdded(KoTableCellStyle*);
    void styleAdded(KoSectionStyle*);
    void styleRemoved(KoParagraphStyle*);
    void styleRemoved(KoCharacterStyle*);
    void styleRemoved(KoListStyle*);
    void styleRemoved(KoTableStyle*);
    void styleRemoved(KoTableColumnStyle*);
    void styleRemoved(KoTableRowStyle*);
    void styleRemoved(KoTableCellStyle*);
    void styleRemoved(KoSectionStyle*);

public slots:
    /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KoParagraphStyle *style);
    /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KoCharacterStyle *style);

    /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KoListStyle *style);

    /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KoTableStyle *style);

    /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KoTableColumnStyle *style);

     /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KoTableRowStyle *style);

   /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KoTableCellStyle *style);

   /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KoSectionStyle *style);

private slots:
    void updateAlteredStyles(); // for the QTimer::singleshot

private:
    friend class ChangeFollower;
    void requestFireUpdate();
    void remove(ChangeFollower *cf);

    friend class KoTextSharedLoadingData;
    void addAutomaticListStyle(KoListStyle *listStyle);
    friend class KoTextShapeData;
    friend class KoTextShapeDataPrivate;
    KoListStyle *listStyle(int id, bool *automatic) const;

private:
    class Private;
    Private* const d;
};

Q_DECLARE_METATYPE(KoStyleManager*)

#endif
