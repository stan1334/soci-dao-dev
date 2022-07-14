#include "HSessionMgr.h"
#include "FileSys/Filesystem.h"
#include "Public/Macro_Def.h"
#include "HDEvent.h"
#include <time.h>

#define HD_PREFIX ("HD-")
#define HE_PREFIX ("HE-")
#define IDX_FILE (".idx") 
#define HDB_FILE (".hdb") 
#define READ_DATAS_SQL_SESSION_POOL (2)
#define READ_INDEX_SQL_SESSION_POOL (1)
#define WRITE_VAR_DATAS_SQL_SESSION_POOL (3)
#define WRITE_VAR_INDEX_SQL_SESSION_POOL (1)
#define WRITE_EVENTS_DATAS_SQL_SESSION_POOL (3)
#define WRITE_EVENTS_INDEX_SQL_SESSION_POOL (1)
//数据库会话字符串参数列表 db文件全路径 权限 用户 密码 是否共享缓存 超时连接
#define DB_FORMATTER ("dbname={} user=INO-FA password=123456 shared_cache=false")
#define READONLY_DB_FORMATTER ("dbname={} readonly user=INO-FA password=123456 shared_cache=false")
//#define READONLY_DB_FORMATTER ("dbname={} readonly user=scott password=tiger shared_cache=false")
//"service=orcl user=scott password=tiger");
//ODBC数据库类型
#define ODBC_SQLITE3_TYPE ("sqlite3")
#define ODBC_MYSQL_TYPE ("mysql")
#define DB_NAME_FIRST_INDEX ("_001")
#define QUERY_POOL_DAYS (60*60*24*5) //查询连接池范围单位:s
#define ONEDAY_S (60*60*24) //一天时间按照秒计算

void stringToTimeStamp(const std::string& daytime, time_t& t) {
	struct tm tm_;
	int year, month, day;
	sscanf_s(daytime.c_str(), "%d-%d-%d", &year, &month, &day);
	tm_.tm_year = year - 1900;
	tm_.tm_mon = month - 1;
	tm_.tm_mday = day;
	tm_.tm_hour = 0;
	tm_.tm_min = 0;
	tm_.tm_sec = 0;
	tm_.tm_isdst = 0;

	//	time = std::chrono::duration_cast<std::chrono::milliseconds>(tm_.time_since_epoch());
	t = mktime(&tm_);
	return;
}

void getTodayYMD(std::string& today) {
	time_t tt = time(0);
	struct tm stm;
	localtime_threadsafe(&tt, &stm);
	today = fmt::format("{:04d}-{:02d}-{:02d}", 1900 + stm.tm_year, 1 + stm.tm_mon, stm.tm_mday);
	return;
}

bool isQueryScope(std::string day) {
	bool inScope = false;
	time_t start_t;
	stringToTimeStamp(day, start_t);

	time_t today_t = time(0);
	struct tm stm;
	localtime_threadsafe(&today_t, &stm);
	stm.tm_hour = 0;
	stm.tm_min = 0;
	stm.tm_sec = 0;
	stm.tm_isdst = 0;

	Ino_Int64 distanceTime = (Ino_Int64)difftime(today_t, start_t);

	if (distanceTime <= QUERY_POOL_DAYS)
	{
		inScope = true;
	}

	return inScope;
}

namespace HDCommon {
	HSessionMgr::HSessionMgr() :
		m_exit(false) {
		initDBDir();
		initAllWritePool();
		initDBFileInfo();
		initReadPool();
	}

	HSessionMgr::~HSessionMgr() {
		stop();
	}

	HSessionMgr* HSessionMgr::instance() {
		static HSessionMgr s_HSessionMgr;
		return &s_HSessionMgr;
	}

	void HSessionMgr::initDBDir() {
		//HD目录创建
		std::string binPath = INO_CM::InoFilesystem::instance()->getBinPath("HDService");
		m_storePath[HStoreDirType::HStore_HD_DIR] = binPath + "/HD/";
		m_storePath[HStoreDirType::HStore_HE_DIR] = binPath + "/HE/";

		if (!INO_CM::InoFilesystem::instance()->isExist(m_storePath[HStoreDirType::HStore_HD_DIR]))
			INO_CM::InoFilesystem::instance()->mkDir(m_storePath[HStoreDirType::HStore_HD_DIR]);

		if (!INO_CM::InoFilesystem::instance()->isExist(m_storePath[HStoreDirType::HStore_HE_DIR]))
			INO_CM::InoFilesystem::instance()->mkDir(m_storePath[HStoreDirType::HStore_HE_DIR]);
	}

	void HSessionMgr::initDBFileInfo() {
		m_hdbNames.clear();
		m_idxNames.clear();

		HStoreDirType hdDir = HStoreDirType::HStore_HD_DIR;
		INO_CM::InoFilesystem::getFilesOfDir(m_storePath[hdDir],
			m_idxNames[hdDir], IDX_FILE);

		INO_CM::InoFilesystem::getFilesOfDir(m_storePath[hdDir],
			m_hdbNames[hdDir], HDB_FILE);

		HStoreDirType heDir = HStoreDirType::HStore_HE_DIR;
		INO_CM::InoFilesystem::getFilesOfDir(m_storePath[heDir],
			m_idxNames[heDir], IDX_FILE);

		INO_CM::InoFilesystem::getFilesOfDir(m_storePath[heDir],
			m_hdbNames[heDir], HDB_FILE);

		//只保留固定天数的
	//	QUERY_POOL_DAYS
		for (vector<std::string>::iterator it = m_hdbNames[hdDir].begin(); it != m_hdbNames[hdDir].end();) {
			std::string hdDbName = *it;
			std::string::size_type iPos = hdDbName.find_last_of('/') + 1;
			hdDbName = hdDbName.substr(iPos + 3, 10);

			if (!isQueryScope(hdDbName)) {
				it = m_hdbNames[hdDir].erase(it);
			}
			else {
				it++;
			}
		}

		for (vector<std::string>::iterator it = m_idxNames[hdDir].begin(); it != m_idxNames[hdDir].end();) {
			std::string hdIdxName = *it;
			std::string::size_type iPos = hdIdxName.find_last_of('/') + 1;
			hdIdxName = hdIdxName.substr(iPos + 3, 10);

			if (!isQueryScope(hdIdxName)) {
				it = m_idxNames[hdDir].erase(it);
			}
			else {
				it++;
			}
		}
	}

	void HSessionMgr::createSessionPool(const int& sqlSessionPer, const std::string& dbName,
		soci::connection_pool*& pool, int poolSize) {
		pool = new soci::connection_pool(poolSize);

		for (size_t i = 0; i != poolSize; ++i)
		{
			soci::session& sql = pool->at(i);

			if (sql.is_connected())
				sql.close();

			std::string sessionFmt = READONLY_DB_FORMATTER;
			if (HD_SQLITE_OPEN_READONLY < sqlSessionPer)
			{
				sessionFmt = DB_FORMATTER;
			}

			std::string sessionStr = fmt::format(sessionFmt, fmt::format("\"{}\"", dbName));
			sql.open(ODBC_SQLITE3_TYPE, sessionStr);

			if (!sql.is_connected())
				INOV_LOG(INOLOG_LEVEL_ERROR, Module::HDCommon, "ERR HSessionMgr::createSessionPool session str{}", sessionStr);
		}
	}

	std::string HSessionMgr::getTodayDBName(const SqlSessionType& ssType) {
		std::string today;
		HStoreDirType storeDir = HStoreDirType::HStore_HD_DIR;

		if (SqlSessionType::RW_HEVENTS_INDEX_SQL_CONNECT_TYPE <= ssType)
			storeDir = HStoreDirType::HStore_HE_DIR;

		getTodayYMD(today);
		today = HD_PREFIX + today;
		std::string dbName = m_storePath[storeDir] + today;
		std::string writeDbDatasName = m_storePath[storeDir] + today;
		std::string suffix = HDB_FILE;

		if (SqlSessionType::RW_HEVENTS_INDEX_SQL_CONNECT_TYPE == ssType ||
			SqlSessionType::RW_HVAR_INDEX_SQL_CONNECT_TYPE == ssType)
			suffix = IDX_FILE;

		if (HStoreDirType::HStore_HE_DIR == storeDir){
			getTodayYMD(today);
			today = HE_PREFIX + today;
			dbName = m_storePath[storeDir] + today;
		}

		dbName += suffix;
		return dbName;
	}

	void HSessionMgr::initWritePoolByType(const HStoreDirType& storeDirType) {

		SqlSessionType indexType = SqlSessionType::RW_HVAR_INDEX_SQL_CONNECT_TYPE;
		SqlSessionType datasType = SqlSessionType::RW_HVAR_DATAS_SQL_CONNECT_TYPE;

		if (HStoreDirType::HStore_HE_DIR == storeDirType){
			indexType = SqlSessionType::RW_HEVENTS_INDEX_SQL_CONNECT_TYPE;
			datasType = SqlSessionType::RW_HEVENTS_DATAS_SQL_CONNECT_TYPE;
		}

		std::string writeDbIndexName = getTodayDBName(indexType);
		std::string writeDbDatasName = getTodayDBName(datasType);

		std::string writeDbDatasNameToday = "";
		findTodayLastDB(writeDbDatasNameToday, storeDirType);

		if (writeDbDatasNameToday.empty()) {
			//数据库名称添加索引号
			fillUpIndex2DatasDbName(writeDbDatasName);
		}
		else {
			writeDbDatasName = writeDbDatasNameToday;
		}

		int sqlSessionPer = HD_SQLITE_OPEN_CREATE | HD_SQLITE_OPEN_READWRITE;
		createSessionPool(sqlSessionPer, writeDbIndexName,
			(soci::connection_pool*&)m_writeSessionPool[indexType].pPool,
			WRITE_VAR_INDEX_SQL_SESSION_POOL);
		m_writeSessionPool[indexType].dbName = writeDbIndexName;

		createSessionPool(sqlSessionPer, writeDbDatasName,
			(soci::connection_pool*&)m_writeSessionPool[datasType].pPool,
			WRITE_VAR_DATAS_SQL_SESSION_POOL);
		m_writeSessionPool[datasType].dbName = writeDbDatasName;

		if (HStoreDirType::HStore_HE_DIR == storeDirType)
		{
			createSessionPool(sqlSessionPer, writeDbIndexName,
				(soci::connection_pool*&)m_writeSessionPool[indexType].pPool,
				WRITE_EVENTS_INDEX_SQL_SESSION_POOL);
			createSessionPool(sqlSessionPer, writeDbDatasName,
				(soci::connection_pool*&)m_writeSessionPool[datasType].pPool,
				WRITE_EVENTS_DATAS_SQL_SESSION_POOL);
		}
	}

	void HSessionMgr::findTodayLastDB(std::string& dbName, const HStoreDirType& storeDirType) {
		std::vector<std::string> dbnames = m_hdbNames[storeDirType];
		//逆序
		std::sort(dbnames.rbegin(), dbnames.rend());
		std::string today;
		getTodayYMD(today);

		auto itFind = std::find_if(dbnames.begin(), dbnames.end(),
			[today, &dbName](const std::string& t)
			{
				Ino_Int64 bfind = t.find(today);
				if (-1 < bfind) {
					dbName = t;
					return 1;
				}
				else
					return 0;
			});
	}

	void HSessionMgr::initAllWritePool() {
		initWritePoolByType(HStoreDirType::HStore_HD_DIR);
		initWritePoolByType(HStoreDirType::HStore_HE_DIR);
	}

	void HSessionMgr::refreshRWPool(const SqlSessionType& sqlSessionType, const int& poolSize, const DBEventType& dBEventType) {
		std::string dbName = "";

		for (auto& itSession : m_writeSessionPool) {
			if (itSession.first != sqlSessionType)
			{
				continue;
			}

			soci::connection_pool& pool = *(soci::connection_pool*)(itSession.second.pPool);
			dbName = itSession.second.dbName;

			if (DBEventType::DB_EVENT_UPDATE_BY_TIMER == dBEventType){
				dbName = getTodayDBName(sqlSessionType);

				if (SqlSessionType::RW_HEVENTS_DATAS_SQL_CONNECT_TYPE == sqlSessionType||
					SqlSessionType::RW_HVAR_DATAS_SQL_CONNECT_TYPE == sqlSessionType)
					fillUpIndex2DatasDbName(dbName);
			}
			else {
				getNextDBName(dbName);
			}

			for (size_t i = 0; i != poolSize; ++i) {
				soci::session sql(pool);
				//关闭旧的数据库连接
				if (sql.is_connected())
					sql.close();

				//打开新数据库连接
				std::string sessionStr = fmt::format(DB_FORMATTER, fmt::format("\"{}\"", dbName));

				try{
					sql.open(ODBC_SQLITE3_TYPE, sessionStr);
					sql.reconnect();

					if (!sql.is_connected())
						INOV_LOG(INOLOG_LEVEL_ERROR, Module::HDCommon, "HSessionMgr::refreshRWPool session {},connnect err", sessionStr);
				}
				catch (std::exception* e)
				{
					INOV_LOG(INOLOG_LEVEL_ERROR, Module::HDCommon, "HSessionMgr::refreshRWPool session open session {},err {}",
						sessionStr,
						e->what());
				}
			}
			m_writeSessionPool[sqlSessionType].dbName = dbName;
		}

		//增加读数据库
		int sqlSessionPer = HD_SQLITE_OPEN_READONLY;
		if (m_querySessionPool.find(dbName) == m_querySessionPool.end()) {
			createSessionPool(sqlSessionPer, dbName,
				m_querySessionPool[dbName], READ_DATAS_SQL_SESSION_POOL);
		}
	}

	void HSessionMgr::fillUpIndex2DatasDbName(std::string& dbname) {
		size_t bfind = dbname.find('_');
		std::string indexStr = DB_NAME_FIRST_INDEX;

		if (-1 == bfind)
		{
			size_t index = dbname.find_last_of('.');
			dbname = dbname.insert(index, indexStr);
		}
	}

	void HSessionMgr::getNextDBName(std::string& dbname) {
		std::string tempDbName = dbname;
		int find = tempDbName.find('_');
		int separator = tempDbName.find_last_of('.');
		std::string indexStr = "_001";
		int index = -1;

		if (-1 == find)	{
			index = 1;
		}
		else {
			indexStr = tempDbName.substr(find + 1, 3);
			index = std::stoi(indexStr);
			++index;
		}

		indexStr = fmt::format("{:03d}", index);
		dbname = tempDbName.substr(0, tempDbName.length() - 7) + indexStr;
		dbname += tempDbName.substr(separator, 4);
	}

	void HSessionMgr::safeRefreshSessionPool(const DBEventType& dBEventType) {
		std::lock_guard<std::mutex>lk(m_mut);

		//新增变量库
		if (DBEventType::DB_EVENT_ADD_VAR_DATAS == dBEventType) {
			refreshRWPool(SqlSessionType::RW_HVAR_DATAS_SQL_CONNECT_TYPE, 
				WRITE_VAR_DATAS_SQL_SESSION_POOL,
				dBEventType);
		}
		//新增事件库
		else if (DBEventType::DB_EVENT_ADD_EVENT_DATAS == dBEventType) {
			refreshRWPool(SqlSessionType::RW_HEVENTS_DATAS_SQL_CONNECT_TYPE, 
				WRITE_EVENTS_DATAS_SQL_SESSION_POOL,
				dBEventType);
		}
		//每天凌晨刷新库
		else if (DBEventType::DB_EVENT_UPDATE_BY_TIMER == dBEventType) {
			INOV_LOG(INOLOG_LEVEL_INFO, Module::HDCommon, "refresh SQl Session Byday", "beging");
			refreshRWPool(SqlSessionType::RW_HVAR_INDEX_SQL_CONNECT_TYPE,
				WRITE_VAR_INDEX_SQL_SESSION_POOL,
				dBEventType);

			refreshRWPool(SqlSessionType::RW_HVAR_DATAS_SQL_CONNECT_TYPE,
				WRITE_VAR_DATAS_SQL_SESSION_POOL,
				dBEventType);

			refreshRWPool(SqlSessionType::RW_HEVENTS_INDEX_SQL_CONNECT_TYPE,
				WRITE_EVENTS_INDEX_SQL_SESSION_POOL,
				dBEventType);

			refreshRWPool(SqlSessionType::RW_HEVENTS_DATAS_SQL_CONNECT_TYPE,
				WRITE_EVENTS_DATAS_SQL_SESSION_POOL,
				dBEventType);

			//刷新查询连接池
			initDBFileInfo();
			refreshReadPool();
			INOV_LOG(INOLOG_LEVEL_INFO, Module::HDCommon, "refresh SQl Session Byday", "end");
		}
	}

	void HSessionMgr::refreshReadPool() {
		//删除过期连接池
		for (auto it = m_querySessionPool.begin(); it != m_querySessionPool.end();) {
			if (!isQueryScope(it->first)) {
				int poolsize = READ_INDEX_SQL_SESSION_POOL;
				closeSessionPool(it->second, poolsize);
				it = m_querySessionPool.erase(it);
			}
			else {
				it++;
			}
		}

		int sqlSessionPer = HD_SQLITE_OPEN_READONLY;
		std::string varIndexDbName = m_writeSessionPool[SqlSessionType::RW_HVAR_INDEX_SQL_CONNECT_TYPE].dbName;
		std::string varDatasDbName = m_writeSessionPool[SqlSessionType::RW_HVAR_DATAS_SQL_CONNECT_TYPE].dbName;
		std::string eventIndexDbName = m_writeSessionPool[SqlSessionType::RW_HEVENTS_INDEX_SQL_CONNECT_TYPE].dbName;
		std::string eventDatasDbName = m_writeSessionPool[SqlSessionType::RW_HEVENTS_DATAS_SQL_CONNECT_TYPE].dbName;

		createSessionPool(sqlSessionPer, varDatasDbName,
			m_querySessionPool[varIndexDbName], READ_INDEX_SQL_SESSION_POOL);

		createSessionPool(sqlSessionPer, varIndexDbName,
			m_querySessionPool[varDatasDbName], READ_DATAS_SQL_SESSION_POOL);

		createSessionPool(sqlSessionPer, eventIndexDbName,
			m_querySessionPool[eventIndexDbName], READ_INDEX_SQL_SESSION_POOL);

		createSessionPool(sqlSessionPer, eventDatasDbName,
			m_querySessionPool[eventDatasDbName], READ_DATAS_SQL_SESSION_POOL);
	}

	void HSessionMgr::initReadPool() {
		int sqlSessionPer = HD_SQLITE_OPEN_READONLY;

		for (auto& dbName : m_hdbNames[HStoreDirType::HStore_HD_DIR])
			createSessionPool(sqlSessionPer, dbName,
				m_querySessionPool[dbName], READ_DATAS_SQL_SESSION_POOL);

		for (auto& dbName : m_idxNames[HStoreDirType::HStore_HD_DIR])
			createSessionPool(sqlSessionPer, dbName,
				m_querySessionPool[dbName], READ_INDEX_SQL_SESSION_POOL);

		for (auto& dbName : m_hdbNames[HStoreDirType::HStore_HE_DIR])
			createSessionPool(sqlSessionPer, dbName,
				m_querySessionPool[dbName], READ_DATAS_SQL_SESSION_POOL);

		for (auto& dbName : m_idxNames[HStoreDirType::HStore_HE_DIR])
			createSessionPool(sqlSessionPer, dbName,
				m_querySessionPool[dbName], READ_INDEX_SQL_SESSION_POOL);
	}

	void HSessionMgr::closeSessionPool(soci::connection_pool* pool, const int& poolSize) {
		for (size_t i = 0; i != poolSize; ++i) {
			soci::session& sql = pool->at(i);

			if (sql.is_connected())
				sql.close();

			if (sql.is_connected())
				INOV_LOG(INOLOG_LEVEL_DEBUG, Module::HDCommon, "HSessionMgr", "closeSessionPool err,poolSize is {}", poolSize);
		}

		SAFE_DELETE(pool);
	}

	void HSessionMgr::closeAllQuerySession() {
		for (auto& pool : m_querySessionPool) {
			int poolsize = READ_INDEX_SQL_SESSION_POOL;

			if (-1 < pool.first.find(".hdb"))
				poolsize = READ_DATAS_SQL_SESSION_POOL;

			closeSessionPool(pool.second, poolsize);
		}
	}

	void HSessionMgr::closeAllRWSession() {
		for (auto& pool : m_writeSessionPool) {
			int poolsize = WRITE_VAR_INDEX_SQL_SESSION_POOL;
			SqlSessionType sqlSessionType = pool.first;

			if (SqlSessionType::RW_HVAR_INDEX_SQL_CONNECT_TYPE == sqlSessionType) {
				poolsize = WRITE_VAR_INDEX_SQL_SESSION_POOL;
			}
			else if (SqlSessionType::RW_HVAR_DATAS_SQL_CONNECT_TYPE == sqlSessionType)
			{
				poolsize = WRITE_VAR_DATAS_SQL_SESSION_POOL;
			}
			else if (SqlSessionType::RW_HEVENTS_INDEX_SQL_CONNECT_TYPE == sqlSessionType)
			{
				poolsize = WRITE_EVENTS_INDEX_SQL_SESSION_POOL;
			}
			else if (SqlSessionType::RW_HEVENTS_DATAS_SQL_CONNECT_TYPE == sqlSessionType)
			{
				poolsize = WRITE_EVENTS_DATAS_SQL_SESSION_POOL;
			}

			closeSessionPool((soci::connection_pool*)pool.second.pPool, poolsize);
		}
	}

	void HSessionMgr::stop() {
		std::lock_guard<std::mutex>lk(m_mut);
		closeAllQuerySession();
		closeAllRWSession();
	}

	void HSessionMgr::getQueryIdxPoolByIdxName(const std::string& idxName, void*& pPool) {
		pPool = m_querySessionPool[idxName];
	}

	void HSessionMgr::getSessionPool(const SqlSessionType& sqlSessionType, SqlSessionPool& sqlSessionPool) {
		std::lock_guard<std::mutex>lk(m_mut);

		for (auto& it : m_writeSessionPool) {
			if (it.first == sqlSessionType) {
				sqlSessionPool = it.second;
				break;
			}
		}

		return;
	}

	bool HSessionMgr::isQueryIndexDbName(const std::string& idxName, const Ino_Int64& stime, const Ino_Int64& etime) {
		std::string::size_type iPos = idxName.find_last_of('/') + 1;
		std::string temp = idxName.substr(iPos + 3, 10);
		Ino_Int64 idxTime = -1;
		stringToTimeStamp(temp, idxTime);
		bool bVaildIndeDbName = false;
		time_t stime_t = UaDateTime(stime).toTime_t();
		time_t etime_t = UaDateTime(etime).toTime_t();
		time_t onedayStime = stime_t - ONEDAY_S;

		//起始时间1天一直到结束事前之前的索引库都需要查询
		if (idxTime >= onedayStime && idxTime < etime_t) {
			bVaildIndeDbName = true;
		}

		return bVaildIndeDbName;
	}

	void HSessionMgr::getStoreDir(std::string& strDir, const HStoreDirType& storeDirType) {
		strDir = m_storePath[storeDirType];
	}

	void HSessionMgr::getIndexDbNames(const HQueryRequestInfo& queryRequestInfo, std::vector<std::string>& vIndexDbNames) {
		HStoreDirType hStoreDirType = HStoreDirType::HStore_HD_DIR;
		if (HQueryRequestDataType::Query_Event == queryRequestInfo.queryRequestDataType)
			hStoreDirType = HStoreDirType::HStore_HE_DIR;

		std::vector<std::string> vFindNames(m_idxNames[hStoreDirType]);
		std::string startDay = HD_PREFIX + std::string(UaDateTime(queryRequestInfo.sTime).toDateString().toUtf8());
		std::string endDay = HD_PREFIX + std::string((UaDateTime(queryRequestInfo.eTime)).toDateString().toUtf8());

		for (auto& idxName : vFindNames) {
			if (isQueryIndexDbName(idxName, queryRequestInfo.sTime, queryRequestInfo.eTime)) {
				vIndexDbNames.emplace_back(idxName);
			}
		}
	}
}
