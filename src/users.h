#ifndef USERS_H
#define USERS_H

#include <Cutelyst/Controller>
#include <QRegExp>

using namespace Cutelyst;

/// \brief Users storage controller
class users : public Controller
{
    Q_OBJECT
public:
    explicit users(QObject *parent = 0);

    C_ATTR(index, :Path :AutoArgs)
    void index(Context *c);
    
    C_ATTR(list, :Local)
    void list(Context *c);
    
    C_ATTR(create, :Local)
    void create(Context *c);
    
    C_ATTR(createMultiple, :Local)
    void createMultiple(Context *c);
    
    C_ATTR(remove, :Local)
    void remove(Context *c);
    
    C_ATTR(removeInactive, :Local)
    void removeInactive(Context *c);
    
    C_ATTR(removeAll, :Local)
    void removeAll(Context *c);
    
    C_ATTR(activate, :Local)
    void activate(Context *c);
    
    C_ATTR(modify, :Local)
    void modify(Context *c);
        
    QVariantList listUsers();
    
private:
    void htmlPage(Context *c);
    void noCache(Context *c);
    
    C_ATTR(Auto, :Private)
    bool Auto(Context *c);
    
private:
    QRegExp m_usernameval;
};

#endif //USERS_H

