#include "ExecSql.h"

namespace HDCommon {
	void getTempType(soci::details::once_temp_type& temp) {

	}

	void getTabelNames(const SqlSessionType& sqlSessionType, std::vector<std::string>& tableNames) {
		SqlSessionPool sqlSessionPool;
		HDCommon::HSessionMgr::instance()->getSessionPool(sqlSessionType, sqlSessionPool);
		soci::connection_pool& pool = *(soci::connection_pool*)sqlSessionPool.pPool;
		soci::session sql(pool);

		try {
			sql.get_table_names(), SQL_INTO(tableNames);
		}
		catch (soci::soci_error* e)
		{
			INOV_LOG(INOLOG_LEVEL_ERROR, Module::HDCommon, "findTableName:{} error, dbname is {}",
				e->what(), sqlSessionPool.dbName);
		}	
	}

};
