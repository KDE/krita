class Action : QObject
{
%TypeHeaderCode
#include "Action.h"
%End

public:
    explicit Action(QObject *parent /TransferThis/ = 0);
    Action(const QString &name, QAction* action, QObject *parent /TransferThis/ = 0);
    virtual ~Action();
    bool operator==(const Action &other) const;
    bool operator!=(const Action &other) const;
public Q_SLOTS:
    QString text() const;
    void setText(QString text);
    QString name() const;
    void setName(QString value);
    bool isCheckable() const;
    void setCheckable(bool value);
    bool isChecked() const;
    void setChecked(bool value);
    QString shortcut() const;
    void setShortcut(QString value);
    bool isVisible() const;
    void setVisible(bool value);
    bool isEnabled() const;
    void setEnabled(bool value);
    void setToolTip(QString tooltip);
    QString tooltip() const;
    void trigger();
    void setMenu(const QString menu);
    QString menu() const;
Q_SIGNALS:
    void triggered(bool);
private:
    private:
    Action(const Action &); // Generated
};
