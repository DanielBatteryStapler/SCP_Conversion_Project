#include "DatabaseTests.hpp"

#include "../../Database/Database.hpp"

#include "../../Config.hpp"

namespace Tests{
	void addDatabaseTests(Tester& tester){
		tester.add("Database::connectToDatabase", [](){
			//make that just making a connection doesn't cause an exception
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			db->getNumberOfPages();//make sure that a very simple command completes without error
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::createPage", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			assertTrue(db->getPageId("test-page-a") == std::nullopt);
			Database::ID id = *db->createPage("test-page-a");
			assertEquals(id, *db->getPageId("test-page-a"));
			
			assertTrue(db->createPage("test-page-a") == std::nullopt);
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::cleanAndInitDatabase", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			assertTrue(db->getPageId("test-page-a") == std::nullopt);
			Database::ID id = *db->createPage("test-page-a");
			assertEquals(id, *db->getPageId("test-page-a"));
			
			db->cleanAndInitDatabase();
			
			assertTrue(db->getPageId("test-page-a") == std::nullopt);
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::getNumberOfPages", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			assertEquals(0, db->getNumberOfPages());
			db->createPage("test-page-a");
			assertEquals(1, db->getNumberOfPages());
			db->createPage("test-page-b");
			assertEquals(2, db->getNumberOfPages());
			db->createPage("test-page-b");
			assertEquals(2, db->getNumberOfPages());
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::getPageList", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			assertEqualsVec({}, db->getPageList());
			Database::ID a = *db->createPage("test-page-a");
			assertEqualsVec({a}, db->getPageList());
			Database::ID b = *db->createPage("test-page-b");
			assertEqualsVec({a,b}, db->getPageList());
			db->createPage("test-page-b");
			assertEqualsVec({a,b}, db->getPageList());
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::getPageName", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			Database::ID a = *db->createPage("test-page-a");
			Database::ID b = *db->createPage("test-page-b");
			assertEquals("test-page-a", db->getPageName(a));
			assertEquals("test-page-b", db->getPageName(b));
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::setPageParent", [](){
            std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			auto pageA = db->createPage("test-page-a");
			auto pageB = db->createPage("test-page-b");
            assertTrue(std::nullopt == db->getPageParent(*pageA));
            assertTrue(std::nullopt == db->getPageParent(*pageB));
            db->setPageParent(*pageA, *pageB);
            assertTrue(*pageB == *db->getPageParent(*pageA));
            assertTrue(std::nullopt == db->getPageParent(*pageB));
            db->setPageParent(*pageA, std::nullopt);
            assertTrue(std::nullopt == db->getPageParent(*pageA));
            assertTrue(std::nullopt == db->getPageParent(*pageB));
            
            Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::setPageDiscussion", [](){
            std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			auto pageA = db->createPage("test-page-a");
			auto threadB = db->createPage("test-page-b");///this should actually be a thread
            assertTrue(std::nullopt == db->getPageDiscussion(*pageA));
            db->setPageDiscussion(*pageA, *threadB);
            assertTrue(*threadB == *db->getPageDiscussion(*pageA));
            db->setPageDiscussion(*pageA, std::nullopt);
            assertTrue(std::nullopt == db->getPageDiscussion(*pageA));
            
            Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::setPageTags", [](){
            std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
            auto pageA = db->createPage("test-page-a");
            assertEqualsVec({}, db->getPageTags(*pageA));
            db->setPageTags(*pageA, {"hello", "tags"});
            assertEqualsVec({"hello", "tags"}, db->getPageTags(*pageA));
            db->setPageTags(*pageA, {"yeet"});
            assertEqualsVec({"yeet"}, db->getPageTags(*pageA));
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::createPageRevision", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			Database::ID pageId = *db->createPage("test-page-a");
			
			Database::PageRevision rev;
			rev.title = "testRevA";
			
			Database::ID revId = db->createPageRevision(pageId, rev);
			assertEquals(rev.title, db->getPageRevision(revId).title);
			
			rev.title = "testRevB";
			revId = db->createPageRevision(pageId, rev);
			assertEquals(rev.title, db->getPageRevision(revId).title);
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::getPageRevisions", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			Database::ID pageId = *db->createPage("test-page-a");
			
			assertEqualsVec({}, db->getPageRevisions(pageId));
			
			Database::PageRevision rev;
			rev.title = "testRevA";
			
			Database::ID revId = db->createPageRevision(pageId, rev);
			
			assertEqualsVec({revId}, db->getPageRevisions(pageId));
			
			rev.title = "testRevB";
			Database::ID revIdb = db->createPageRevision(pageId, rev);
			
			assertEqualsVec({revId, revIdb}, db->getPageRevisions(pageId));
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::getLatestPageRevision", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			Database::ID pageId = *db->createPage("test-page-a");
			
			Database::PageRevision rev;
			rev.title = "testRevA";
			
			Database::ID revId = db->createPageRevision(pageId, rev);
			assertEquals(rev.title, db->getLatestPageRevision(pageId).title);
			
			rev.title = "testRevB";
			revId = db->createPageRevision(pageId, rev);
			assertEquals(rev.title, db->getLatestPageRevision(pageId).title);
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::createPageFile", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			Database::ID pageId = *db->createPage("test-page-a");
			
			Database::PageFile file;
			file.name = "testFile";
			
			Database::ID fileId = *db->createPageFile(pageId, file);
			assertEquals(file.name, db->getPageFile(fileId).name);
			assertEquals(fileId, *db->getPageFileId(pageId, file.name));
			assertTrue(std::nullopt == db->getPageFileId(pageId, "nonexistantFile"));
			assertTrue(std::nullopt == db->createPageFile(pageId, file));//duplicate file
			
			assertEqualsVec({fileId}, db->getPageFiles(pageId));
			
			Database::PageFile fileB;
			fileB.name = "testFileB";
			Database::ID fileIdB = *db->createPageFile(pageId, fileB);
			
			assertEqualsVec({fileId, fileIdB}, db->getPageFiles(pageId));
			
			Database::ID pageIdB = *db->createPage("test-page-b");
			
			db->createPageFile(pageIdB, file);
			
			assertEqualsVec({fileId, fileIdB}, db->getPageFiles(pageId));
			
			std::string fileData = "BINARY TESTING\0DATA\r\n\r\nWITH WEIRD CHaraCTERS";
			
			std::stringstream inputStream(fileData);
			
			db->uploadPageFile(fileId, inputStream);
			std::stringstream outputStream;
			db->downloadPageFile(fileId, outputStream);
			assertEquals(fileData, outputStream.str());
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::createForumGroup", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			assertEqualsVec({}, db->getForumGroups());
			
			Database::ForumGroup groupA;
			groupA.title = "Title A";
			groupA.description = "description A";
			
			Database::ID groupAId = db->createForumGroup(groupA);
			assertEqualsVec({groupAId}, db->getForumGroups());
			
			Database::ForumGroup groupB;
			groupB.title = "Title B";
			groupB.description = "description B";
			
			Database::ID groupBId = db->createForumGroup(groupB);
			assertEqualsVec({groupAId, groupBId}, db->getForumGroups());
			
			assertEquals(groupA.title, db->getForumGroup(groupAId).title);
			assertEquals(groupA.description, db->getForumGroup(groupAId).description);
			
			assertEquals(groupB.title, db->getForumGroup(groupBId).title);
			assertEquals(groupB.description, db->getForumGroup(groupBId).description);
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::createForumCategory", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			Database::ForumGroup groupA;
			groupA.title = "Title A";
			groupA.description = "description A";
			
			Database::ID groupAId = db->createForumGroup(groupA);
			
			Database::ForumGroup groupB;
			groupB.title = "Title B";
			groupB.description = "description B";
			
			Database::ID groupBId = db->createForumGroup(groupB);
			
			assertEqualsVec({}, db->getForumCategories(groupAId));
			assertEqualsVec({}, db->getForumCategories(groupBId));
			
			Database::ForumCategory categoryA;
			categoryA.title = "Category Title A";
			categoryA.description = "Category Description A";
			
			Database::ID categoryAId = db->createForumCategory(groupAId, categoryA);
			
			assertEqualsVec({categoryAId}, db->getForumCategories(groupAId));
			assertEqualsVec({}, db->getForumCategories(groupBId));
			
			Database::ForumCategory categoryB;
			categoryB.title = "Category Title B";
			categoryB.description = "Category Description B";
			
			Database::ID categoryBId = db->createForumCategory(groupAId, categoryB);
			
			Database::ForumCategory categoryC;
			categoryC.title = "Category Title C";
			categoryC.description = "Category Description C";
			
			Database::ID categoryCId = db->createForumCategory(groupBId, categoryC);
			
			assertEqualsVec({categoryAId, categoryBId}, db->getForumCategories(groupAId));
			assertEqualsVec({categoryCId}, db->getForumCategories(groupBId));
			
			assertEquals(categoryA.title, db->getForumCategory(categoryAId).title);
			assertEquals(categoryA.description, db->getForumCategory(categoryAId).description);
			
			assertEquals(categoryB.title, db->getForumCategory(categoryBId).title);
			assertEquals(categoryB.description, db->getForumCategory(categoryBId).description);
			
			assertEquals(categoryC.title, db->getForumCategory(categoryCId).title);
			assertEquals(categoryC.description, db->getForumCategory(categoryCId).description);
			
			Database::eraseDatabase(std::move(db));
		});
	}
}
















