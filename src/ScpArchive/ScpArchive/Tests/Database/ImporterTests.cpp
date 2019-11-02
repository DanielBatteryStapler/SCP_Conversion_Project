#include "ImporterTests.hpp"

#include "../../Database/Importer.hpp"
#include "../../Database/Json.hpp"

#include "DatabaseTests.hpp"

#include "../../Config.hpp"

namespace Tests{
	namespace{
		const nlohmann::json testForumGroups = {
			{
				{"title", "Test Group A"},
				{"description", "Group A Description"},
				{"categories", 
					{
						{
							{"id", "category-id-a"},
							{"title", "Test Category A"}, 
							{"description", "Category A Description"}
						},
						{
							{"id", "category-id-b"},
							{"title", "Test Category B"}, 
							{"description", "Category B Description"}
						}
					}
				}
			},
			{
				{"title", "Test Group B"},
				{"description", "Group B Description"},
				{"categories", 
					{
						{
							{"id", "category-id-c"},
							{"title", "Test Category C"}, 
							{"description", "Category C Description"}
						}
					}
				}
			}
		};
		
		const nlohmann::json testPageA = {
			{"id", "testPageARawID"},
			{"name", "test-page-a"},
			{"parent", "testPageBRawID"},
			{"discussionId", "testThreadARawID"},
			{"tags", {"test", "page"}},
			{"votes", 
				nlohmann::json::array()
			},
			{"files", 
				{
					{
						{"id", "testPageAFileIDA"},
						{"name", "test-file"},
						{"description", "test description"},
						{"url", "https://test.com/"},
						{"timeStamp", "1111116"},
						{"authorId", "authorAID"}
					}
				}
			},
			{"revisions",
				{
					{
						{"id", "testPageARevisionIDA"},
						{"title", "Test Page A"},
						{"timeStamp", "1111110"},
						{"authorId", "authorAID"},
						{"changeMessage", "created page A"},
						{"changeType", "initial"},
						{"sourceCode", "Page A body"}
					},
					{
						{"id", "testPageARevisionIDB"},
						{"title", "Test Page B"},
						{"timeStamp", "1111113"},
						{"authorId", "authorAID"},
						{"changeMessage", "edited page A"},
						{"changeType", "edited source code"},
						{"sourceCode", "Page A body edited"}
					}
				}
			}
		};
		
		const nlohmann::json testPageB = {
			{"id", "testPageBRawID"},
			{"name", "test-page-b"},
			{"parent", ""},
			{"discussionId", "testThreadBRawID"},
			{"tags", {"test", "not-page"}},
			{"votes", 
				nlohmann::json::array()
			},
			{"files", 
				nlohmann::json::array()
			},
			{"revisions",
				{
					{
						{"id", "testPageBRevisionID"},
						{"title", "Test Page B"},
						{"timeStamp", "1111112"},
						{"authorId", "authorBID"},
						{"changeMessage", "created page B"},
						{"changeType", "initial"},
						{"sourceCode", "Page B body"}
					}
				}
			}
		};
	}

	void addImporterTests(Tester& tester){
		tester.add("Importer::importForumGroups", [](){
			Importer::ImportMap map;
			
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			db->cleanAndInitDatabase();
			
			importForumGroups(db.get(), map, testForumGroups);
			
			std::vector<Database::ID> groups = db->getForumGroups();
			assertEquals(testForumGroups.size(), groups.size());
			for(std::size_t i = 0; i < groups.size(); i++){
                nlohmann::json exGroup = testForumGroups[i];
                Database::ForumGroup acGroup = db->getForumGroup(groups[i]);
                assertEquals(exGroup["title"].get<std::string>(), acGroup.title);
                assertEquals(exGroup["description"].get<std::string>(), acGroup.description);
                
                std::vector<Database::ID> categories = db->getForumCategories(groups[i]);
                assertEquals(exGroup["categories"].size(), categories.size());
                for(std::size_t y = 0; y < categories.size(); y++){
                    nlohmann::json exCategory = exGroup["categories"][y];
                    Database::ForumCategory acCategory = db->getForumCategory(categories[y]);
                    assertEquals(exCategory["title"].get<std::string>(), acCategory.title);
                    assertEquals(exCategory["description"].get<std::string>(), acCategory.description);
                }
			}
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Importer::importBasicPageData", [](){
			Importer::ImportMap map;
			
			std::unique_ptr<Database> database = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			database->cleanAndInitDatabase();
			
			Importer::importBasicPageData(database.get(), map, testPageA);
			
			assertEquals(1u, database->getNumberOfPages());
			
			Database::ID pageId = *database->getPageId(testPageA["name"].get<std::string>());
			assertEquals(map.getPageMap(testPageA["id"].get<std::string>()), pageId);
			{
				std::vector<std::string> expectedTags = testPageA["tags"];
				std::vector<std::string> actualTags = database->getPageTags(pageId);
				
				assertEquals(expectedTags.size(), actualTags.size());
				for(std::size_t i = 0; i < expectedTags.size(); i++){
					assertEquals(expectedTags[i], actualTags[i]);
				}
			}
			assertTrue(std::nullopt == database->getPageDiscussion(pageId));
			assertTrue(std::nullopt == database->getPageParent(pageId));
			{
				std::vector<Database::ID> pageRevisions = database->getPageRevisions(pageId);
				
				assertEquals(testPageA["revisions"].size(), pageRevisions.size());
				for(std::size_t i = 0; i < pageRevisions.size(); i++){
					nlohmann::json exRev = testPageA["revisions"][i];
					Database::PageRevision acRev = database->getPageRevision(pageRevisions[i]);
					
					assertEquals(exRev["title"].get<std::string>(), acRev.title);
					assertEquals(std::stoll(exRev["timeStamp"].get<std::string>()), acRev.timeStamp);
					assertEquals(exRev["changeMessage"].get<std::string>(), acRev.changeMessage);
					assertEquals(exRev["changeType"].get<std::string>(), acRev.changeType);
					assertEquals(exRev["sourceCode"].get<std::string>(), acRev.sourceCode);
				}
			}
			
			{
				std::vector<Database::ID> pageFiles = database->getPageFiles(pageId);
				
				assertEquals(testPageA["files"].size(), pageFiles.size());
				for(std::size_t i = 0; i < pageFiles.size(); i++){
					nlohmann::json exFile = testPageA["files"][i];
					Database::PageFile acFile = database->getPageFile(pageFiles[i]);
					
					assertEquals(exFile["name"].get<std::string>(), acFile.name);
					assertEquals(exFile["description"].get<std::string>(), acFile.description);
					assertEquals(std::stoll(exFile["timeStamp"].get<std::string>()), acFile.timeStamp);
					
					assertEquals(map.getFileMap(exFile["id"].get<std::string>()), pageFiles[i]);
				}
			}
			
			Database::wipeDatabaseFromMongo(std::move(database));
		});
		
		tester.add("Importer::linkPageParent", [](){
			Importer::ImportMap map;
			
			std::unique_ptr<Database> database = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			database->cleanAndInitDatabase();
			
			Importer::importBasicPageData(database.get(), map, testPageA);
			Database::ID pageAId = *database->getPageId(testPageA["name"].get<std::string>());
			
			Importer::importBasicPageData(database.get(), map, testPageB);
			Database::ID pageBId = *database->getPageId(testPageB["name"].get<std::string>());
			
			assertEquals(2u, database->getNumberOfPages());
			
			assertTrue(database->getPageParent(pageAId) == std::nullopt);
			assertTrue(database->getPageParent(pageBId) == std::nullopt);
			
			Importer::linkPageParent(database.get(), map, testPageA);
			Importer::linkPageParent(database.get(), map, testPageB);
			
			assertTrue(*database->getPageParent(pageAId) == pageBId);
			assertTrue(database->getPageParent(pageBId) == std::nullopt);
			
			Database::wipeDatabaseFromMongo(std::move(database));
		});
		
		tester.add("Importer::ImportMap", [](){
			{
				Importer::ImportMap importMap;
				
				Database::ID idA = Database::ID("507f1f77bcf86cd799439011");//just some valid, but meaningless test ids
				Database::ID idB = Database::ID("507f191e810c19729de860ea");
				
				std::string rawA = "testRawIDA";
				std::string rawB = "testRawIDB";
				
				shouldThrowException([&](){
					importMap.getThreadMap(rawA);
				});
				shouldThrowException([&](){
					importMap.getThreadMap(rawB);
				});
				importMap.setThreadMap(rawA, idA);
				importMap.setThreadMap(rawB, idB);
				assertEquals(idA, importMap.getThreadMap(rawA));
				assertEquals(idB, importMap.getThreadMap(rawB));
			}
			{
				Importer::ImportMap importMap;
				
				Database::ID idA = Database::ID("507f1f77bcf86cd799439011");//just some valid, but meaningless test ids
				Database::ID idB = Database::ID("507f191e810c19729de860ea");
				
				std::string rawA = "testRawIDA";
				std::string rawB = "testRawIDB";
				
				shouldThrowException([&](){
					importMap.getCategoryMap(rawA);
				});
				shouldThrowException([&](){
					importMap.getCategoryMap(rawB);
				});
				importMap.setCategoryMap(rawA, idA);
				importMap.setCategoryMap(rawB, idB);
				assertEquals(idA, importMap.getCategoryMap(rawA));
				assertEquals(idB, importMap.getCategoryMap(rawB));
			}
			{
				Importer::ImportMap importMap;
				
				Database::ID idA = Database::ID("507f1f77bcf86cd799439011");//just some valid, but meaningless test ids
				Database::ID idB = Database::ID("507f191e810c19729de860ea");
				
				std::string rawA = "testRawIDA";
				std::string rawB = "testRawIDB";
				
				shouldThrowException([&](){
					importMap.getPageMap(rawA);
				});
				shouldThrowException([&](){
					importMap.getPageMap(rawB);
				});
				importMap.setPageMap(rawA, idA);
				importMap.setPageMap(rawB, idB);
				assertEquals(idA, importMap.getPageMap(rawA));
				assertEquals(idB, importMap.getPageMap(rawB));
			}
			{
				Importer::ImportMap importMap;
				
				Database::ID idA = Database::ID("507f1f77bcf86cd799439011");//just some valid, but meaningless test ids
				Database::ID idB = Database::ID("507f191e810c19729de860ea");
				
				std::string rawA = "testRawIDA";
				std::string rawB = "testRawIDB";
				
				shouldThrowException([&](){
					importMap.getFileMap(rawA);
				});
				shouldThrowException([&](){
					importMap.getFileMap(rawB);
				});
				importMap.setFileMap(rawA, idA);
				importMap.setFileMap(rawB, idB);
				assertEquals(idA, importMap.getFileMap(rawA));
				assertEquals(idB, importMap.getFileMap(rawB));
			}
		});
	}
}
