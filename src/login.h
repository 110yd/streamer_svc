#ifndef LOGIN_H
#define LOGIN_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

/// \brief Application login controller
class login : public Controller
{
    Q_OBJECT
public:
    explicit login (QObject *parent = 0);
    
    C_ATTR(index, :Path :Args(0))
    void index(Context *c);
};

#endif //LOGIN_H

