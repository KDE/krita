#ifndef KOSECTIONMODEL_H
#define KOSECTIONMODEL_H

#include <QTextDocument>
#include <QAbstractItemModel>
#include <QVector>
#include <QSet>

#include <KoSection.h>
#include <KoSectionEnd.h>

/**
 * Used to handle all the sections in the document
 *
 * Now there actually two levels of section handling:
 * 1) Formatting Level: on this level we should be sure, that
 * pointers to KoSection and KoSectionEnd in the KoParagraphStyles
 * properties SectionEndings and SectionStartings are consistent.
 * Handling on this level is provided on the level of text editing
 * commands: DeleteCommand, NewSectionCommand
 * We can't move it to another place, because we should know the
 * semantics of operation to handle it right way.
 * 2) Model(Tree) Level: on this level we should update KoSectionModel
 * right way, so it in any moment represents the actual tree
 * of sections. Tree is builded easily:
 *    One section is son of another, if it is directly nested in it.
 * As text editing commands have access to change Formatting Level,
 * they are declared as friend classes of KoSectionModel to be able
 * affect Model structure without changing something on Formatting
 * Level. Also affected by RenameSectionCommand.
 *
 * Also we need to look at the consistency of some section properties:
 *
 * 1) Bounds. Those now are handled with QTextCursors that are placed
 * on start and end of the section. In default state start cursor
 * isn't moving if text inserted in its position, and end cursor
 * moves. But in the case of initial document loading, it is necessary
 * to make some end cursors stop moving, so we have:
 *         KoTextLoader -> calling -> KoSection::setKeepEndBound()
 *         KoTextLoader -> calling -> KoSectionModel::allowMovingEndBound()
 *      ^-- this needed to restore defaul behaviour after load
 *
 * 2) Level. Level means the depth of the section in tree. Root
 * sections has 0 (zero) level. Now if you look at the possible
 * text editing command affecting sections you may notice that
 * level of section doesn't change in any case. Initial level
 * is set in KoSection constructor as parent's level plus one.
 * TODO: write about drag-n-drop here, when its implemented
 *
 * 3) Name. Each KoSection has a name that must be unique. We have
 * two groups of KoSections in each moment of time: the first group
 * consists of the sections that are present in document now,
 * the second group consists of the sections that were deleted, but
 * we still need them as they may be restored with "undo".
 * This groups are stored in m_registeredSections and m_sectionNames.
 *
 * Sections are created through this newSection() and newSectionEnd()
 * functions.
 *
 * This object is created for QTextDocument on the first query of it.
 */
class KRITATEXT_EXPORT KoSectionModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    static const int PointerRole = Qt::UserRole;

    explicit KoSectionModel(QTextDocument *doc);
    virtual ~KoSectionModel();

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    /// Creates KoSection in position of @param cursor with some allowed name
    KoSection *createSection(const QTextCursor &cursor, KoSection *parent);

    /// Creates KoSection in position of @param cursor with specified @param name
    KoSection *createSection(const QTextCursor &cursor, KoSection *parent, const QString &name);

    /// Creates KoSectionEnd in pair for a @param section
    KoSectionEnd *createSectionEnd(KoSection *section);

    /** Tries to set @param section name to @param name
     * @return @c false if there is a section with such name
     * and new name isn't accepted and @c true otherwise.
     */
    bool setName(KoSection *section, const QString &name);

    /**
     * Returns pointer to the deepest KoSection that covers @p pos
     * or NULL if there is no such section
     */
    KoSection *sectionAtPosition(int pos);

    /// Returns name for the new section.
    QString possibleNewName();

    /// Returns if this name is possible.
    bool isValidNewName(const QString &name) const;

    /// Setting all sections end bound cursor to move with text inserting.
    void allowMovingEndBound();

    /// Finds index of @param child inside his parent.
    int findRowOfChild(KoSection *child) const;

private:
    Q_DISABLE_COPY(KoSectionModel)

    friend class DeleteCommand;
    friend class NewSectionCommand;

    /**
     * Inserts @param section to it's parent (should be
     * stored in @param section already) in position childIdx.
     * Affects only Model Level(@see KoSectionModel).
     */
    void insertToModel(KoSection* section, int childIdx);
    /**
     * Deletes @param section from it's parent (should be
     * stored in @param section already).
     * Affects only Model Level(@see KoSectionModel).
     */
    void deleteFromModel(KoSection *section);

    QTextDocument *m_doc;
    QSet<KoSection *> m_registeredSections; ///< stores pointer to sections that sometime was registered
    QHash<QString, KoSection *> m_sectionNames; ///< stores name -> pointer reference, for sections that are visible in document now
    QHash<KoSection *, QPersistentModelIndex> m_modelIndex;

    QVector<KoSection *> m_rootSections;

};

Q_DECLARE_METATYPE(KoSectionModel *)

#endif //KOSECTIONMODEL_H
