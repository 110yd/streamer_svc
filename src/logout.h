#ifndef LOGOUT_H
#define LOGOUT_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

/// \brief Logout action controller
class logout : public Controller
{
    Q_OBJECT
public:
    explicit logout (QObject *parent = 0);

    C_ATTR(index, :Path :Args(0))
    void index(Context *c);
};

#endif //LOGOUT_H

