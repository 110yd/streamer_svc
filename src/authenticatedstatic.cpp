#include "authenticatedstatic.h"
#include <QLoggingCategory>
#include <QDir>
#include <QMimeDatabase>
#include <QMimeType>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Request>
#include <Cutelyst/Response>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_AUTHSTATIC, "streaming_svc.authenticatedstatic")
static QMimeDatabase db;

/// \brief Default constructor
AuthenticatedStatic::AuthenticatedStatic(Cutelyst::Application* app)
    : Cutelyst::Plugin(app)
{
    m_staticDirectory = app->config(QLatin1String("root")).toString();
    m_basePath        = "authcontent/";
}

/// \brief Dummy destructor
AuthenticatedStatic::~AuthenticatedStatic()
{}

/// \brief Override default settings
void AuthenticatedStatic::setBasePath(const QString& basePath)
{
    m_basePath = basePath;
}

/// \brief Override default settings
void AuthenticatedStatic::setStaticDirectory(const QString& staticDirectory)
{
    m_staticDirectory = staticDirectory;
}

/// \brief Perform plugin initialization
bool AuthenticatedStatic::setup(Cutelyst::Application* app)
{
    connect(app,  &Application::beforePrepareAction,
            this, &AuthenticatedStatic::beforePrepareAction);
    
    return true;
}

/// \brief Capture HTTP request
void AuthenticatedStatic::beforePrepareAction(Cutelyst::Context *c, 
                                              bool *skipMethod) 
{
    if (*skipMethod) 
        return;
    
     if (c->req()->path().startsWith(m_basePath)) {
         *skipMethod = true;
         serveStaticFile(c);
     }
}

/// \brief Respond with static file content
void AuthenticatedStatic::serveStaticFile(Cutelyst::Context* c)
{
    if (!Authentication::userExists(c)) {
        respondForbidden(c);
        return;
    }
    
    auto subpath = c->req()->path().mid(m_basePath.length());
    
    QDir      basePath(m_staticDirectory);
    QString   filepath = basePath.absoluteFilePath(subpath);
    QFileInfo info(filepath);
    
    if (!info.exists()) {
        qCWarning(C_AUTHSTATIC) << "Not found " << filepath 
                                                << subpath 
                                                << m_basePath;
        respondNotFound(c);
    }
    else 
        respondFile(c, filepath);
}

/// \brief Respond with HTTP 403 Forbidden status code
void AuthenticatedStatic::respondForbidden(Cutelyst::Context* c)
{
    Response *res = c->res();
    res->setStatus(Response::Forbidden);
}

/// \brief Respond with HTTP 404 Not Found status code
void AuthenticatedStatic::respondNotFound(Cutelyst::Context* c)
{
    c->res()->setStatus(Response::NotFound);
}

/// \brief Responsd with file
void AuthenticatedStatic::respondFile(Cutelyst::Context* c, 
                                      const QString& filepath)
{
    auto res = c->res();
    auto file = new QFile(filepath);
    
    if (file->open(QFile::ReadOnly)) {
        qCDebug(C_AUTHSTATIC) << "Serving" << filepath;
        
        auto &headers = res->headers();
        
        res->setBody(file);
        QMimeType mimeType = db.mimeTypeForFile(filepath, 
                                                QMimeDatabase::MatchExtension);
        
        if (mimeType.isValid()) 
            headers.setContentType(mimeType.name());
        
        headers.setContentLength(file->size());
        
        headers.setHeader(QStringLiteral("CACHE_CONTROL"), 
                          QStringLiteral("no-cache"));

        return;
    }

    qCWarning(C_AUTHSTATIC) << "Could not serve" 
                            << filepath 
                            << file->errorString();
                            
    delete file;
}

#include "moc_authenticatedstatic.cpp"
