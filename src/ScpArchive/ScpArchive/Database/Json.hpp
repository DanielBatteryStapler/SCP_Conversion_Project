#ifndef JSON_HPP
#define JSON_HPP

#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <nlohmann/json.hpp>

namespace Json{
	nlohmann::json fromBson(const bsoncxx::document::view doc);
	bsoncxx::document::value toBson(const nlohmann::json doc);
	
	nlohmann::json oid(bsoncxx::oid id);
	bsoncxx::oid getOid(nlohmann::json json);
}

#endif // JSON_HPP
