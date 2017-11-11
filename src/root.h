#ifndef ROOT_H
#define ROOT_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

/// \brief Root application controller
class root : public Controller
{
    Q_OBJECT
    C_NAMESPACE("")
public:
    explicit root(QObject *parent = 0);

    C_ATTR(index, :Path("/") :Args(0))
    void index(Context *c);
    
    C_ATTR(check, :Local :Args(0))
    void check(Context *c);

    C_ATTR(defaultPage, :Path)
    void defaultPage(Context *c);

private:
    C_ATTR(Auto, :Private)
    bool Auto(Context *c);
    
    C_ATTR(End, :ActionClass("RenderView"))
    void End(Context *c) { Q_UNUSED(c); }
};

#endif //ROOT_H

