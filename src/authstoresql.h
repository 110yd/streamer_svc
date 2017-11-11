#ifndef AUTHSTORESQL_H
#define AUTHSTORESQL_H

#include <Cutelyst/Plugins/Authentication/authenticationstore.h>

using namespace Cutelyst;

/// \brief SQLite storage for registered users
class AuthStoreSql : public AuthenticationStore
{
public:
    explicit AuthStoreSql(QObject *parent = 0);
    virtual AuthenticationUser findUser(Context *c, 
                                        const ParamsMultiMap &userinfo);

    static QByteArray hashFor(const QString &password);
    
private:
    QString m_idField;
};

#endif // AUTHSTORESQL_H

