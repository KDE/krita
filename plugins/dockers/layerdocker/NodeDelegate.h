/*
  SPDX-FileCopyrightText: 2006 Gábor Lehel <illissius@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIS_DOCUMENT_SECTION_DELEGATE_H
#define KIS_DOCUMENT_SECTION_DELEGATE_H

#include <QAbstractItemDelegate>
class NodeView;

class KisNodeModel;

/**
 * See KisNodeModel and NodeView.
 *
 * A delegate provides the gui machinery, using Qt's model/view terminology.
 * This class is owned by NodeView to do the work of generating the
 * graphical representation of each item.
 */
class NodeDelegate: public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit NodeDelegate(NodeView *view, QObject *parent = 0);
    ~NodeDelegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& index) const override;

    void toggleSolo(const QModelIndex &index);

    void slotUpdateIcon();

Q_SIGNALS:
    void resetVisibilityStasis();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    typedef KisNodeModel Model;
    typedef NodeView View;
    class Private;
    Private* const d;

    static QStyleOptionViewItem getOptions(const QStyleOptionViewItem &option, const QModelIndex &index);
    void drawProgressBar(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void drawBranch(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawColorLabel(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawFrame(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QRect thumbnailClickRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawThumbnail(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QRect iconsRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QRect textRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawText(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawIcons(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QRect visibilityClickRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawVisibilityIconHijack(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QRect decorationClickRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawDecoration(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawExpandButton(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawAnimatedDecoration(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private Q_SLOTS:
    void slotConfigChanged();
    void slotResetState();
};

#endif
