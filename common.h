/******************************************************************************
 * HDCommon.h
 * project: HVDataStore
 * Description: 历史变量公共信息定义
 * Creater:  duchuan 2022.2.12
 * Modifier:
 *
 ******************************************************************************/
#pragma once
#ifndef HDCOMMON_H
#define HDCOMMON_H
#include "Os/OS_Def.h"
#include <string>
using namespace std;
//导出宏定义
#ifdef HDCOMMON_EXPORTS
#define HDCOMMON_EXPORT INOV_DLL_EXPORT
#else
#define HDCOMMON_EXPORT INOV_DLL_IMPORT
#endif
#include "uadatetime.h"
#include <ctime>
#include <vector>
#include "HDORMDef.h"

//历史数据库目录类型
typedef enum class HStoreDirType{
	HStore_HD_DIR = -1,
	HStore_HE_DIR = 0
} _HStoreDirType;

//历史请求范围
typedef enum class HQueryRequestScope {
	Query_Request_Scope_In_Pool = -1,	//连接池内数据/5天内数据
	Query_Request_Scope_Out_Pool = 0,	//连接池外数据/5天前数据
	Query_Request_Scope_Mix_Pool = 1,	//混合数据，包含5天和5天外
	Query_Request_ByTime = 2
} _HQueryRequestScope;

//历史查询请求数据分类
typedef enum class HQueryRequestDataType {
	Query_Var = -1,
	Query_Event = 0
} _HQueryRequestDataType;

//历史请求分类
typedef enum class HQueryRequestType {
	Query_Request_Original = -1,
	Query_Request_Modifyed = 0,
	Query_Request_AtTime = 1
} _HQueryRequestType;

//历史请求信息
typedef struct HQueryRequestInfo {
	Ino_Int64 eventId = -1;
	std::string nodeid;
	Ino_Int64 sTime;
	Ino_Int64 eTime;
	HQueryRequestType queryRequestType;
	HQueryRequestDataType queryRequestDataType;
	HQueryRequestScope queryRequestScope;
	bool reverse = false;
	std::vector<Ino_Int64> requestedTimes;//attime请求时间戳
}_HQueryRequestInfo;

//请求分类
typedef enum class HQueryType {
	Query_None_Time = -1,	//不需要查询时间
	Query_By_Start_Time = 0,//查询>=startime的数据
	Query_By_SE_Time = 1,	//查询起止时间
	Query_By_End_Time = 1,	//查询<=endtim的数据
}_HQueryType;

//历史请求单元
typedef struct HQueryRequestUnit{
	HQueryType queryType;
	HQueryRequestInfo queryRequestInfo;
	std::string dbname;
	void* pool;
}_HQueryRequestUnit;

//数据库会话类型
typedef enum class SqlSessionType {
	RW_HVAR_INDEX_SQL_CONNECT_TYPE = -1,	//历史变量索引库
	RW_HVAR_DATAS_SQL_CONNECT_TYPE = 0,	//历史变量数据库
	RW_HEVENTS_INDEX_SQL_CONNECT_TYPE = 1,	//历史事件索引库
	RW_HEVENTS_DATAS_SQL_CONNECT_TYPE = 2	//历史事件数据库
} _SqlSessionType;

//自定义数据库连接池
typedef struct SqlSessionPool {
	SqlSessionPool():
		pPool(nullptr),
		dbName("-1") {
	}

	std::string dbName;
	void* pPool;

	void operator=(const SqlSessionPool& s)
	{
		pPool = s.pPool;
		dbName = s.dbName;
	}

} _SqlSessionPool;

//数据库会话权限
typedef enum class SqlSessionPerType {
	SQL_Per_Write = -1,
	SQL_Per_Read = 0
} _SqlSessionPerType;

//数据库事件
typedef enum class DBEventType{
	DB_EVENT_ADD_VAR_DATAS = -1,		//增加新的变量数据库
	DB_EVENT_ADD_EVENT_DATAS = 1,	//增加新的事件数据库
	DB_EVENT_UPDATE_BY_TIMER = 2,	//更新索引库和数据库
} _DBEventType;

//数据库会话权限
#define HD_SQLITE_OPEN_READONLY         0x00000001  /* Ok for sqlite3_open_v2() */
#define HD_SQLITE_OPEN_READWRITE        0x00000002  /* Ok for sqlite3_open_v2() */
#define HD_SQLITE_OPEN_CREATE           0x00000004  /* Ok for sqlite3_open_v2() */

#endif
