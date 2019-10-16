#include "DatabaseTests.hpp"

#include "../../Database/Database.hpp"

namespace{
	const std::string testDatabaseName = "TemporaryTestingDatabaseFile";
}

namespace Tests{
	void addDatabaseTests(Tester& tester){
		tester.add("testDatabaseConnection", [](){
			//make that just making a connection doesn't cause an exception
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			db->getNumberOfPages();//make sure that a very simple command completes without error
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("testDatabasePageInsert", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			
			db->cleanAndInitDatabase();
			
			assertTrue(db->getPageId("testPageA") == std::nullopt);
			Database::ID id = *db->createPage("testPageA");
			assertEquals(id, *db->getPageId("testPageA"));
			
			
			Database::wipeDatabaseFromMongo(std::move(db));
		});
		
		tester.add("testDatabasePageCount", [](){
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
		
		tester.add("testDatabaseGetRevision", [](){
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
		
		tester.add("testDatabaseGetRevisions", [](){
			std::unique_ptr<Database> db = Database::connectToMongoDatabase(testDatabaseName);
			
			db->cleanAndInitDatabase();
			
			Database::ID pageId = *db->createPage("testPageA");
			
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
		
		tester.add("testDatabaseGetLastRevision", [](){
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
	}
}
