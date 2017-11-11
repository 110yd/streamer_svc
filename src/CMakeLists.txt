file(GLOB_RECURSE streaming_svc_SRCS *.cpp *.h)

set(streaming_svc_SRCS
    ${streaming_svc_SRCS}
    ${TEMPLATES_SRC}
)

# Create the application
add_library(streaming_svc SHARED ${streaming_svc_SRCS})

# Link to Cutelyst
target_link_libraries(streaming_svc
    Cutelyst::Core
    Cutelyst::StaticSimple
    Cutelyst::View::Grantlee
    Cutelyst::View::JSON
    Cutelyst::Utils::Sql
    Cutelyst::Session
    Cutelyst::Authentication
    Qt5::Core
    Qt5::Network
    Qt5::Sql
)

