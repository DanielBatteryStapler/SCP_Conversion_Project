# The SCP Conversion Project
*This is completely unofficial and has not been approved or backed by the SCP Wiki or its staff*

The SCP Conversion Project, which so-far only has one member, is dedicated to converting and providing the SCP Wiki (http://www.scp-wiki.net/) in different formats, which currently consist of PDF and a simplified version of the original HTML. This project has the secondary goal of preparing for the event that Wikidot is lost by archiving all of the original article source-code and images and providing tools that can be used to convert what the authors have written into what the readers see without relying on Wikidot or any Wikidot code so that writting can continue even with the loss of Wikidot.

## The PDFs
If you just want a quick download of the entire site in pdf form, you can find the full ZIP on my Google Drive: https://drive.google.com/drive/folders/1nC8dAtsHCApuD23sDExwot_QM4lKUqQL?usp=sharing

Last Updated At: 01-06-2018 02:51:18 UTC

I will try to update this in a somewhat regular fashion.

## Pages with Errors
There are two categories of errors that a page can have:
1. They have messed up formatting or are just missing content, these should be reported on this github
1. They are completely missing from the zip file entirely, these are already known, and I need help with them

Due to how my code works, it is more strict on syntax correctness than WikiDot, which is not a problem because the majority of articles are correctly formatted. But, some articles have formatting syntax inside of them that is incorrect, even according to WikiDot, but WikiDot is much more forgiving about this and will render them mostly correct anyways. My code cannot replicate this behavior due to the fact that the syntax is technically wrong and WikiDot's behavior is inconsistent. I have a solution for this, though, but I cannot do it myself. In the "pagesWithErrors.txt" file, I have made a list of all of the pages that have actual syntactic errors with a short description of what the error is. If somebody could fix the errors in these pages and let me know, I can redownload them, create the PDFs, for them and then the ZIP file can finally become complete. If you are able to help, but you cannot understand where or what the error is because my description in not understandable, just message me and I can give a full explination of what the problem is — some of my descriptions are a bit less than helpful.

Basically, somebody who is able to help out with fixing pages errors should read the "pagesWithError.txt" file and fix all of the errors because I am unable to.

--------------

The rest of this is technical information and can be disregarded

## The Converter
Due to the current implementation of the program I wrote, it is not easy to use for non-programmers. The sourcecode and codeblocks project files can be found in the "sourceCode" folder, and if you know enough to get that to compile and run, good! It has a few basic command line arguments that can be used to download and convert pages. I would like to distribute a working executable version — instead of just dumping the source code and telling you to do it yourself — but I am running Debian Linux, and my executables won't work on Windows. If I can get a working Windows version, or if somebody wants to help me out, I will update the project to include it and more instructions on how to use it. But, for the people who can get the code to work, I'll just give the following quick explanation:

The "ScpScraper" data folder should look like this:
```
ScpScraper/
  wkhtmltopdf
  ScpScraper
  websitePages/
    /*everything downloaded will be put here*/
```
The "wkhtmltopdf" executable can be obtained on it's website https://wkhtmltopdf.org/

The "ScpScraper" program (specifically the "performFullToolChain" option) works by:
1. (the "downloadPage" option only does this step) Downloading the WikiDot source code to an article, making a folder to put it in, and saving it in that folder (e.g. "websitePages/scp-262/scp-262.wikidotSource")
1. (the "convertToHtml" option only does this step) Converting the WikiDot source code into the two different types of HTML and places them into the same folder as the WikiDot source file and download any connected images into that same folder (e.g. "websitePages/scp-262/scp-262.pdfVersion.html" and "websitePages/scp-262/scp-262.websiteVersion.html" and "websitePages/scp-262/<imageFile>") The "websiteVersion" is more verbose and shows "unsupported section tag" errors(such has the "voting module" that appears in every page) and has interactive elements, such as collapsibles and tabviews. The "pdfVersion" suppresses the "unsupported section tag" errors and has all collapsibles and tabviews expanded out, instead of having them as interactive elements.
1. (the "convertToPdf" option only does this step) Converting the "pdfVersion.html" into a pdf file using the "wkhtmltopdf" executable found in the data folder. This will also embed any images that it previously downloaded into the pdf file. This will result in a pdf file in the page's folder with the same name as the page (e.g. "websitePages/scp-262/scp-262.pdf")

You are free to experiment with trying to format and convert locally created articles, instead of ones downloaded off the site, by creating the correct page folder and WikiDot source code file and then just running the "convertToHtml" action with the folder name. Keep in mind, the "SCP_Conversion_Project_Page_Info_Header" is not completely optional and the "Page_Title" must be in the WikiDot source code file(you can use a downloaded article as reference on how the header should be formatted).
 




