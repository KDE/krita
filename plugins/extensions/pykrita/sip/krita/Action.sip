class Action : QObject
{
%TypeHeaderCode
#include "Action.h"
%End
public:
    Action(QObject*  parent /TransferThis/ = 0);
    Action(const QString & name, QAction*  action, QObject*  parent /TransferThis/ = 0);
    virtual ~Action();
    QString name() const;
    void setName(QString value);
    QString menu() const;
    void setMenu(QString value);
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
public Q_SLOTS:
    void trigger();
Q_SIGNALS:
    void triggered(bool);
private:
    private:
    Action(const Action &); // Generated
};

