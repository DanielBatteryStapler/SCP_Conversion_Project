#include "DatabaseTests.hpp"

#include "../../Database/Database.hpp"

namespace Tests{
	void addDatabaseTests(Tester& tester){
		tester.add("Database::connectToMongoDatabase", [](){
			//make that just making a connection doesn't cause an exception
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			db->getNumberOfPages();//make sure that a very simple command completes without error
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::createPage", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			
			db->cleanAndInitDatabase();
			
			assertTrue(db->getPageId("testPageA") == std::nullopt);
			Database::ID id = *db->createPage("testPageA");
			assertEquals(id, *db->getPageId("testPageA"));
			
			assertTrue(db->createPage("testPageA") == std::nullopt);
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::cleanAndInitDatabase", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			
			db->cleanAndInitDatabase();
			
			assertTrue(db->getPageId("testPageA") == std::nullopt);
			Database::ID id = *db->createPage("testPageA");
			assertEquals(id, *db->getPageId("testPageA"));
			
			db->cleanAndInitDatabase();
			
			assertTrue(db->getPageId("testPageA") == std::nullopt);
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::getNumberOfPages", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			
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
            std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			
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
            std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			
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
            std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			
			db->cleanAndInitDatabase();
			
            auto pageA = db->createPage("A");
            assertEquals(0, db->getPageTags(*pageA).size());
            db->setPageTags(*pageA, {"hello", "tags"});
            assertEquals(2, db->getPageTags(*pageA).size());
            db->setPageTags(*pageA, {"yeet"});
            assertEquals(1, db->getPageTags(*pageA).size());
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::createPageRevision", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			
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
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			
			db->cleanAndInitDatabase();
			
			Database::ID pageId = *db->createPage("testPageA");
			
			assertEquals(0u, db->getPageRevisions(pageId).size());
			
			Database::PageRevision rev;
			rev.title = "testRevA";
			
			Database::ID revId = db->createPageRevision(pageId, rev);
			
			auto revs = db->getPageRevisions(pageId);
			assertEquals(1u, revs.size());
			assertEquals(revId, revs[0]);
			
			rev.title = "testRevB";
			revId = db->createPageRevision(pageId, rev);
			
			revs = db->getPageRevisions(pageId);
			assertEquals(2u, revs.size());
			assertEquals(revId, revs[1]);
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("Database::getLatestPageRevision", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			
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
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			
			db->cleanAndInitDatabase();
			
			Database::ID pageId = *db->createPage("testPageA");
			
			Database::PageFile file;
			file.name = "testFile";
			
			Database::ID fileId = *db->createPageFile(pageId, file);
			assertEquals(file.name, db->getPageFile(fileId).name);
			assertEquals(fileId, *db->getPageFileId(pageId, file.name));
			assertTrue(std::nullopt == db->getPageFileId(pageId, "nonexistantFile"));
			assertTrue(std::nullopt == db->createPageFile(pageId, file));//duplicate file
			
			assertEquals(1, db->getPageFiles(pageId).size());
			
			Database::PageFile fileB;
			fileB.name = "testFileB";
			Database::ID fileIdB = *db->createPageFile(pageId, fileB);
			
			assertEquals(2, db->getPageFiles(pageId).size());
			auto arr = db->getPageFiles(pageId);
			assertEquals(fileId, arr[0]);
			assertEquals(fileIdB, arr[1]);
			
			Database::ID pageIdB = *db->createPage("testPageB");
			
			db->createPageFile(pageIdB, file);
			
			assertEquals(2, db->getPageFiles(pageId).size());
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
	}
}
