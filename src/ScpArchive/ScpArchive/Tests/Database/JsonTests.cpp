#include "JsonTests.hpp"

#include "../../Database/Json.hpp"

#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>

namespace bbb = bsoncxx::builder::basic;

namespace Tests{
	void addJsonTests(Tester& tester){
		tester.add("Json::fromBson", [](){
			assertTrue(nlohmann::json({{"basic", "value"}}) == Json::fromBson(bbb::make_document(bbb::kvp("basic", "value"))));
			assertTrue(nlohmann::json({{"num", 420}}) == Json::fromBson(bbb::make_document(bbb::kvp("num", 420))));
			assertTrue(nlohmann::json({{"basic", "value"}, {"num", 420}}) == Json::fromBson(bbb::make_document(bbb::kvp("basic", "value"), bbb::kvp("num", 420))));
			
			assertTrue(nlohmann::json({{"basic", "value"}, {"nested", {{"value", 500}}}}) 
			== Json::fromBson(bbb::make_document(bbb::kvp("basic", "value"), bbb::kvp("nested", bbb::make_document(bbb::kvp("value", 500))))));
			assertTrue(nlohmann::json({{"array", {"a", "b", false, 45}}}) == Json::fromBson(bbb::make_document(bbb::kvp("array", bbb::make_array("a", "b", false, 45)))));
			
			assertTrue(nlohmann::json({{"emptyArray", nlohmann::json::array()}}) == Json::fromBson(bbb::make_document(bbb::kvp("emptyArray", bbb::make_array()))));
			
			assertTrue(nlohmann::json({{"basicId", Json::oid(bsoncxx::oid{"5da7d276b5f02a16362f2c73"})}}) == Json::fromBson(bbb::make_document(bbb::kvp("basicId", bsoncxx::oid{"5da7d276b5f02a16362f2c73"}))));
		});
		
		tester.add("Json::toBson", [](){
			assertTrue(Json::toBson({{"basic", "value"}}).view() == bbb::make_document(bbb::kvp("basic", "value")));
			assertTrue(Json::toBson({{"num", 420}}).view() == bbb::make_document(bbb::kvp("num", 420l)));
			assertTrue(Json::toBson({{"basic", "value"}, {"num", 420}}).view() == bbb::make_document(bbb::kvp("basic", "value"), bbb::kvp("num", 420l)));
			
			assertTrue(Json::toBson({{"basic", "value"}, {"nested", {{"value", 500}}}}).view() 
			== bbb::make_document(bbb::kvp("basic", "value"), bbb::kvp("nested", bbb::make_document(bbb::kvp("value", 500l)))));
			assertTrue(Json::toBson(nlohmann::json({{"array", {"a", "b", false, 45, {1, 2}}}})).view() == bbb::make_document(bbb::kvp("array", bbb::make_array("a", "b", false, 45l, bbb::make_array(1l, 2l)))));
			
			assertTrue(Json::toBson({{"basicId", Json::oid(bsoncxx::oid{"5da7d276b5f02a16362f2c73"})}}).view() == bbb::make_document(bbb::kvp("basicId", bsoncxx::oid{"5da7d276b5f02a16362f2c73"})));
		});
	}
}
