/*****************************************************************//**
 * \file   HSessionMgr.h
 * \brief  历史存储目录管理,查询、写数据库连接池管理。
 * \description 
 * \author 10009025
 * \date   June 2022
 *********************************************************************/
#pragma once
#ifndef HSESSIONMGR_H
#define HSESSIONMGR_H
#include "HDCommon.h"
#include "SqlORMDef.h"
#include <map>
#include <memory>
#include <thread>
#include <mutex>

namespace HDCommon {
	class HDCOMMON_EXPORT HSessionMgr {
	private:
		HSessionMgr();

	public:
		~HSessionMgr();
		HSessionMgr(HSessionMgr const&) = delete;
		HSessionMgr& operator=(HSessionMgr const&) = delete;
		static HSessionMgr* instance();

		/**
		 * @brief 获取当前写数据库连接池
		 * \param sqlSessionType 数据库连接池类型
		 * \param sqlSessionPool 数据库连接池
		 */
		void getSessionPool(const SqlSessionType& sqlSessionType, SqlSessionPool& sqlSessionPool);

		/**
		 * @brief 安全定时更新数据库连接池
		 * \param dBEventType 更新机制
		 */
		void safeRefreshSessionPool(const DBEventType& dBEventType);

		/**
		 * @brief 根据查询信息获取查询数据库文件集合
		 * \param queryRequestInfo 查询请求信息
		 * \param vIndexDbNames 返回数据库文件集
		 */
		void getIndexDbNames(const HQueryRequestInfo& queryRequestInfo, std::vector<std::string>& vIndexDbNames);

		/**
		 * @brief 获取索引表数据库连接池
		 * \param idxName 索引表名
		 * \param pPool 返回数据库连接池指针
		 */
		void getQueryIdxPoolByIdxName(const std::string& idxName, void*& pPool);

		/**
		 * @brief 解析查询请求，将其切割为单元
		 * \param queryRequestInfo 查询请求信息
		 * \param hVarIndexs 变量表索引库集合
		 * \param vQueryRequestUnits 返回查询请求单元
		 */
		template<typename T>
		void parseQueryQuest(const HQueryRequestInfo& queryRequestInfo, const std::vector<T>& hVarIndexs,
			std::vector<HQueryRequestUnit>& vQueryRequestUnits) {

			int index = 0;
			for (auto& varIndex : hVarIndexs) {

				HQueryRequestUnit queryRequestUnit;
				queryRequestUnit.dbname = varIndex.fileName;
				queryRequestUnit.queryRequestInfo = queryRequestInfo;
				//stime he endtime暂时不切割
				queryRequestUnit.pool = m_querySessionPool[varIndex.fileName];

				//数组查询后面的eventid均从1开始
				if (index > 0)
					queryRequestUnit.queryRequestInfo.eventId = 1;

				vQueryRequestUnits.emplace_back(queryRequestUnit);
				++index;
			}
		}

		/**
		 * @brief 获取存储路径
		 * \param strDir 返回存储路径
		 * \param storeDirType 存储路径类型
		 */
		void getStoreDir(std::string& strDir, const HStoreDirType& storeDirType);

	private:
		bool isQueryIndexDbName(const std::string& dbName, const Ino_Int64& stime, const Ino_Int64& etime);
		//获取当天最后一天数据库名称
		void findTodayLastDB(std::string& dbName, const HStoreDirType& storeDirType);
		//更新下一条数据库文件名称XXX-00X.hdb
		void getNextDBName(std::string& dbname);

		//数据库名称补齐索引号
		void fillUpIndex2DatasDbName(std::string& dbname);

		std::string getTodayDBName(const SqlSessionType& ssType);
		//更新当前最新的写库，将旧的写库更新为读库
		void refreshRWPool(const SqlSessionType& sqlSessionType, const int& poolSize,
			const DBEventType& dBEventType);
		void initDBDir();
		void initDBFileInfo();
		void initReadPool();
		void initAllWritePool();
		void initWritePoolByType(const HStoreDirType& storeDirType);

		void refreshReadPool();
		void createSessionPool(const int& sqlSessionPer, const std::string& dbName,
			soci::connection_pool*& pool, int poolSize);

		void closeSessionPool(soci::connection_pool* pool, const int& poolSize);
		void closeAllQuerySession();
		void closeAllRWSession();
		void stop();

	private:
		//索引库全路径集合
		std::map<HStoreDirType, std::vector<std::string>> m_idxNames;
		//数据库全路径集合
		std::map<HStoreDirType, std::vector<std::string>> m_hdbNames;
		//存储路径
		std::map<HStoreDirType, std::string> m_storePath;
		//查询数据库连接池
		std::map<std::string, soci::connection_pool*> m_querySessionPool;
		//存储数据库连接池
		std::map<SqlSessionType, SqlSessionPool> m_writeSessionPool;
		bool m_exit;
		mutable std::mutex m_mut;
	};
};
#endif
