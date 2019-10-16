

# Internal MonogoDB Database Map:
## pages:
- _id: <pageId>
- name: <pageName>
- parent: <parentPageId> (null if no parent)
- discussion: <discussionId>
- tags:
  - 0: <tag>
  - ...
- files:
  - 0: <fileId>
  - ...
- revisions
  - 0: <revisionId>
  - ...

## revisions:
- _id: <revisionId>
- title: <revisionTitle>
- authorId: <authorId> (null if author is deleted)
- timeStamp: <timeStamp>
- changeMessage: <changeMessage>
- changeType: <changeType> (possible values not yet defined)
- sourceCode: <markdownSourceCode>

## files:
- _id: <fileId>
- name: <fileName>
- description: <fileDescription>
- timeStamp: <timeStamp>
- authorId: <fileUploaderId> (null if account is deleted)

## forumGroups:
- _id: <forumGroupId>
- title: <groupTitle>
- description: <groupDescription>
- categories
  - 0: <forumCategoryId>
  - ...
	
## forumCategories:
- _id: <forumCategoryId>
- title: <categoryTitle>
- description: <categoryDescription>

## threads:
- _id: <threadId>
- categoryId: <threadCategoryId>
- authorId: <threadAuthorId> (null if the account is deleted, false if created by the website itself)
- timeStamp: <timeStamp>
- title: <threadTitle>
- description: <threadDescription>
- posts:
  - 0: <postId>
  - ...

## posts:
- _id: <postId>
- authorId: <authorId> (null if account is deleted)
- timeStamp: <timeStamp>
- title: <postTitle>
- sourceCode: <markdownSourceCode>
- posts:
  - 0: <postId>
  - ...

# Scraper Output File Map:
- pageList.json
- forumCategories.json
- threadList.json
- pages
  - <pageId>
    - data.json
	- files
	  - <fileId>
	  - ...
  - ...
- threads
  - <threadId>
	- data.json
  - ...

# Scraper Data File Layouts:

## pageList.json:
- 0: <pageId>
- ...
	
## pages/<pageId>/data.json
- id: <pageId>
- name: <pageName>
- parent: <parentPageId> ("" if no parent)
- discussionId: <discussionThreadId>
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

## forumCategories.json
- groups
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

## threadList.json:
- 0: <threadId>
- ...
	  
## pages/<threadId>/threads.json
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





















