/*****************************************************************//**
 * \file   HDConst.h
 * \brief  历史变量公共信息定义
 * \author 10009025
 * \date   Feb 2022
 *********************************************************************/
#pragma once
#ifndef HDCONST_H
#define HDCONST_H

//默认组id
#define DEFAULT_GROUPID (-1)
//订阅事件字段长度
#define EVENT_FIELD_LEN (19)

//sql语句集合
//查询索引库获取时间段内的所有数据库信息,时间段有重合
#define SELECT_HV_DBINFO_BY_TIME ("select * from HV_INDEX where (NOT(:eTime<STARTTIME OR :sTime>ENDTIME))")
//查询索引库获取时间段内的所有数据库信息,时间段有重合
#define SELECT_HE_DBINFO_BY_TIME ("select * from HE_INDEX where (NOT(:eTime<STARTTIME OR :sTime>ENDTIME))")
//事件数据查询
#define SELECT_HE_DATA_BY_TIME_AND_EVENTID ("select * from HE_DATA \
	where EVENTID>=:eventId AND RECEIVETIME>=:stime AND RECEIVETIME<=:etime \
	ORDER BY RECEIVETIME ASC LIMIT:queryLen")
//查询统一表通过nodeid
#define SELECT_UNIFY_BY_NODEID ("select * from UNIFY_NODEID where NODEID=:nodeid")
//查询数据表通过nodeid,时间范围，结果按照servertime升序排序
#define SELECT_HV_DATA_BY_NODEID ("select * from HV_DATA \
	where UNIFIEDID=:unifyId AND SERVERTIME>=:stime AND SERVERTIME<=:etime \
	ORDER BY SERVERTIME ASC LIMIT {}")
//通过时刻查询变量
#define SELECT_HV_DATA_BY_NODEID_TIMES ("select * from HV_DATA \
	where UNIFIEDID=:unifyId AND SERVERTIME in (:times)")

//单张表100W条数据宏
#define DATAS_TABLE_MAX (100*10000)
//索引库-索引表名
#define HV_INDEX_TABLE_NAME ("HV_INDEX")
//数据库-统一表名
#define HV_UNIFY_NODEID_TABLE_NAME ("UNIFY_NODEID")
//数据库-数据表名
#define HV_DATA_TABLE_NAME ("HV_DATA")
//索引表名
#define HE_INDEX_TABLE_NAME ("HE_INDEX")
//数据库-数据表名
#define HE_DATA_TABLE_NAME ("HE_DATA")

//插入索引表SQL语句，忽略重复插入
#define INSERT_HV_INDEX_SQL ("insert into HV_INDEX(FILENAME, STARTTIME, ENDTIME) "\
    "values(:FILENAME,:STARTTIME,:ENDTIME)")
//插入索引表SQL语句，忽略重复插入
#define INSERT_HE_INDEX_SQL ("insert into HE_INDEX(FILENAME, STARTTIME, ENDTIME) "\
    "values(:FILENAME,:STARTTIME,:ENDTIME)")

//创建统一表sql
#define CREATE_UNIFY_NODEID_TABLE_SQL ("create table UNIFY_NODEID ("\
    "UNIFIEDID integer PRIMARY KEY,"\
    "NODEID varchar(128),"\
    "VARGRPID integer"\
    ")")

//创建索引表sql
#define CREATE_HV_INDEX_TABLE_SQL ("create table HV_INDEX ("\
    "FILENAME varchar(256) PRIMARY KEY,"\
    "STARTTIME BIGINT,"\
    "ENDTIME BIGINT"\
    ")")
//创建数据表sql
#define CREATE_HV_DATA_TABLE_SQL ("create table HV_DATA ("\
    "UNIFIEDID integer,"\
    "SERVERTIME BIGINT,"\
    "SOURCETIME BIGINT,"\
    "VALUE varchar(128),"\
    "QUALITY integer,"\
    "TYPE integer,"\
    "PRIMARY KEY(UNIFIEDID, SERVERTIME)"\
	")")
//插入数据表sql语句
#define INSERT_HV_DATA_SQL ("insert into HV_DATA (UNIFIEDID,SERVERTIME,SOURCETIME,VALUE,QUALITY,TYPE) "\
    "values(:UNIFIEDID,:SERVERTIME,:SOURCETIME,:VALUE,:QUALITY,:TYPE)")
//查询数据表sql语句
#define SELECT_HV_DATA_COUNT_SQL ("select count(*) from HV_DATA")
//插入统一表sql语句
#define INSERT_UNIFY_NODEID_SQL ("insert into UNIFY_NODEID (UNIFIEDID,NODEID,VARGRPID) "\
    "values(:UNIFIEDID,:NODEID,:VARGRPID)")
//查询统一表最后一条数据UNIFIEDID值
#define SELECT_UNIFY_NODEID_LASTUNIFYID_SQL ("select * from UNIFY_NODEID order by UNIFIEDID desc LIMIT 1")
#define UPDATE_HV_INDEX_FULLDB_SQL ("update HV_INDEX set ENDTIME=:eTime where FILENAME=:dbName")
//查询统一表根据变量id数据
#define SELET_UNIFY_NODEID_BYVARGRPID ("select * from UNIFY_NODEID where VARGRPID in(:varids)")

#endif
