#include "Json.hpp"

#include <fstream>

#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/core.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>

namespace Json{
	nlohmann::json loadJsonFromFile(std::string fileName){
		std::ifstream file(fileName);
		if(!file){
			throw std::runtime_error("Attempted to JSON from file \"" + fileName + "\", but could not read from that file");
		}
		
		std::vector<std::uint8_t> fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		std::string fileContentsStr(fileContents.begin(), fileContents.end());
		return nlohmann::json::parse(fileContentsStr);
	}
	
	namespace{
		template<typename T>
		nlohmann::json fromBsonInternal(const T& ele){
			switch(ele.type()){
				case bsoncxx::type::k_double:
					return ele.get_double().value;
				case bsoncxx::type::k_utf8:
					return std::string(ele.get_utf8().value);
				case bsoncxx::type::k_document:
					return fromBson(ele.get_document().value);
				case bsoncxx::type::k_array:
					{
						nlohmann::json arr = nlohmann::json::array();
						for(const auto& i : ele.get_array().value){
							arr.push_back(fromBsonInternal(i));
						}
						return arr;
					}
				case bsoncxx::type::k_oid:
					return oid(ele.get_oid().value);
				case bsoncxx::type::k_bool:
					return ele.get_bool().value;
				case bsoncxx::type::k_null:
					return nlohmann::json();
				case bsoncxx::type::k_int32:
					return ele.get_int32().value;
				case bsoncxx::type::k_int64:
					return ele.get_int64().value;
				default:
					throw std::runtime_error("Error in fromBson(...), unsupported data type");
			}
		}
	}
	
	nlohmann::json fromBson(const bsoncxx::document::view doc){
		nlohmann::json output = nlohmann::json::object();
		
		for(const auto& i : doc){
			output[std::string(i.key())] = fromBsonInternal(i);
		}
		
		return output;
	}
	
	namespace{
		namespace bbb = bsoncxx::builder::basic;
		
		void toBsonInternalArr(bbb::array& builder, const nlohmann::json& ele){
			switch(ele.type()){
				case nlohmann::detail::value_t::null:
					//nothing
					break;
				case nlohmann::detail::value_t::boolean:
					builder.append(ele.get<bool>());
					break;
				case nlohmann::detail::value_t::string:
					builder.append(ele.get<std::string>());
					break;
				case nlohmann::detail::value_t::number_unsigned:
					//builder.append(ele.get<uint64_t>());
					//break;
				case nlohmann::detail::value_t::number_integer:
					builder.append(ele.get<int64_t>());
					break;
				case nlohmann::detail::value_t::number_float:
					builder.append(ele.get<double>());
					break;
				case nlohmann::detail::value_t::object:
					if(ele.find("__bsonOid") != ele.end()){
						builder.append(getOid(ele));
					}
					else{
						builder.append(toBson(ele));
					}
					break;
				case nlohmann::detail::value_t::array:
					{
						bbb::array arr;
						
						for(auto i : ele){
							toBsonInternalArr(arr, i);
						}
						
						builder.append(arr.view());
					}
					break;
				default:
					throw std::runtime_error("Error in toBson(...), unsupported data type");
			}
		}

		void toBsonInternalDoc(bbb::document& builder, std::string_view key, const nlohmann::json& ele){
			switch(ele.type()){
				case nlohmann::detail::value_t::null:
					//nothing
					break;
				case nlohmann::detail::value_t::boolean:
					builder.append(bbb::kvp(key, ele.get<bool>()));
					break;
				case nlohmann::detail::value_t::string:
					builder.append(bbb::kvp(key, ele.get<std::string>()));
					break;
				case nlohmann::detail::value_t::number_unsigned:
					//builder.append(bbb::kvp(key, ele.get<uint64_t>()));
					//break;
				case nlohmann::detail::value_t::number_integer:
					builder.append(bbb::kvp(key, ele.get<int64_t>()));
					break;
				case nlohmann::detail::value_t::number_float:
					builder.append(bbb::kvp(key, ele.get<double>()));
					break;
				case nlohmann::detail::value_t::object:
					if(ele.find("__bsonOid") != ele.end()){
						builder.append(bbb::kvp(key, getOid(ele)));
					}
					else{
						builder.append(bbb::kvp(key, toBson(ele)));
					}
					break;
				case nlohmann::detail::value_t::array:
					{
						bbb::array arr;
						
						for(auto i : ele){
							toBsonInternalArr(arr, i);
						}
						
						builder.append(bbb::kvp(key, arr.view()));
					}
					break;
				default:
					throw std::runtime_error("Error in toBson(...), unsupported data type");
			}
		}
	}
	
	bsoncxx::document::value toBson(const nlohmann::json doc){
		bbb::document builder;
		
		for(auto i = doc.begin(); i != doc.end(); i++){
			toBsonInternalDoc(builder, i.key(), i.value());
		}
		
		return builder.extract();
	}
	
	nlohmann::json oid(bsoncxx::oid id){
		return {{"__bsonOid", id.to_string()}};
	}
	
	bsoncxx::oid getOid(nlohmann::json json){
		return bsoncxx::oid{json["__bsonOid"].get<std::string>()};
	}
}


















