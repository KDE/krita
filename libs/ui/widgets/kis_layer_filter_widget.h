#ifndef KISLAYERFILTERWIDGET_H
#define KISLAYERFILTERWIDGET_H

#include <QWidget>
#include "kis_types.h"

#include "kritaui_export.h"


class KRITAUI_EXPORT KisLayerFilterWidget : public QWidget
{
    Q_OBJECT
private:

    class EventFilter : public QObject {
    private:
        QWidget* m_buttonContainer;

        enum State{
            Idle,
            WaitingForDragLeave, //Waiting for mouse to exit first clicked while the mouse button is down.
            WaitingForDragEnter //Waiting for mouse to slide across buttons within the same button group.
        };

        State currentState;
        QPoint lastKnownMousePosition;

    public:
        EventFilter(QWidget *buttonContainer, QObject *parent = nullptr);

    protected:
        bool eventFilter(QObject *obj, QEvent *event);

        void checkSlideOverNeighborButtons(QMouseEvent* mouseEvent, class QAbstractButton* startingButton);

        bool tryToggleButton(class QAbstractButton* btn) const;
    };

    EventFilter *buttonEventFilter;
    class QLineEdit *textFilter;

    QList<class KisColorLabelButton*> colorLabelButtons;

public:
    KisLayerFilterWidget(QWidget *parent = nullptr);

    static void scanUsedColorLabels(KisNodeSP node, QSet<int> &colorLabels);
    void updateColorLabels(KisNodeSP root);

    bool isCurrentlyFiltering();
    QList<int> getActiveColors();
    QString getTextFilter();

    void reset();

Q_SIGNALS:
    void filteringOptionsChanged();

};

#endif // KISLAYERFILTERWIDGET_H
