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
			{"parent", "test-page-b"},
			{"discussionId", ""},
			{"tags", {"test", "page"}},
			{"votes", 
				{
					{
						{"authorId", "deleted"},
						{"voteType", true}
					},
					{
						{"authorId", "deleted"},
						{"voteType", false}
					},
					{
						{"authorId", "authorAID"},
						{"voteType", true}
					}
				}
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
			{"discussionId", "testThreadAId"},
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
		
		const nlohmann::json testThread = {
			{"id", "testThreadAId"}, 
			{"categoryId", "category-id-a"},
			{"title", "Thread Title"},
			{"authorId", "authorAID"},
			{"description", "Thread description"},
			{"timeStamp", "1111213"},
			{"posts",
				{
					{
						{"postId", "postIdA"},
						{"authorId", "authorAID"}, 
						{"timeStamp", "1111233"},
						{"title", "Post Title"},
						{"content", "post content"},
						{"posts",
							{
								{
									{"postId", "postIdB"},
									{"authorId", "authorAID"}, 
									{"timeStamp", "1111234"},
									{"title", "Post Title B"},
									{"content", "post content B"},
									{"posts",
										nlohmann::json::array()
									}
								}
							}
						}
					}
				}
			}
		};
		
		const nlohmann::json testAuthorA = {
			{"id", "authorAID"}, 
			{"name", "Author A's Name"}
		};
		
		const nlohmann::json testAuthorB = {
			{"id", "authorBID"}, 
			{"name", "Agent Buthor B"}
		};
	}

	void addImporterTests(Tester& tester){
		tester.add("Importer::importForumGroups", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			db->cleanAndInitDatabase();
			Importer::ImportMap map(db.get());
			
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
                    assertEquals(exCategory["id"].get<std::string>(), acCategory.sourceId);
                    assertEquals(exCategory["title"].get<std::string>(), acCategory.title);
                    assertEquals(exCategory["description"].get<std::string>(), acCategory.description);
                }
			}
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Importer::importPage", [](){
			std::unique_ptr<Database> database = Database::connectToDatabase(Config::getTestingDatabaseName());
			database->cleanAndInitDatabase();
			Importer::ImportMap map(database.get());
			
			Importer::importAuthor(database.get(), map, testAuthorA);
			Importer::importAuthor(database.get(), map, testAuthorB);
			
			Importer::importPage(database.get(), map, testPageA);
			
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
                    assertEquals(map.getAuthorMap(exRev["authorId"].get<std::string>()), acRev.authorId.value());
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
					assertEquals(map.getAuthorMap(exFile["authorId"].get<std::string>()), acFile.authorId.value());
					
					assertEquals(map.getFileMap(exFile["id"].get<std::string>()), pageFiles[i]);
				}
			}
			
			assertEquals(3u, database->getPageVotes(pageId).size());
			assertEquals(1, database->getPageRating(pageId));
			
			Database::eraseDatabase(std::move(database));
		});
		
		tester.add("Importer::importPage Parents", [](){
			std::unique_ptr<Database> database = Database::connectToDatabase(Config::getTestingDatabaseName());
			database->cleanAndInitDatabase();
			Importer::ImportMap map(database.get());
			
			Importer::importAuthor(database.get(), map, testAuthorA);
			Importer::importAuthor(database.get(), map, testAuthorB);
			
			Importer::importPage(database.get(), map, testPageA);
			Database::ID pageAId = *database->getPageId(testPageA["name"].get<std::string>());
			
			Importer::importPage(database.get(), map, testPageB);
			Database::ID pageBId = *database->getPageId(testPageB["name"].get<std::string>());
			
			assertEquals(2u, database->getNumberOfPages());
			
			//assertTrue(database->getPageParent(pageAId) == std::nullopt);
			//assertTrue(database->getPageParent(pageBId) == std::nullopt);
			
			//Importer::linkPageParent(database.get(), map, testPageA);
			//Importer::linkPageParent(database.get(), map, testPageB);
			
			assertEquals(testPageB["name"].get<std::string>(), *database->getPageParent(pageAId));
			assertTrue(database->getPageParent(pageBId) == std::nullopt);
			
			Database::eraseDatabase(std::move(database));
		});
		
		tester.add("Importer::importThread", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			db->cleanAndInitDatabase();
			Importer::ImportMap map(db.get());
			
			Importer::importAuthor(db.get(), map, testAuthorA);
			Importer::importAuthor(db.get(), map, testAuthorB);
			
			Importer::importForumGroups(db.get(), map, testForumGroups);
			Database::ID categoryId = db->getForumCategories(db->getForumGroups()[0])[0];
			
			assertEqualsVec({}, db->getForumThreads(categoryId));
			Importer::importThread(db.get(), map, testThread);
			Database::ID threadId = *db->getForumThreadId(testThread["id"].get<std::string>());
			assertEqualsVec({threadId}, db->getForumThreads(categoryId));
			
			Database::ForumThread thread = db->getForumThread(threadId);
			assertEquals(categoryId, thread.parent);
			assertEquals(testThread["id"].get<std::string>(), thread.sourceId);
			assertEquals(testThread["title"].get<std::string>(), thread.title);
			assertEquals(testThread["description"].get<std::string>(), thread.description);
			assertEquals(std::stoll(testThread["timeStamp"].get<std::string>()), thread.timeStamp);
			
			assertEquals(1u, db->getForumReplies(threadId).size());
			Database::ID postAId = db->getForumReplies(threadId)[0];
			Database::ForumPost postA = db->getForumPost(postAId);
			assertEquals(threadId, postA.parentThread);
			assertTrue(postA.parentPost == std::nullopt);
			assertEquals(testThread["posts"][0]["title"].get<std::string>(), postA.title);
			assertEquals(testThread["posts"][0]["content"].get<std::string>(), postA.content);
			assertEquals(std::stoll(testThread["posts"][0]["timeStamp"].get<std::string>()), postA.timeStamp);
			assertEquals(map.getAuthorMap(testThread["posts"][0]["authorId"].get<std::string>()), postA.authorId.value());
			
			assertEquals(1u, db->getForumReplies(threadId, postAId).size());
			Database::ID postBId = db->getForumReplies(threadId, postAId)[0];
			Database::ForumPost postB = db->getForumPost(postBId);
			assertEquals(threadId, postB.parentThread);
			assertEquals(postAId, postB.parentPost.value());
			assertEquals(testThread["posts"][0]["posts"][0]["title"].get<std::string>(), postB.title);
			assertEquals(testThread["posts"][0]["posts"][0]["content"].get<std::string>(), postB.content);
			assertEquals(std::stoll(testThread["posts"][0]["posts"][0]["timeStamp"].get<std::string>()), postB.timeStamp);
			assertEquals(map.getAuthorMap(testThread["posts"][0]["posts"][0]["authorId"].get<std::string>()), postB.authorId.value());
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Importer::importPage Discussion Linking", [](){
			std::unique_ptr<Database> database = Database::connectToDatabase(Config::getTestingDatabaseName());
			database->cleanAndInitDatabase();
			Importer::ImportMap map(database.get());
			
			Importer::importAuthor(database.get(), map, testAuthorA);
			Importer::importAuthor(database.get(), map, testAuthorB);
			
			Importer::importForumGroups(database.get(), map, testForumGroups);
			Importer::importThread(database.get(), map, testThread);
			
			Importer::importPage(database.get(), map, testPageA);
			Database::ID pageAId = *database->getPageId(testPageA["name"].get<std::string>());
			
			Importer::importPage(database.get(), map, testPageB);
			Database::ID pageBId = *database->getPageId(testPageB["name"].get<std::string>());
			
			//assertTrue(database->getPageDiscussion(pageAId) == std::nullopt);
			//assertTrue(database->getPageDiscussion(pageBId) == std::nullopt);
			//Importer::linkPageDiscussionThread(database.get(), map, testPageA);
			//Importer::linkPageDiscussionThread(database.get(), map, testPageB);
			assertTrue(database->getPageDiscussion(pageAId) == std::nullopt);
			assertEquals(testThread["id"].get<std::string>(), *database->getPageDiscussion(pageBId));
			
			Database::eraseDatabase(std::move(database));
		});
		
		tester.add("Importer::importAuthor", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			db->cleanAndInitDatabase();
			Importer::ImportMap map(db.get());
			
			Database::Author wikidot = db->getAuthor(map.getAuthorMap("wikidot"));
			assertEquals(static_cast<short>(Database::Author::Type::System), static_cast<short>(wikidot.type));
			
			assertTrue(map.authorMapExists(testAuthorA["id"].get<std::string>()) == false);
			
			Importer::importAuthor(db.get(), map, testAuthorA);
			assertTrue(map.authorMapExists(testAuthorA["id"].get<std::string>()));
			
			Database::ID authorId = map.getAuthorMap(testAuthorA["id"].get<std::string>());
			Database::Author aTest = db->getAuthor(authorId);
			assertEquals(static_cast<short>(Database::Author::Type::User), static_cast<short>(aTest.type));
			assertEquals(testAuthorA["name"].get<std::string>(), aTest.name);
			
			//make sure that importing the same thing twice works
			Importer::importAuthor(db.get(), map, testAuthorA);
			
			assertEquals(authorId, map.getAuthorMap(testAuthorA["id"].get<std::string>()));
			aTest = db->getAuthor(map.getAuthorMap(testAuthorA["id"].get<std::string>()));
			assertEquals(static_cast<short>(Database::Author::Type::User), static_cast<short>(aTest.type));
			assertEquals(testAuthorA["name"].get<std::string>(), aTest.name);
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Importer::ImportMap", [](){
			{//MapCategory::Page
				std::unique_ptr<Database> database = Database::connectToDatabase(Config::getTestingDatabaseName());
				database->cleanAndInitDatabase();
				Importer::ImportMap importMap(database.get());
				
				Database::ID idA = 0;//just some valid, but meaningless test ids
				Database::ID idB = 1;
				
				std::string rawA = "testRawIDA";
				std::string rawB = "testRawIDB";
				
				shouldThrowException([&](){
					importMap.getPageMap(rawA);
				});
				shouldThrowException([&](){
					importMap.getPageMap(rawB);
				});
				assertTrue(!importMap.pageMapExists(rawA));
				importMap.setPageMap(rawA, idA);
				assertTrue(importMap.pageMapExists(rawA));
				importMap.setPageMap(rawB, idB);
				assertEquals(idA, importMap.getPageMap(rawA));
				assertEquals(idB, importMap.getPageMap(rawB));
				assertEquals(rawA, importMap.getPageMapRaw(idA));
				assertEquals(rawB, importMap.getPageMapRaw(idB));
			}
			{//MapCategory::File
				std::unique_ptr<Database> database = Database::connectToDatabase(Config::getTestingDatabaseName());
				database->cleanAndInitDatabase();
				Importer::ImportMap importMap(database.get());
				
				Database::ID idA = 0;//just some valid, but meaningless test ids
				Database::ID idB = 1;
				
				std::string rawA = "testRawIDA";
				std::string rawB = "testRawIDB";
				
				shouldThrowException([&](){
					importMap.getFileMap(rawA);
				});
				shouldThrowException([&](){
					importMap.getFileMap(rawB);
				});
				assertTrue(!importMap.fileMapExists(rawA));
				importMap.setFileMap(rawA, idA);
				assertTrue(importMap.fileMapExists(rawA));
				importMap.setFileMap(rawB, idB);
				assertEquals(idA, importMap.getFileMap(rawA));
				assertEquals(idB, importMap.getFileMap(rawB));
				assertEquals(rawA, importMap.getFileMapRaw(idA));
				assertEquals(rawB, importMap.getFileMapRaw(idB));
			}
			{//MapCategory::Author
				std::unique_ptr<Database> database = Database::connectToDatabase(Config::getTestingDatabaseName());
				database->cleanAndInitDatabase();
				Importer::ImportMap importMap(database.get());
				
				Database::ID idA = 0;//just some valid, but meaningless test ids
				Database::ID idB = 1;
				
				std::string rawA = "testRawIDA";
				std::string rawB = "testRawIDB";
				
				shouldThrowException([&](){
					importMap.getAuthorMap(rawA);
				});
				shouldThrowException([&](){
					importMap.getAuthorMap(rawB);
				});
				assertTrue(!importMap.authorMapExists(rawA));
				importMap.setAuthorMap(rawA, idA);
				assertTrue(importMap.authorMapExists(rawA));
				importMap.setAuthorMap(rawB, idB);
				assertEquals(idA, importMap.getAuthorMap(rawA));
				assertEquals(idB, importMap.getAuthorMap(rawB));
				assertEquals(rawA, importMap.getAuthorMapRaw(idA));
				assertEquals(rawB, importMap.getAuthorMapRaw(idB));
			}
		});
	}
}
