class View : QObject
{
%TypeHeaderCode
#include "View.h"
%End
    View(const View & __0);
public:
    Window * window() const;
    Document * document() const;
    bool visible() const;
    void setVisible(bool value);
    Canvas * canvas() const;
public Q_SLOTS:
    void close(bool confirm);
private:
};
