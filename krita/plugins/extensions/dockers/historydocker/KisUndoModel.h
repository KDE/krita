#include <QAbstractItemModel>

#include <QUndoStack>
#include <QItemSelectionModel>
#include <QIcon>
#include <QUndoCommand>
#include "kis_types.h"
#include "kis_canvas2.h"
#include "kis_view2.h"
#include "kis_image.h"
#include "kis_paint_device.h"

class QUndoModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    QUndoModel(QObject *parent = 0);

    QUndoStack *stack() const;

    virtual QModelIndex index(int row, int column,
                const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QModelIndex selectedIndex() const;
    QItemSelectionModel *selectionModel() const;

    QString emptyLabel() const;
    void setEmptyLabel(const QString &label);

    void setCleanIcon(const QIcon &icon);
    QIcon cleanIcon() const;

    void setCanvas(KisCanvas2* canvas);

public slots:
    void setStack(QUndoStack *stack);
    void addImage(int idx);

private slots:
    void stackChanged();
    void stackDestroyed(QObject *obj);
    void setStackCurrentIndex(const QModelIndex &index);

private:
    QUndoStack *m_stack;
    QItemSelectionModel *m_sel_model;
    QString m_emty_label;
    QIcon m_clean_icon;
    KisCanvas2* m_canvas;
    QMap<const QUndoCommand*, QImage> imageMap;
};
