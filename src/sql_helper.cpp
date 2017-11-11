#include "sql_helper.h"
#include <QSqlRecord>

///
/// \brief Execute query and fill single result
///
/// \param query
///        Query execution result
///
/// \param target
///        Target storage
///
/// \return true on success operation
bool sql_helper::exec_single(QSqlQuery &query, QVariantMap &target) {
    if (query.exec() && query.next()) {
        int columns = query.record().count();
        QStringList cols;
        
        for (int j = 0; j < columns; ++j) {
            cols << query.record().fieldName(j);
        }

        for (int j = 0; j < columns; ++j) {
            target.insert(cols.at(j), query.value(j));
        }

        return true;
    }    
    
    return false;
}

///
/// \brief Execute query and fill table of results
///
/// \param query
///        Query execution result
///
/// \param target
///        Target storage
///
/// \return true on success operation
bool sql_helper::exec_table(QSqlQuery& query, QList<QVariantMap>& target)
{
    if (!query.exec())
        return false;
    
    int columns = query.record().count();
    QStringList cols;
            
    for (int j = 0; j < columns; ++j) {
        cols << query.record().fieldName(j);
    }
    
    while (query.next()) {
        QVariantMap entity;
        
        for (int j = 0; j < columns; ++j) {
            entity.insert(cols.at(j), query.value(j));
        }
        
        target << entity;
    }
    
    return true;
}
