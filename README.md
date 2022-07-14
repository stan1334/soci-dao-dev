为什么要做这件事
dao层主要是为了应用层/service快速简单的调用增删改查接口
SOCI支持了ORM 即支持soci::into soci::use等接口 当然也可以自定意拼接sql语句

个人觉得soci dao层封装主要是以下几个事儿
1、orm sql语句动参模板 简化操作
2、getSession封装 动态new还是缓存从池子拿根据需求来
3、select 复杂结构体解析通过rttr 遍历直接获取可操作性的数组对象 简化操作

还是两个字简单+简单

--------1、
//通过sessiontype执行sql语句
#define EXEC_SQL_BY_SESSIONTYPE(sqlSessionType, strSql, ...) HDCommon::execsqlByType(sqlSessionType, strSql, __VA_ARGS__)
//通过pool执行sql语句
#define EXEC_SQL_BY_POOL(pool, strSql, ...) HDCommon::execsql(soci::session(*(soci::connection_pool*)pool), strSql, __VA_ARGS__)


--------2、
HSessionMgr管理一堆池子 提供一个抽象接口
void getSessionPool(const SqlSessionType& sqlSessionType, SqlSessionPool& sqlSessionPool);
内部session怎么玩看需求  后台服务都需要做池子预估 前端的话由于某些操作是主观点击而非被动接受 个人感觉没必要做池子


--------3、RTTR定义

RTTR_REGISTRATION
{
    rttr::registration::class_<HEventData>("HEventData")
		.constructor<>()
			.property("EVENTID", &HEventData::eventId)
			.property("SOURCENODE", &HEventData::sourceNode)
			.property("PARENTNODE", &HEventData::parentNode)
			.property("RECEIVETIME", &HEventData::receiveTime)
			.property("SOURCETIME", &HEventData::sourceTime)
			
			
}

select动态解析

	template<typename T, class ...Args>
	void execSelectSql(const HQueryRequestUnit& queryRequestUnit, const std::string& strSql, std::vector<T>& datas, Args &&...args) {
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
						auto value = rowData.get<Int64>(prop.get_name().to_string());
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
		|	
	|

有了以上操作service侧的代码会变得相对简洁
		EXEC_SQL_BY_SESSIONTYPE(SqlSessionType::RW_HEVENTS_DATAS_SQL_CONNECT_TYPE, SELECT_MAX_EVENTID_SQL, 
			SQL_INTO(maxEventID));
			
			EXEC_SQL_BY_SESSIONTYPE(sqlSessionType, sqlGetTableSize, SQL_INTO(count));
			等等 基本都是一个宏或者一个模板接口直接获取增删改查结果
			
			
			
