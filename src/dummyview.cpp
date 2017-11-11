#include "dummyview.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>

/// \brief Default constructor
dummyview::dummyview ( QObject* parent, const QString& name )
 : View(parent, name)
{}

/// \brief Perform render
QByteArray dummyview::render ( Cutelyst::Context* c ) const
{
    return c->response()->body();
}
