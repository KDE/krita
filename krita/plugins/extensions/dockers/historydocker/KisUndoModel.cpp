#include "KisUndoModel.h"

QUndoModel::QUndoModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_stack = 0;
    m_canvas = 0;
    m_sel_model = new QItemSelectionModel(this, this);
    connect(m_sel_model, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(setStackCurrentIndex(QModelIndex)));
    m_emty_label = tr("<empty>");
}

QItemSelectionModel *QUndoModel::selectionModel() const
{
    return m_sel_model;
}

QUndoStack *QUndoModel::stack() const
{
    return m_stack;
}

void QUndoModel::setStack(QUndoStack *stack)
{
    if (m_stack == stack)
        return;

    if (m_stack != 0) {
        disconnect(m_stack, SIGNAL(cleanChanged(bool)), this, SLOT(stackChanged()));
        disconnect(m_stack, SIGNAL(indexChanged(int)), this, SLOT(stackChanged()));
        disconnect(m_stack, SIGNAL(destroyed(QObject*)), this, SLOT(stackDestroyed(QObject*)));
        disconnect(m_stack, SIGNAL(indexChanged(int)), this, SLOT(addImage(int)));
    }
    m_stack = stack;
    if (m_stack != 0) {
        connect(m_stack, SIGNAL(cleanChanged(bool)), this, SLOT(stackChanged()));
        connect(m_stack, SIGNAL(indexChanged(int)), this, SLOT(stackChanged()));
        connect(m_stack, SIGNAL(destroyed(QObject*)), this, SLOT(stackDestroyed(QObject*)));
        connect(m_stack, SIGNAL(indexChanged(int)), this, SLOT(addImage(int)));
    }

    stackChanged();
}

void QUndoModel::stackDestroyed(QObject *obj)
{
    if (obj != m_stack)
        return;
    m_stack = 0;

    stackChanged();
}

void QUndoModel::stackChanged()
{
    reset();
    m_sel_model->setCurrentIndex(selectedIndex(), QItemSelectionModel::ClearAndSelect);
}

void QUndoModel::setStackCurrentIndex(const QModelIndex &index)
{
    if (m_stack == 0)
        return;

    if (index == selectedIndex())
        return;

    if (index.column() != 0)
        return;

    m_stack->setIndex(index.row());
}

QModelIndex QUndoModel::selectedIndex() const
{
    return m_stack == 0 ? QModelIndex() : createIndex(m_stack->index(), 0);
}

QModelIndex QUndoModel::index(int row, int column, const QModelIndex &parent) const
{
    if (m_stack == 0)
        return QModelIndex();

    if (parent.isValid())
        return QModelIndex();

    if (column != 0)
        return QModelIndex();

    if (row < 0 || row > m_stack->count())
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex QUndoModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int QUndoModel::rowCount(const QModelIndex &parent) const
{
    if (m_stack == 0)
        return 0;

    if (parent.isValid())
        return 0;

    return m_stack->count() + 1;
}

int QUndoModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant QUndoModel::data(const QModelIndex &index, int role) const
{
    if (m_stack == 0)
        return QVariant();

    if (index.column() != 0)
        return QVariant();

    if (index.row() < 0 || index.row() > m_stack->count())
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (index.row() == 0)
            return m_emty_label;
        return m_stack->text(index.row() - 1);
    } else if (role == Qt::DecorationRole) {
        if (index.row() == m_stack->cleanIndex() && !m_clean_icon.isNull()) {
            return m_clean_icon;
        } else {
            const QUndoCommand* currentCommand = m_stack->command(index.row() - 1);
            return imageMap[currentCommand];
        }
    }

    return QVariant();
}

QString QUndoModel::emptyLabel() const
{
    return m_emty_label;
}

void QUndoModel::setEmptyLabel(const QString &label)
{
    m_emty_label = label;
    stackChanged();
}

void QUndoModel::setCleanIcon(const QIcon &icon)
{
    m_clean_icon = icon;
    stackChanged();
}

QIcon QUndoModel::cleanIcon() const
{
    return m_clean_icon;
}

void QUndoModel::setCanvas(KisCanvas2 *canvas) {
    m_canvas = canvas;
}

void QUndoModel::addImage(int idx) {
    const QUndoCommand* currentCommand = m_stack->command(idx-1);
    if( m_stack->count() == idx && !imageMap.contains(currentCommand)) {
        KisImageWSP historyImage = m_canvas->view()->image();
        KisPaintDeviceSP paintDevice = historyImage->projection();
        QImage image = paintDevice->createThumbnail(32, 32);
        imageMap[currentCommand] = image;
    }
    qDebug() << "value of stack count : " << m_stack->count() << " idx " << idx << "  ImageMap " << imageMap.contains(currentCommand);
}
