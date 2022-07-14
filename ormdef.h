/******************************************************************************
 * HDRMDef.h
 * project: HVDataStore
 * Description: SOCI-ORM结构定义。
 * Creater:  duchuan 2022.2.17
 * Modifier:
 *
 ******************************************************************************/
#pragma once
#ifndef HDORMDEF_H
#define HDORMDEF_H
#include <string>
#include "Os/OS_Def.h"

//数据库ORM字段定义
struct HEventData
{
	Ino_Int64 eventId = -2;		//事件ID	LONG
	std::string sourceNode = "";//消息源节点ID	VARCHAR(256)
	std::string parentNode = "";//父对象ID	VARCHAR(256)
	Ino_Int64 receiveTime;		//消息存储时间	BIGINT
	Ino_Int64 sourceTime;		//值的数据源时间戳	BIGINT
	int type = -2;				//消息类型（系统消息、操作消息、变位消息、报警消息、SOE消息）	INTEGER
	int subType = -2;			//消息子类型（各消息的子类型如：登录、注销等）	INTEGER
	int severity = -2;			//报警级别，最大255	INTEGER
	std::string message = "";	//消息内容	TEXT
	std::string OPERATOR = "";	//操作者	TEXT
	std::string comment = "";	//操作信息	TEXT
	std::string value = "";		//事件发生时的值	TEXT
	int valueType;	//值类型	INTEGER
	std::string conditionNodeId = "";//消息源节点ID	VARCHAR(128)
	int retain;		//是否保留 BOOLEAN
	int active;		//是否活动告警 BOOLEAN
	Ino_Int64 activeTime;			//活动时间 BIGINT
	int acked;		//是否确认 BOOLEAN
	Ino_Int64 ackedTime;			//确认时间 BIGINT

	template<typename OStream>
	friend OStream& operator<<(OStream& os, const HEventData& c){
		return os << "HEventData[eventId:" << c.eventId << " sourceNode:" << c.sourceNode
			<< " parentNode:" << c.parentNode << " receiveTime:" << c.receiveTime << " sourceTime:" << c.sourceTime << " type:" << c.type << " subType:" << c.subType << " severity:" << c.severity <<
			" message:" << c.message << " OPERATOR:" << c.OPERATOR << " comment:" << c.comment<<
			" value:" << c.value << " valueType:" << c.valueType << " conditionNodeId:" << c.conditionNodeId<<
			" retain:"<< c.retain << " active:" << c.active << " activeTime:" << c.activeTime << 
			" acked:" << c.acked << " ackedTime:" << c.ackedTime << "]";
	}

	bool operator < (const HEventData& hEventData) const {
		return receiveTime < hEventData.receiveTime;
	}

	bool operator > (const HEventData& hEventData) const {
		return receiveTime > hEventData.receiveTime;
	}
};

struct HEventIndex
{
	std::string fileName = "";	//文件名称	TEXT
	Ino_Int64 startTime = -2;		// 记录开始时间	INTEGER
	Ino_Int64 endTime = -2;		//记录结束时间	INTEGER
};

//历史变量索引表HV_INDEX表结构
struct HVarIndex
{
	std::string fileName = "";	//文件名称	TEXT
	Ino_Int64 startTime = -2;		// 记录开始时间	INTEGER
	Ino_Int64 endTime = -2;		//记录结束时间	INTEGER
};

//表HV_DATA表结构
struct HVarData
{
	int unifiedID = -1;     // NodeID统一映射ID   INTEGER
	Ino_Int64 serverTime = 0;    // 值的数据源服务器戳   INTEGER
	Ino_Int64 sourceTime = 0;    // 值的数据源时间戳，原始时间戳 INTEGER
	std::string value = ""; // 值数据    TEXT
	int quality = -1;       // 值质量    INTEGER
	int type = -1;          // 值类型    INTEGER
	std::string nodeid = "";		//非数据库数据结构
	int varGrpID;			//变量表ID	INTEGER

	template <typename OStream>
	friend OStream& operator<<(OStream& os, const HVarData& c) {
		return os << "HVarData[unifiedID:" << c.unifiedID << " serverTime:" << c.serverTime
			<< " sourceTime:" << c.sourceTime << " value:" << c.value << " quality:" << c.quality << " type:" << c.type
			<< " nodeid:" << c.nodeid << " varGrpID:" << c.varGrpID << "]";
	}

	bool operator < (const HVarData& hVarData) const{
		return serverTime < hVarData.serverTime;
	}

	bool operator > (const HVarData& hVarData) const{
		return serverTime > hVarData.serverTime;
	}
};

//历史变量统一表UNIFY_NODEID表
struct HVarUnify
{
	int unifiedID = -1;	//NodeID统一映射ID	INTEGER
	std::string nodeID = "";	//点标识，格式为：ns = 1012; s = Moto_1.State.PV	TEXT
	int varGrpID = -2;	//变量表ID	INTEGER

	template<typename OStream>
	friend OStream& operator<<(OStream& os, const HVarUnify& c)
	{
		return os << "[HVarUnify unifiedID=" << c.unifiedID << "]";
	}

	//查找结构体e是否存在
	bool operator == (const HVarUnify& e) {
		return (this->varGrpID == e.varGrpID) && (0 == this->nodeID.compare(e.nodeID));
	}
	
};

#endif
