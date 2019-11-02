#include "DatabaseTests.hpp"

#include "../../Database/Database.hpp"

#include "../../Config.hpp"

namespace Tests{
	void addDatabaseTests(Tester& tester){
		tester.add("Database::connectToMongoDatabase", [](){
			//make that just making a connection doesn't cause an exception
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			db->getNumberOfPages();//make sure that a very simple command completes without error
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::createPage", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			assertTrue(db->getPageId("testPageA") == std::nullopt);
			Database::ID id = *db->createPage("testPageA");
			assertEquals(id, *db->getPageId("testPageA"));
			
			assertTrue(db->createPage("testPageA") == std::nullopt);
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::cleanAndInitDatabase", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			assertTrue(db->getPageId("testPageA") == std::nullopt);
			Database::ID id = *db->createPage("testPageA");
			assertEquals(id, *db->getPageId("testPageA"));
			
			db->cleanAndInitDatabase();
			
			assertTrue(db->getPageId("testPageA") == std::nullopt);
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::getNumberOfPages", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			assertEquals(0, db->getNumberOfPages());
			db->createPage("testPageA");
			assertEquals(1, db->getNumberOfPages());
			db->createPage("testPageB");
			assertEquals(2, db->getNumberOfPages());
			db->createPage("testPageB");
			assertEquals(2, db->getNumberOfPages());
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::setPageParent", [](){
            std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			auto pageA = db->createPage("A");
			auto pageB = db->createPage("B");
            assertTrue(std::nullopt == db->getPageParent(*pageA));
            assertTrue(std::nullopt == db->getPageParent(*pageB));
            db->setPageParent(*pageA, *pageB);
            assertTrue(*pageB == *db->getPageParent(*pageA));
            assertTrue(std::nullopt == db->getPageParent(*pageB));
            db->setPageParent(*pageA, std::nullopt);
            assertTrue(std::nullopt == db->getPageParent(*pageA));
            assertTrue(std::nullopt == db->getPageParent(*pageB));
            
            Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::setPageDiscussion", [](){
            std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			auto pageA = db->createPage("A");
			auto threadB = db->createPage("B");///this should actually be a thread
            assertTrue(std::nullopt == db->getPageDiscussion(*pageA));
            db->setPageDiscussion(*pageA, *threadB);
            assertTrue(*threadB == *db->getPageDiscussion(*pageA));
            db->setPageDiscussion(*pageA, std::nullopt);
            assertTrue(std::nullopt == db->getPageDiscussion(*pageA));
            
            Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::setPageTags", [](){
            std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
            auto pageA = db->createPage("A");
            assertEqualsVec({}, db->getPageTags(*pageA));
            db->setPageTags(*pageA, {"hello", "tags"});
            assertEqualsVec({"hello", "tags"}, db->getPageTags(*pageA));
            db->setPageTags(*pageA, {"yeet"});
            assertEqualsVec({"yeet"}, db->getPageTags(*pageA));
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::createPageRevision", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			Database::ID pageId = *db->createPage("testPageA");
			
			Database::PageRevision rev;
			rev.title = "testRevA";
			
			Database::ID revId = db->createPageRevision(pageId, rev);
			assertEquals(rev.title, db->getPageRevision(revId).title);
			
			rev.title = "testRevB";
			revId = db->createPageRevision(pageId, rev);
			assertEquals(rev.title, db->getPageRevision(revId).title);
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::getPageRevisions", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			Database::ID pageId = *db->createPage("testPageA");
			
			assertEqualsVec({}, db->getPageRevisions(pageId));
			
			Database::PageRevision rev;
			rev.title = "testRevA";
			
			Database::ID revId = db->createPageRevision(pageId, rev);
			
			assertEqualsVec({revId}, db->getPageRevisions(pageId));
			
			rev.title = "testRevB";
			Database::ID revIdb = db->createPageRevision(pageId, rev);
			
			assertEqualsVec({revId, revIdb}, db->getPageRevisions(pageId));
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::getLatestPageRevision", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			Database::ID pageId = *db->createPage("testPageA");
			
			Database::PageRevision rev;
			rev.title = "testRevA";
			
			Database::ID revId = db->createPageRevision(pageId, rev);
			assertEquals(rev.title, db->getLatestPageRevision(pageId).title);
			
			rev.title = "testRevB";
			revId = db->createPageRevision(pageId, rev);
			assertEquals(rev.title, db->getLatestPageRevision(pageId).title);
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::createPageFile", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			
			db->cleanAndInitDatabase();
			
			Database::ID pageId = *db->createPage("testPageA");
			
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
			
			Database::ID pageIdB = *db->createPage("testPageB");
			
			db->createPageFile(pageIdB, file);
			
			assertEqualsVec({fileId, fileIdB}, db->getPageFiles(pageId));
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::createForumGroup", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			
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
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::createForumCategory", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getTestingDatabaseName());
			
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
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
	}
}
















