#include "DatabaseTests.hpp"

#include "../../Database/Database.hpp"

#include "../../Config.hpp"

namespace Tests{
	void addDatabaseTests(Tester& tester){
		tester.add("Database::connectToDatabase", [](){
			//make that just making a connection doesn't cause an exception
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			db->cleanAndInitDatabase();
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
            db->setPageParent(*pageA, "test-page-b");
            assertTrue("test-page-b" == *db->getPageParent(*pageA));
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
			Database::ForumGroup group;
			Database::ID groupId = db->createForumGroup(group);
			Database::ForumCategory category;
			Database::ID categoryId = db->createForumCategory(groupId, category);
			
            assertTrue(std::nullopt == db->getPageDiscussion(*pageA));
            db->setPageDiscussion(*pageA, "thread-b");
            assertEquals("thread-b", *db->getPageDiscussion(*pageA));
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
		
		tester.add("Database::ForumGroup", [](){
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
		
		tester.add("Database::ForumCategory", [](){
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
			categoryA.sourceId = "CATEGORY_A_ID";
			categoryA.title = "Category Title A";
			categoryA.description = "Category Description A";
			
			Database::ID categoryAId = db->createForumCategory(groupAId, categoryA);
			
			assertEquals(categoryAId, db->getForumCategoryId(categoryA.sourceId).value());
			assertEqualsVec({categoryAId}, db->getForumCategories(groupAId));
			assertEqualsVec({}, db->getForumCategories(groupBId));
			
			Database::ForumCategory categoryB;
			categoryB.sourceId = "CATEGORY_B_ID";
			categoryB.title = "Category Title B";
			categoryB.description = "Category Description B";
			
			assertTrue(std::nullopt == db->getForumCategoryId(categoryB.sourceId));
			Database::ID categoryBId = db->createForumCategory(groupAId, categoryB);
			assertEquals(categoryBId, db->getForumCategoryId(categoryB.sourceId).value());
			
			Database::ForumCategory categoryC;
			categoryC.sourceId = "CATEGORY_C_ID";
			categoryC.title = "Category Title C";
			categoryC.description = "Category Description C";
			
			Database::ID categoryCId = db->createForumCategory(groupBId, categoryC);
			
			assertEqualsVec({categoryAId, categoryBId}, db->getForumCategories(groupAId));
			assertEqualsVec({categoryCId}, db->getForumCategories(groupBId));
			
			assertEquals(categoryA.sourceId, db->getForumCategory(categoryAId).sourceId);
			assertEquals(categoryA.title, db->getForumCategory(categoryAId).title);
			assertEquals(categoryA.description, db->getForumCategory(categoryAId).description);
			
			assertEquals(categoryB.sourceId, db->getForumCategory(categoryBId).sourceId);
			assertEquals(categoryB.title, db->getForumCategory(categoryBId).title);
			assertEquals(categoryB.description, db->getForumCategory(categoryBId).description);
			
			assertEquals(categoryC.sourceId, db->getForumCategory(categoryCId).sourceId);
			assertEquals(categoryC.title, db->getForumCategory(categoryCId).title);
			assertEquals(categoryC.description, db->getForumCategory(categoryCId).description);
			
			Database::eraseDatabase(std::move(db));
		});
		
		tester.add("Database::ForumThread", [](){
			std::unique_ptr<Database> db = Database::connectToDatabase(Config::getTestingDatabaseName());
			db->cleanAndInitDatabase();
			
			Database::ForumGroup group;
			group.title = "Group";
			Database::ID groupId = db->createForumGroup(group);
			
			Database::ForumCategory category;
			category.sourceId = "CATEGORY_ID";
			category.title = "Category";
			Database::ID categoryId = db->createForumCategory(groupId, category);
			
			assertEqualsVec({}, db->getForumThreads(categoryId));
			Database::ForumThread forumThreadA;
			forumThreadA.sourceId = "THREAD_A_ID";
			forumThreadA.parent = categoryId;
			forumThreadA.title = "Title A";
			forumThreadA.description = "Description A";
			forumThreadA.timeStamp = 20;
			
			assertEquals(0, db->getNumberOfForumThreads(categoryId));
			assertTrue(std::nullopt == db->getForumThreadId(forumThreadA.sourceId));
			Database::ID forumThreadAId = db->createForumThread(forumThreadA);
			assertEquals(forumThreadAId, *db->getForumThreadId(forumThreadA.sourceId));
			assertEquals(1, db->getNumberOfForumThreads(categoryId));
			
			Database::ForumThread forumThreadB = db->getForumThread(forumThreadAId);
			assertEquals(forumThreadA.sourceId, forumThreadB.sourceId);
			assertEquals(forumThreadA.parent, forumThreadB.parent);
			assertEquals(forumThreadA.title, forumThreadB.title);
			assertEquals(forumThreadA.description, forumThreadB.description);
			assertEquals(forumThreadA.timeStamp, forumThreadB.timeStamp);
			
			assertEqualsVec({forumThreadAId}, db->getForumThreads(categoryId));
			
			assertEqualsVec({}, db->getForumReplies(forumThreadAId));
			assertEquals(0, db->getNumberOfForumReplies(forumThreadAId));
			
			Database::ForumPost postA;
			postA.parentThread = forumThreadAId;
			postA.title = "Post Title A";
			postA.content = "Post Content A";
			postA.timeStamp = 31;
			Database::ID postAId = db->createForumPost(postA);
			assertEqualsVec({postAId}, db->getForumReplies(forumThreadAId));
			assertEquals(1, db->getNumberOfForumReplies(forumThreadAId));
			
			Database::ForumPost postB = db->getForumPost(postAId);
			assertEquals(postA.parentThread, postB.parentThread);
			assertTrue(postA.parentPost == postB.parentPost);
			assertEquals(postA.title, postB.title);
			assertEquals(postA.content, postB.content);
			assertEquals(postA.timeStamp, postB.timeStamp);
			
			assertEqualsVec({}, db->getForumReplies(forumThreadAId, postAId));
			assertEquals(0, db->getNumberOfForumReplies(forumThreadAId, postAId));
			
			postB.title = "Post Title B";
			postB.parentPost = postAId;
			Database::ID postBId = db->createForumPost(postB);
			
			assertEqualsVec({postBId}, db->getForumReplies(forumThreadAId, postAId));
			assertEquals(1, db->getNumberOfForumReplies(forumThreadAId, postAId));
			assertEqualsVec({}, db->getForumReplies(forumThreadAId, postBId));
			assertEquals(0, db->getNumberOfForumReplies(forumThreadAId, postBId));
			
			assertEqualsVec({postAId}, db->getForumReplies(forumThreadAId));
			forumThreadA.sourceId = "NEW_ID";
			forumThreadA.title = "NEW TITLE!";
			db->resetForumThread(forumThreadAId, forumThreadA);
			assertEqualsVec({}, db->getForumReplies(forumThreadAId));
			assertEquals(0, db->getNumberOfForumReplies(forumThreadAId));
			assertEquals(forumThreadA.sourceId, db->getForumThread(forumThreadAId).sourceId);
			assertEquals(forumThreadA.title, db->getForumThread(forumThreadAId).title);
			
			Database::eraseDatabase(std::move(db));
		});
	}
}
















