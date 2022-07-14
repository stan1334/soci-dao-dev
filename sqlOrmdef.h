/******************************************************************************
 * HDRMDef.h
 * project: HVDataStore
 * Description: SOCI-ORM结构定义。
 * Creater:  duchuan 2022.2.17
 * Modifier:
 *
 ******************************************************************************/
#pragma once
#ifndef SQLORMDEF_H
#define SQLRMDEF_H
#include "HDORMDef.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/fmt/bundled/ostream.h"
#include "spdlog/fmt/bundled/ranges.h"

//SOCI包含
#ifdef INCLUDE_SOCI
#include "soci.h"
//using namespace soci;
//ORM HVarData define
namespace soci
{
	template<>
	struct soci::type_conversion<HEventData>
	{
		typedef values base_type;

		static void from_base(values const& v, indicator /* ind */, HEventData& p)
		{
			p.eventId = v.get<Ino_Int64>("EVENTID");
			p.sourceNode = v.get<std::string>("SOURCENODE");
			p.parentNode = v.get<std::string>("PARENTNODE");
			p.receiveTime = v.get<Ino_Int64>("RECEIVETIME");
			p.sourceTime = v.get<Ino_Int64>("SOURCETIME");
			p.type = v.get<int>("TYPE");
			p.subType = v.get<int>("SUBTYPE");
			p.severity = v.get<int>("SEVERITY");
			p.message = v.get<std::string>("MESSAGE");
			p.OPERATOR = v.get<std::string>("OPERATOR");
			p.comment = v.get<std::string>("COMMENT");
    		p.value = v.get<std::string>("VALUE");
	    	p.valueType = v.get<int>("VALUETYPE");
			p.conditionNodeId = v.get<std::string>("CONDITIONNODEID");
			p.retain = v.get<int>("RETAIN");
			p.active = v.get<int>("ACTIVE");
			p.activeTime = v.get<Ino_Int64>("ACTIVETIME");
			p.acked = v.get<int>("ACKED");
    		p.ackedTime = v.get<Ino_Int64>("ACKEDTIME");
		}

		static void to_base(const HEventData& p, values& v, indicator& ind)
		{
			v.set("EVENTID", p.eventId);

			v.set("SOURCENODE", p.sourceNode);
			v.set("PARENTNODE", p.parentNode);

			v.set("RECEIVETIME", p.receiveTime);
			v.set("SOURCETIME", p.sourceTime);

			v.set("TYPE", p.type);
			v.set("SUBTYPE", p.subType);
			v.set("SEVERITY", p.severity);

			v.set("MESSAGE", p.message);
			v.set("OPERATOR", p.OPERATOR);
			v.set("COMMENT", p.comment);
			v.set("VALUE", p.value);
			v.set("VALUETYPE", p.valueType);

			v.set("CONDITIONNODEID", p.conditionNodeId);
			v.set("RETAIN", p.retain);
			v.set("ACTIVE", p.active);
			v.set("ACTIVETIME", p.activeTime);
			v.set("ACKED", p.acked);
			v.set("ACKEDTIME", p.ackedTime);

			ind = i_ok;
		}
	};

	template<>
	struct type_conversion<HEventIndex>
	{
		typedef values base_type;

		static void from_base(values const& v, indicator /* ind */, HEventIndex& p)
		{
			p.fileName = v.get<std::string>("FILENAME");
			p.startTime = v.get<Ino_Int64>("STARTTIME");
			p.endTime = v.get<Ino_Int64>("ENDTIME");
		}

		static void to_base(const HEventIndex& p, values& v, indicator& ind)
		{
			v.set("FILENAME", p.fileName);
			v.set("STARTTIME", p.startTime);
			v.set("ENDTIME", p.endTime);
			ind = i_ok;
		}
	};

	template<>
	struct type_conversion<HVarIndex>
	{
		typedef values base_type;

		static void from_base(values const& v, indicator /* ind */, HVarIndex& p)
		{
			p.fileName = v.get<std::string>("FILENAME");
			p.startTime = v.get<Ino_Int64>("STARTTIME");
			p.endTime = v.get<Ino_Int64>("ENDTIME");
		}

		static void to_base(const HVarIndex& p, values& v, indicator& ind)
		{
			v.set("FILENAME", p.fileName);
			v.set("STARTTIME", p.startTime);
			v.set("ENDTIME", p.endTime);
			ind = i_ok;
		}
	};

	template<>
	struct type_conversion<HVarData>
	{
		typedef values base_type;

		static void from_base(values const& v, indicator /* ind */, HVarData& p)
		{
			p.unifiedID = v.get<int>("UNIFIEDID");
			p.serverTime = v.get<Ino_Int64>("SERVERTIME");
			p.sourceTime = v.get<Ino_Int64>("SOURCETIME");
			p.value = v.get<std::string>("VALUE");
			p.quality = v.get<int>("QUALITY");
			p.type = v.get<int>("TYPE");
		}

		static void to_base(const HVarData& p, values& v, indicator& ind)
		{
			v.set("UNIFIEDID", p.unifiedID);
			v.set("SERVERTIME", p.serverTime);
			v.set("SOURCETIME", p.sourceTime);
			v.set("VALUE", p.value);
			v.set("QUALITY", p.quality);
			v.set("TYPE", p.type);
			ind = i_ok;
		}
	};

	template<>
	struct type_conversion<HVarUnify>
	{
		typedef values base_type;

		static void from_base(values const& v, indicator /* ind */, HVarUnify& p)
		{
			p.unifiedID = v.get<int>("UNIFIEDID");
			p.nodeID = v.get<std::string>("NODEID");
			p.varGrpID = v.get<int>("VARGRPID");
		}

		static void to_base(const HVarUnify& p, values& v, indicator& ind)
		{
			v.set("UNIFIEDID", p.unifiedID);
			v.set("NODEID", p.nodeID);
			v.set("VARGRPID", p.varGrpID);
			ind = i_ok;
		}
	};
}
#endif
#endif
