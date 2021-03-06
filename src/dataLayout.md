
# PDF File Map:
- pdfData.json
- collection
  - <pageName>.pdf
  - ...
- temp
  - ...
# PDF File Layouts:
## pdfData.json
- pages
  - 0
    - pageName: <pageName>
    - revisionIndex: <revisionIndex>
  - ...

# MySQL Database Map:
full table definitions can be found in src/ScpArchive/Database/Database.cpp

# Scraper Data File Map:
- batchData.json
- authors.json
- batches
  - <batchId>
    - batch.json
	- pages
	  - <pageName>
	    - data.json
	    - files
	  	  - ...
	  - ...
	- threads
	  - <threadId>
	    - data.json
	  - ...
  - ...
- threads
  - <threadId>
	- data.json
  - ...

# Scraper Data File Layouts:
## batchData.json
- availableBatches
  - 0: <batchId>
- appliedBatches
  - 0: <batchId>
  - ...
- batchErrors
  - 0
    - error: <errorString>
    - batchId: <batchId>
  - ...
	
## batches/<batchId>/batch.json
### initial-type
used for initial database creation, generated by ScpScraper
- type: "initial"
- timeStamp: <batchStartTimeStamp>
- pageList
	- 0: <pageName>
	- ...
- forumGroups
  - 0
	- title: <groupTitle>
	- description: <groupDescription>
	- categories
	  - 0
		- id: <categoryId>
		- title: <categoryTitle>
		- description: <categoryDescription>
	  - ...
  - ...
- threadList
	- 0: <theadId>
	- ...
	
### diff-type
used to keep the database up to date with the actual wiki, generated by ScpScraper
- type: "diff"
- timeStamp: <batchStartTimeStamp>
- pageList
	- 0: <pageName>
	- ...
- threadList
	- 0: <theadId>
	- ...

### user-type
used by a user attempted to insert special data into the thread that is not on the actual wiki
notice the lack of timestamp, as this does not have an impact on the timeline of the batch diffs
- type: "user"
- pageList
	- 0: <pageName>
	- ...
- threadList
	- 0: <theadId>
	- ...

## pages/<pageName>/data.json when page does not exist
- nonExistent: true

## pages/<pageName>/data.json
- id: <pageId>
- name: <pageName>
- parent: <parentPageId> ("" if no parent)
- discussionId: <discussionThreadId> ("" if no discussion)
- tags
  - 0: <tag>
  - ...
- votes
  - 0
	- authorId: <idOfVoter> ("deleted" if account is deleted)
	- voteType: <voteType> (true if upvote, false if downvote)
  - ...
- files
  - 0
	- id: <fileId>
	- name: <fileName>
	- description: <fileDescription>
	- url: <urlOfLocationOnWikidot>
	- timeStamp: <timeStamp>
	- authorId: <fileUploaderId> ("deleted" if account is deleted)
  - ...
- revisions
  - 0
	- id: <revisionId>
	- title: <revisionTitle>
	- timeStamp: <timeStamp>
	- authorId: <revisionAuthorId> ("deleted" if account is deleted)
	- changeMessage: <changeMessage>
	- changeType: <changeType> (possible values are defined by Wikidot)
	- sourceCode: <wikidotSourceCode>
  - ...


## threads/<threadId>/data.json when thread does not exist
- nonExistent: true

## threads/<threadId>/data.json
- id: <threadId>
- categoryId: <threadCategoryId>
- authorId: <threadAuthorId> ("deleted" if the account is deleted, "wikidot" if created by wikidot)
- timeStamp: <timeStamp>
- title: <threadTitle>
- description: <threadDescription>
- posts:
  - 0
	- postId: <postId>
	- authorId: <authorId> ("deleted" if account is deleted)
	- timeStamp: <timeStamp>
	- title: <postTitle>
	- content: <postContent>
	- posts:
	  - 0: ...
	  - ...
  - ...

## authors.json
- 0
  - id: <authorId>
  - name: <authorName>
- ...
















