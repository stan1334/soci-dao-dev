/*****************************************************************//**
 * \file   ExecSql.h
 * \brief  C++/SOCI dao层封装，统一执行ORM sql语句。
 * \author 10009025
 * \date   June 2022
 *********************************************************************/
#pragma once
#ifndef EXECSQL_H
#define EXECSQL_H
#include <string>
#include "Public/Macro_Def.h"
#include "HDCommon.h"
#include "HDORMDef.h"
#include "HSessionMgr.h"
#include <queue>
#include <vector>
#include <rttr/type>
#include "rttr/variant.h"
#include <iostream>
#include <rttr/registration>
using namespace rttr;

namespace HDCommon {
#define SQL_USE(param) soci::use(param)
#define SQL_INTO(param) soci::into(param)

	/**
	 * @brief 递归结束
	 * \param temp
	 */
	template <typename T>
	void getTempType(T& temp) {
	
	}

	template <typename T1, typename T>
	void getTempType(T1& srcTemp, const T& t) {
		srcTemp, t;
	}

	/**
	 * @brief 递归构造动态SOCI TempType
	 * \param temp 构造源
	 * \param t 构造参数
	 * \param ...rest 构造动参
	 */
	template <typename T1, typename T, typename...Args>
	void getTempType(T1& srcTemp, const T& t, const Args&...rest) {
		srcTemp, t;
		getTempType(srcTemp, rest...);       // 递归调用
	}

	/**
	 * @brief 执行sql语句
	 * \param sql
	 * \param strSql
	 * \param ...args
	 */
	template <class ...Args>
	void execsql(soci::session& sql, const std::string& strSql, Args &&...args) {
		soci::details::once_temp_type temp(sql);
		getTempType(temp, args...);
		temp << strSql;
//		temp << strSql, temp;
	}

	template <class ...Args>
	void execsqlByType(const SqlSessionType& sqlSessionType, const std::string& strSql, Args &&...args) {
		SqlSessionPool sqlSessionPool;
		HDCommon::HSessionMgr::instance()->getSessionPool(sqlSessionType, sqlSessionPool);
		soci::connection_pool& pool = *(soci::connection_pool*)sqlSessionPool.pPool;
		soci::session sql(pool);
	
		if (!sql.is_connected()){
			INOV_LOG(INOLOG_LEVEL_ERROR, Module::HDCommon, "execsqlByType sql session isnot connect {}， dbname {}", 
				strSql,
				sqlSessionPool.dbName);
			return;
		}

		try {
			execsql(sql, strSql, args...);
			INOV_LOG(INOLOG_LEVEL_DEBUG, Module::HDCommon, "execsqlByType Success sqlStr is {}, dbname is {}",
				strSql, sqlSessionPool.dbName);
		}
		catch (soci::soci_error const& e) {
			INOV_LOG(INOLOG_LEVEL_ERROR, Module::HDCommon, "execsqlByType error sqlStr is {}, what is {}, dbname is {}",
				strSql, e.what(), sqlSessionPool.dbName);
		}
	}

	/**
	 * @brief 通过sessiontype插入队列数据模板函数
	 * \param sqlSessionType 数据库连接池类型
	 * \param strSql sql语句
	 * \param datas 插入队列数据
	 */
	/*
	template<typename T>
	void insertQueueBySt(const SqlSessionType& sqlSessionType, const std::string& strSql, std::queue<T>& datas) {
		SqlSessionPool sqlSessionPool;
		HDCommon::HSessionMgr::instance()->getSessionPool(sqlSessionType, sqlSessionPool);
		soci::connection_pool& pool = *(soci::connection_pool*)sqlSessionPool.pPool;
		soci::session sql(pool);

		try {
			soci::transaction tr(sql);
			T insertData;
			soci::statement st = (sql.prepare << strSql, SQL_USE(insertData));

			while (0 != datas.size()) {
				insertData = datas.front();
				datas.pop();
				TEST_INOV_LOG(INOLOG_LEVEL_DEBUG, Module::HDCommon, "Insert last EventData {}", insertData);
				st.execute(true);
			}
			tr.commit();
		}
		catch (soci::soci_error const& e) {
			TEST_INOV_LOG(INOLOG_LEVEL_ERROR, Module::HDCommon, "insertQueueBySt Failed sqlStr is {}, what is {}, dbname is {}",
				strSql, e.what(), sqlSessionPool.dbName);
		}
	}
	*/

	/**
	 * @brief 通过sessiontype插入数组数据模板函数
	 * \param sqlSessionType 数据库连接池类型
	 * \param strSql sql语句
	 * \param datas 插入数据数组
	 */
	template<typename T>
	void insertVectorBySt(const SqlSessionType& sqlSessionType, const std::string& strSql, const std::vector<T>& datas) {
		SqlSessionPool sqlSessionPool;
		HDCommon::HSessionMgr::instance()->getSessionPool(sqlSessionType, sqlSessionPool);
		soci::connection_pool& pool = *(soci::connection_pool*)sqlSessionPool.pPool;
		soci::session sql(pool);

		try {
			soci::transaction tr(sql);
			T insertData;
			soci::statement st = (sql.prepare << strSql, SQL_USE(insertData));

			for (auto& data : datas) {
				insertData = data;
				INOV_LOG(INOLOG_LEVEL_DEBUG, Module::HDCommon, "Insert last VarData {}", insertData);
				st.execute(true);
			}
			tr.commit();
		}
		catch (soci::soci_error const& e) {
			INOV_LOG(INOLOG_LEVEL_ERROR, Module::HDCommon, "insertVectorBySt Failed sqlStr is {}, what is {}, dbname is {}",
				strSql, e.what(), sqlSessionPool.dbName);
		}
	}
	/**
	 * @brief 获取数据库所有表名信息
	 * \param sqlSessionType 数据库连接池类型
	 * \param tableNames 返回表名数组
	 */
	HDCOMMON_EXPORT void getTabelNames(const SqlSessionType& sqlSessionType, std::vector<std::string>& tableNames);

	/**
	 * @brief 执行查询操作
	 * \param queryRequestUnit 查询单元
	 * \param strSql 查询sql语句
	 * \param datas 查询结果
	 * \param ...args 查询orm拼接
	 */
	template<typename T, class ...Args>
	void execSelectSql(const HQueryRequestUnit& queryRequestUnit, const std::string& strSql, std::vector<T>& datas, Args &&...args) {
		soci::connection_pool& pool = *(soci::connection_pool*)queryRequestUnit.pool;
		soci::session sql(pool);

		try {
			soci::details::prepare_temp_type temp(sql);
			getTempType(temp, args...);
			temp = (sql.prepare << strSql, temp);
			soci::rowset<soci::row>rs = temp;

			for (const soci::row& rowData : rs) {
				T obj;
				rttr::type t = rttr::type::get<T>();
				for (auto prop : t.get_properties()) {
					std::string strType = prop.get_type().get_name().to_string().c_str();
					/*
					std::cout << "name:" << prop.get_name().to_string()
						<< "type:" << strType << std::endl;
					*/

					if (0 == strcmp(strType.c_str(), "__int")||
						0 == strcmp(strType.c_str(), "int"))
					{
						auto value = rowData.get<int>(prop.get_name().to_string());
						prop.set_value(obj, value);
					}
					else if (0 == strcmp(strType.c_str(), "__int64"))
					{
						auto value = rowData.get<Ino_Int64>(prop.get_name().to_string());
						prop.set_value(obj, value);
					}
					else if (0 == strcmp(strType.c_str(), "std::string"))
					{
						auto value = rowData.get<std::string>(prop.get_name().to_string());
						prop.set_value(obj, value);
					}
				}
				datas.emplace_back(obj);
			}			
		}
		catch (soci::soci_error const& e) {
			INOV_LOG(INOLOG_LEVEL_ERROR, Module::HDCommon, "execSelectSql Failed sqlStr is {}, what is {}, dbname is {}",
				strSql, 
				e.what(), 
				queryRequestUnit.dbname);
		}
	}

//通过sessiontype执行sql语句
#define EXEC_SQL_BY_SESSIONTYPE(sqlSessionType, strSql, ...) HDCommon::execsqlByType(sqlSessionType, strSql, __VA_ARGS__)
//通过pool执行sql语句
#define EXEC_SQL_BY_POOL(pool, strSql, ...) HDCommon::execsql(soci::session(*(soci::connection_pool*)pool), strSql, __VA_ARGS__)

};
#endif
