class View : QObject
{
%TypeHeaderCode
#include "View.h"
%End
    View(const View & __0);
public Q_SLOTS:
    Window * window() const /Factory/;
    Document * document() const /Factory/;
    bool visible() const;
    void setVisible();
    Canvas * canvas() const /Factory/;
    void close(bool confirm);
    void activateResource(Resource *resource);
private:
};
