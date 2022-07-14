#include "SqlORMDef.h"
#include <rttr/type>
#include <rttr/registration>
#include <rttr/rttr_enable.h>

//手动注册属性方法和构造函数
RTTR_REGISTRATION
{
    rttr::registration::class_<HEventData>("HEventData")
		.constructor<>()
			.property("EVENTID", &HEventData::eventId)
			.property("SOURCENODE", &HEventData::sourceNode)
			.property("PARENTNODE", &HEventData::parentNode)
			.property("RECEIVETIME", &HEventData::receiveTime)
			.property("SOURCETIME", &HEventData::sourceTime)
			.property("TYPE", &HEventData::type)
			.property("SUBTYPE", &HEventData::subType)
			.property("SEVERITY", &HEventData::severity)
			.property("MESSAGE", &HEventData::message)
			.property("OPERATOR", &HEventData::OPERATOR)
			.property("COMMENT", &HEventData::comment)
			.property("VALUE", &HEventData::value)
			.property("VALUETYPE", &HEventData::valueType)
			.property("CONDITIONNODEID", &HEventData::conditionNodeId)
			.property("RETAIN", &HEventData::retain)
			.property("ACTIVE", &HEventData::active)
			.property("ACTIVETIME", &HEventData::activeTime)
			.property("ACKED", &HEventData::acked)
			.property("ACKEDTIME", &HEventData::ackedTime);

        rttr::registration::class_<HVarData>("HVarData")
            .constructor<>()
                .property("UNIFIEDID", &HVarData::unifiedID)
                .property("SERVERTIME", &HVarData::serverTime)
                .property("SOURCETIME", &HVarData::sourceTime)
                .property("VALUE", &HVarData::value)
                .property("QUALITY", &HVarData::quality)
                .property("TYPE", &HVarData::type);

		rttr::registration::class_<HEventIndex>("HEventIndex")
			.constructor<>()
			.property("FILENAME", &HEventIndex::fileName)
			.property("STARTTIME", &HEventIndex::startTime)
			.property("ENDTIME", &HEventIndex::endTime);

		rttr::registration::class_<HVarIndex>("HVarIndex")
			.constructor<>()
			.property("FILENAME", &HVarIndex::fileName)
			.property("STARTTIME", &HVarIndex::startTime)
			.property("ENDTIME", &HVarIndex::endTime);		

		rttr::registration::class_<HVarUnify>("HVarUnify")
			.constructor<>()
			.property("UNIFIEDID", &HVarUnify::unifiedID)
			.property("NODEID", &HVarUnify::nodeID)
			.property("VARGRPID", &HVarUnify::varGrpID);
}
