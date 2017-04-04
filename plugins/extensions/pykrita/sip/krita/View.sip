class View : QObject
{
%TypeHeaderCode
#include "View.h"
%End
    View(const View & __0);
public:    
    bool operator==(const View &other) const;
    bool operator!=(const View &other) const;

public Q_SLOTS:
    Window * window() const /Factory/;
    Document * document() const /Factory/;
    bool visible() const;
    void setVisible();
    Canvas * canvas() const /Factory/;
    void activateResource(Resource *resource);
private:
};
