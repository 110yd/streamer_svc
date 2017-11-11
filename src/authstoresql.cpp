#include "authstoresql.h"

#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QCryptographicHash>
#include "sql_helper.h"

/// \brief Default constructor
AuthStoreSql::AuthStoreSql(QObject *parent) : AuthenticationStore(parent)
{
    m_idField = "username";
}

/// \brief Find user with corresponding identifier
///
/// \param c
///        Request context
///
/// \param userInfo
///        Requested user information
///
AuthenticationUser AuthStoreSql::findUser(Context *c, 
                                          const ParamsMultiMap &userinfo)
{
    Q_UNUSED(c);
    
    QString id = userinfo[m_idField];
    AuthenticationUser user(id);

    QSqlQuery query = CPreparedSqlQueryThreadForDB(
        "SELECT * FROM users WHERE username = :username", "MyDB");
    
    query.bindValue(":username", id);
        
    if (sql_helper::exec_single(query, user)) {
        
        if (user["active"].toInt() != 1)
            return AuthenticationUser();
        
        auto vlstr = user["valid_to"].toString();
        
        if (!vlstr.isEmpty()) {
            auto valid_to = QDate::fromString(vlstr, Qt::ISODate);
                        
            if (valid_to < QDate::currentDate())
                return AuthenticationUser();
        }
        
        return user;
    }
    
    return AuthenticationUser();
}

/// \brief Obtain hash from password
QByteArray AuthStoreSql::hashFor(const QString &password) {
    return CredentialPassword::createPassword(         
        password.toUtf8(),
        QCryptographicHash::Sha256,                                         
        3, // iterations
        16, // bytes salt
        16 // bytes hash
    );
}
