/*
    SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef COMPOSITEOPMODEL_H
#define COMPOSITEOPMODEL_H

#include <QModelIndex>
#include <kis_layer.h>
#include <kis_types.h>

class KoCanvasController;

class CompositeOpModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* view READ view WRITE setView NOTIFY viewChanged);
    Q_PROPERTY(bool eraserMode READ eraserMode WRITE setEraserMode NOTIFY eraserModeChanged);

    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged);
    Q_PROPERTY(bool opacityEnabled READ opacityEnabled WRITE setOpacityEnabled NOTIFY opacityEnabledChanged);
    Q_PROPERTY(qreal flow READ flow WRITE setFlow NOTIFY flowChanged);
    Q_PROPERTY(bool flowEnabled READ flowEnabled WRITE setFlowEnabled NOTIFY flowEnabledChanged);
    Q_PROPERTY(qreal size READ size WRITE setSize NOTIFY sizeChanged);
    Q_PROPERTY(bool sizeEnabled READ sizeEnabled WRITE setSizeEnabled NOTIFY sizeEnabledChanged);

    Q_PROPERTY(bool mirrorHorizontally READ mirrorHorizontally WRITE setMirrorHorizontally NOTIFY mirrorHorizontallyChanged);
    Q_PROPERTY(bool mirrorVertically READ mirrorVertically WRITE setMirrorVertically NOTIFY mirrorVerticallyChanged);

    Q_PROPERTY(QString currentCompositeOpID READ currentCompositeOpID NOTIFY currentCompositeOpIDChanged);
public:
    enum CompositeOpModelRoles {
        TextRole = Qt::UserRole + 1,
        IsCategoryRole
    };
    explicit CompositeOpModel(QObject* parent = 0);
    virtual ~CompositeOpModel();
    QHash<int, QByteArray> roleNames() const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    Q_INVOKABLE void activateItem(int index);

    QObject* view() const;
    void setView(QObject* newView);
    bool eraserMode() const;
    void setEraserMode(bool newEraserMode);

    qreal opacity() const;
    void setOpacity(qreal newOpacity);
    bool opacityEnabled() const;
    void setOpacityEnabled(bool newOpacityEnabled);

    qreal flow() const;
    void setFlow(qreal newFlow);
    bool flowEnabled() const;
    void setFlowEnabled(bool newFlowEnabled);

    qreal size() const;
    void setSize(qreal newSize);
    bool sizeEnabled() const;
    void setSizeEnabled(bool newSizeEnabled);

    bool mirrorHorizontally() const;
    void setMirrorHorizontally(bool newMirrorHorizontally);
    bool mirrorVertically() const;
    void setMirrorVertically(bool newMirrorVertically);

    QString currentCompositeOpID() const;

    Q_INVOKABLE void changePaintopValue(QString propertyName, QVariant value);
    Q_INVOKABLE int indexOf(QString compositeOpId);

Q_SIGNALS:
    void viewChanged();
    void eraserModeChanged();

    void opacityChanged();
    void opacityEnabledChanged();
    void flowChanged();
    void flowEnabledChanged();
    void sizeChanged();
    void sizeEnabledChanged();

    void mirrorHorizontallyChanged();
    void mirrorVerticallyChanged();

    void currentCompositeOpIDChanged();

private Q_SLOTS:
    void slotToolChanged(KoCanvasController* canvas, int toolId);
    void resourceChanged(int key, const QVariant& v);
    void currentNodeChanged(KisLayerSP newNode);

private:
    class Private;
    Private* d;
};

#endif // COMPOSITEOPMODEL_H
