#include "root.h"
#include <Cutelyst/Plugins/Authentication/authentication.h>

using namespace Cutelyst;

/// \brief Default constructor
root::root(QObject *parent) : Controller(parent) {}

/// \brief Show main page
void root::index(Context *c)
{
    auto user = Authentication::user(c);
    c->stash({
        { "bodyclass",   "dark"        },
        { "template",    "player.html" },
        { "username",    user.id() },
        { "is_admin",    user["is_admin"].toInt() == 1 },
        { "scripts",     QVariantList { 
            "/static/js/dash.all.min.js"
        }},
        { "stylesheets", QVariantList { 
            "/static/css/spectre.min.css",
            "/static/css/player.css"
        }}
    });
}

/// \brief Show 404 error
void root::defaultPage(Context *c)
{
    c->response()->body() = "Page not found!";
    c->response()->setStatus(404);
}

/// \brief Check, if user is permitted to view stream
void root::check(Context *c) {
    c->setView(QStringLiteral("dummy"));
    c->response()->setStatus(200);
}

/// \brief Check if there is a user and, if not, forward to login page
bool root::Auto(Context *c) {
    // Allow unauthenticated users to reach the login page.  This
    // allows unauthenticated users to reach any action in the Login
    // controller.  To lock it down to a single action, we could use:
    //   if (c->action() eq c->controller("Login")->actionFor("index"))
    // to only allow unauthenticated access to the 'index' action we
    // added above
    if (c->controller() == c->controller("login")) {
        return true;
    }

    // If a user doesn't exist, force login
    if (!Authentication::userExists(c)) {
        // Dump a log message to the development server debug output
        qDebug("***Root::Auto User not found, forwarding to /login");

        // Redirect the user to the login page
        c->response()->redirect(c->uriFor("/login"));

        // Return false to cancel 'post-auto' processing and prevent use of application
        return false;
    }

    // User found, so return true to continue with processing after this 'auto'
    return true;
}
