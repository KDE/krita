#ifndef KisUndoView_H
#define KisUndoView_H

#include <QtGui/qlistview.h>
#include <QtCore/qstring.h>
#include "kis_canvas2.h"

#ifndef QT_NO_UNDOVIEW

class KisUndoViewPrivate;
class QUndoStack;
class QUndoGroup;
class QIcon;

class KisUndoView : public QListView
{
    Q_OBJECT
    Q_PROPERTY(QString emptyLabel READ emptyLabel WRITE setEmptyLabel)
    Q_PROPERTY(QIcon cleanIcon READ cleanIcon WRITE setCleanIcon)

public:
    explicit KisUndoView(QWidget *parent = 0);
    explicit KisUndoView(QUndoStack *stack, QWidget *parent = 0);
#ifndef QT_NO_UNDOGROUP
    explicit KisUndoView(QUndoGroup *group, QWidget *parent = 0);
#endif
    ~KisUndoView();

    QUndoStack *stack() const;
#ifndef QT_NO_UNDOGROUP
    QUndoGroup *group() const;
#endif

    void setEmptyLabel(const QString &label);
    QString emptyLabel() const;

    void setCleanIcon(const QIcon &icon);
    QIcon cleanIcon() const;

    //my new imba function
    void setCanvas(KisCanvas2* canvas);

public Q_SLOTS:
    void setStack(QUndoStack *stack);
#ifndef QT_NO_UNDOGROUP
    void setGroup(QUndoGroup *group);
#endif

private:
    KisUndoViewPrivate* const d;
    Q_DISABLE_COPY(KisUndoView)
};

#endif // QT_NO_UNDOVIEW
#endif // KisUndoView_H
