#include "streaming_svc.h"

#include <Cutelyst/Plugins/StaticSimple/staticsimple.h>
#include <Cutelyst/Plugins/View/Grantlee/grantleeview.h>
#include <Cutelyst/Plugins/View/JSON/viewjson.h>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>

#include <QtSql>
#include <QDebug>

#include "dummyview.h"

#include "root.h"
#include "users.h"
#include "login.h"
#include "logout.h"
#include "authstoresql.h"
#include "authenticatedstatic.h"

using namespace Cutelyst;

/// \brief Default constructor
streaming_svc::streaming_svc(QObject *parent) : Application(parent) {}

/// \brief Service initialization
bool streaming_svc::init()
{
    //-- Create controllers
    new root(this);
    new users(this);
    new login(this);
    new logout(this);
    
    //-- Activate components
    //new StaticSimple(this);
    new Session(this);
    auto authstatic = new AuthenticatedStatic(this);
    authstatic->setBasePath       ("dash/");
    authstatic->setStaticDirectory("/tmp/dash/");
    
    //-- Activate view engine
    auto view = new GrantleeView(this);
    view->setIncludePaths({ pathTo({ "root", "src" }) });
    view->setWrapper("wrapper.html");
    
    new dummyview(this, QStringLiteral("dummy"));    
    new ViewJson (this, QStringLiteral("json"));
    
    //-- Configure authentication
    auto auth = new Authentication(this);
    auto credential = new CredentialPassword;
    credential->setPasswordType(CredentialPassword::Hashed);
    auth->addRealm(new AuthStoreSql, credential);
    
    return true;
}

/// \brief Init database connection
bool streaming_svc::postFork()
{
    QSqlDatabase db = QSqlDatabase::addDatabase(
        "QSQLITE", 
        Sql::databaseNameThread("MyDB"));
    
    db.setDatabaseName  ("streaming_svcdb.sqlite");
    db.setConnectOptions("foreign_keys = ON");
    
    if (!db.open()) {
        qCritical() << "Failed to open database:" << db.lastError().text();
        return false;
    }
    
    return true;
}

