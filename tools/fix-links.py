import os
import sys
import re
import json
import click

existingLinks = dict()

def GetRootDir():
    myScriptDir = os.path.dirname(os.path.realpath(__file__))
    rootDir = os.path.join(myScriptDir, '..')
    return os.path.realpath(rootDir)

def HasExtension(fileName, extension):
    (root, ext) = os.path.splitext(fileName)
    return ext == extension

def EmptyString(s):
    return s is None or not s

def EmptyLine(s):
    return not EmptyString(s) and s == "\n"

def ConvertNameToLink(name):
    result = ''.join([c.upper() if c.isalnum() else '_' for c in name if (c.isalnum() or c == ' ' or c == '_' or c == '-' or c == '+')])
    return result

class MarkDownLinkFixer:
    path = ""
    inFile = None
    outFile = None
    lineNumber = 0
    currentSectionLevel = 0
    linkNames = []
    dirty = False
    inCodeSection = False
    logfile = None
    verbose = False

    def __init__(self, logOutput, verbose):
        self.path = ""
        self.inFile = None
        self.outFile = None
        self.lineNumber = 0
        self.currentSectionLevel = 0
        self.linkNames = []
        self.dirty = False
        self.inCodeSection = False
        self.logFile = logOutput
        self.verbose = verbose

    def log(self, str):
        print(str, file=self.logFile)

    def AddLinkName(self, name):
        if self.currentSectionLevel > len(self.linkNames):
            self.linkNames.append(name)
        else:
            self.linkNames[self.currentSectionLevel-1] = name

    def CheckAndAddLink(self, line):
        matcherCodeBegin = re.compile(r'^```\w*$')
        matcherCodeEnd = re.compile(r'^```$')
        if not matcherCodeEnd.match(line) is None:
            self.inCodeSection = not self.inCodeSection
            return line
        if not matcherCodeBegin.match(line) is None:
            self.inCodeSection = True
            return line

        if self.inCodeSection:
            return line
        
        if line[0] == '#':
            characterMatchString = r'[\w_().:/,<>\-+]'
            characterLinkMatchString = r'[\w_]'
            matcherNoLink = re.compile(r'^([#]+)\s+('+characterMatchString+r'+(\s+'+characterMatchString+r'+)*)\s*$')
            matchesNoLink = matcherNoLink.findall(line)
            matcherWithLink = re.compile(r'^([#]+)\s+('+characterMatchString+r'+(\s+'+characterMatchString+r'+)*)\s+\{#('+characterLinkMatchString+r'+)\}\s*$')
            matchesWithLink = matcherWithLink.findall(line)
            if (len(matchesNoLink) == 0 and len(matchesWithLink) == 0):
                raise ValueError(f"{self.path}:{self.lineNumber} NO MATCHES: {line.strip()}")
            if (len(matchesWithLink) > 1) or (len(matchesNoLink) > 1):
                raise ValueError(f"{self.path}:{self.lineNumber} TOO MANY MATCHES: {line.strip()}")
            
            if len(matchesNoLink) == 1:
                self.log(f"{self.path}:{self.lineNumber} MISSING LINK: {line.strip()}")
                match = matchesNoLink[0]
                newLevel = len(match[0])
                if (newLevel > self.currentSectionLevel) and (newLevel != self.currentSectionLevel + 1):
                    raise ValueError(f"{self.path}:{self.lineNumber} Incorrect section currentSectionLevel {newLevel}, expected <{self.currentSectionLevel} or {self.currentSectionLevel+1}")
                else:
                    sectionName = match[1]
                    self.currentSectionLevel = newLevel
                    self.AddLinkName(ConvertNameToLink(sectionName))
                    fullLinkName = '_'.join(self.linkNames[:self.currentSectionLevel])
                    if fullLinkName in existingLinks.keys():
                        raise ValueError(f"{self.path}:{self.lineNumber} PANIC: link {fullLinkName} already defined")
                    existingLinks[fullLinkName] = f"{self.path}:{self.lineNumber}"
                    line = f"{match[0]} {match[1]} {'{'}#{fullLinkName}{'}'}\n"
                    self.dirty = True

                return line

            if len(matchesWithLink) == 1:
                if self.verbose:
                    self.log(f"{self.path}:{self.lineNumber} MATCH: {line.strip()}")
                match = matchesWithLink[0]
                newLevel = len(match[0])
                if (newLevel > self.currentSectionLevel) and (newLevel != self.currentSectionLevel +1):
                    raise ValueError(f"{self.path}:{self.lineNumber} Incorrect section currentSectionLevel {newLevel}, expected <{self.currentSectionLevel} or {self.currentSectionLevel+1}")
                else:
                    sectionName = match[1]
                    self.currentSectionLevel = newLevel
                    self.AddLinkName(ConvertNameToLink(sectionName))
                    fullLinkName = '_'.join(self.linkNames[:self.currentSectionLevel])
                    if fullLinkName in existingLinks.keys():
                        raise ValueError(f"{self.path}:{self.lineNumber} PANIC: link {fullLinkName} already defined")
                    existingLinks[fullLinkName] = f"{self.path}:{self.lineNumber}"
                    if (match[3] != fullLinkName):
                        line = f"{match[0]} {match[1]} {'{'}#{fullLinkName}{'}'}\n"
                        self.dirty = True
        return line

    def CreateLinksForSections(self, path):
        with open(path, 'r') as self.inFile:
            outPath = path + '.new'
            with open(outPath, 'w') as self.outFile:
                self.path = path
                self.currentSectionLevel = 0
                self.dirty = False
                self.inCodeSection = False
                self.lineNumber = 0
                line = self.inFile.readline()
                while not EmptyString(line):
                    self.lineNumber = self.lineNumber + 1
                    if not EmptyLine(line):
                        line = self.CheckAndAddLink(line)
                    self.outFile.write(line)
                    line = self.inFile.readline()
        if self.dirty:
            os.remove(path)
            os.rename(outPath, path)
            self.dirty = False
        else:
            os.remove(outPath)
        return

    def CreateLinksForMarkdownSections(self):
        for (root,dirs,files) in os.walk(GetRootDir(),topdown=True):
            # self.log(f"Directory path: {root}")
            mdFiles = [x for x in files if HasExtension(x, '.md')]
            for file in mdFiles:
                self.CreateLinksForSections(os.path.join(root, file))
        return True

class PageFinder:
    path = ""
    inFile = None
    lineNumber = 0
    logfile = None

    def __init__(self, logOutput):
        self.path = ""
        self.inFile = None
        self.lineNumber = 0
        self.logFile = logOutput

    def log(self, str):
        print(str, file=self.logFile)

    def UpdateLink(self, line):
        matcherPage = re.compile(r'[\\@]page\s+([\w_]+)')
        matchPage = matcherPage.findall(line)
        for m in matchPage:
            linkName = m
            if linkName in existingLinks.keys():
                raise ValueError(f"{self.path}:{self.lineNumber} PANIC: link {linkName} already defined")
            existingLinks[linkName] = f"{self.path}:{self.lineNumber}"
            #self.log(f"{self.path}:{self.lineNumber} Add link {linkName}")

    def UpdateLinksForFile(self, path):
        with open(path, 'r') as self.inFile:
            self.path = path
            self.lineNumber = 0
            line = self.inFile.readline()
            while not EmptyString(line):
                self.lineNumber = self.lineNumber + 1
                if not EmptyLine(line):
                    line = self.UpdateLink(line)
                line = self.inFile.readline()
        return

    def UpdateLinks(self):
        for (root,dirs,files) in os.walk(GetRootDir(),topdown=True):
            # self.log(f"Directory path: {root}")
            mdFiles = [x for x in files if HasExtension(x, '.md')]
            for file in mdFiles:
                self.UpdateLinksForFile(os.path.join(root, file))
        return True

class MarkDownLinkChecker:
    path = ""
    inFile = None
    lineNumber = 0
    logfile = None
    inCodeSection = False

    def __init__(self, logOutput):
        self.path = ""
        self.inFile = None
        self.lineNumber = 0
        self.logFile = logOutput
        self.inCodeSection = False
        self.inComment = False

    def log(self, str):
        print(str, file=self.logFile)

    def CheckLink(self, line):
        matcherCodeBegin = re.compile(r'^```\w*$')
        matcherCodeEnd = re.compile(r'^```$')
        matcherCommentBegin = re.compile(r'<!--')
        matcherCommentEnd = re.compile(r'-->')
        matcherRef = re.compile(r'[\\@]ref\s+([\w_]+)')
        matcherSubPage = re.compile(r'[\\@]subpage\s+([\w_]+)')
        matcherMarkDownRef = re.compile(r'\[[^\]]+\]\(#([\w_]+)\)')
        matchRef = matcherRef.findall(line)
        if not matcherCodeEnd.match(line) is None:
            self.inCodeSection = not self.inCodeSection
            return
        if not matcherCodeBegin.match(line) is None:
            self.inCodeSection = True
            return

        if not matcherCommentEnd.match(line) is None:
            self.inComment = False
            return
        if not matcherCommentBegin.match(line) is None:
            self.inComment = True
            return

        if self.inCodeSection:
            return
        if self.inComment:
            return
        for m in matchRef:
            linkName = m
            if not linkName in existingLinks.keys():
                self.log(f"{self.path}:{self.lineNumber} Link {linkName} not found")
        matchSubPage = matcherSubPage.findall(line)
        for m in matchSubPage:
            linkName = m
            if not linkName in existingLinks.keys():
                self.log(f"{self.path}:{self.lineNumber} Link {linkName} not found")
        matchMarkDownRef = matcherMarkDownRef.findall(line)
        for m in matchMarkDownRef:
            linkName = m
            if not linkName in existingLinks.keys():
                self.log(f"{self.path}:{self.lineNumber} Link {linkName} not found")

    def CheckLinksForFile(self, path):
        with open(path, 'r') as self.inFile:
            self.path = path
            self.lineNumber = 0
            line = self.inFile.readline()
            while not EmptyString(line):
                self.lineNumber = self.lineNumber + 1
                if not EmptyLine(line):
                    line = self.CheckLink(line)
                line = self.inFile.readline()
        return

    def CheckLinks(self):
        for (root,dirs,files) in os.walk(GetRootDir(),topdown=True):
            # self.log(f"Directory path: {root}")
            mdFiles = [x for x in files if HasExtension(x, '.md')]
            for file in mdFiles:
                self.CheckLinksForFile(os.path.join(root, file))
        return True

class ContentLinkChecker:
    path = ""
    inFile = None
    lineNumber = 0
    logfile = None

    def __init__(self, logOutput):
        self.path = ""
        self.inFile = None
        self.lineNumber = 0
        self.logFile = logOutput

    def log(self, str):
        print(str, file=self.logFile)

    def CheckLink(self, line):
        matcherMarkDownRef = re.compile(r'\[[^\]]+\]\(([^\)]+)\)')
        matcherImageRef = re.compile(r'\<img\ssrc=\"')
        matcherURL = re.compile(r'^http[s]?\:\/\/[\w\d\-\+\.\~\#\_\&\/=\?/]*$')
        matcherInternalRef = re.compile(r'^#[\w\d\_]*$')
        matcherPDF = re.compile(r'^pdf\/')
        matcherImages = re.compile(r'^images\/')
        matchRef = matcherMarkDownRef.findall(line)
        for m in matchRef:
            linkName = m
            if not matcherURL.match(linkName) is None:
                pass
                #self.log(f"{self.path}:{self.lineNumber} URL {linkName}")
            else:
                if not matcherInternalRef.match(linkName) is None:
                    pass
                    #self.log(f"{self.path}:{self.lineNumber} Ref {linkName}")
                else:
                    if not matcherPDF.match(linkName) is None:
                        if not os.path.exists(os.path.join(GetRootDir(), 'doc', linkName)):
                            self.log(f"{self.path}:{self.lineNumber} PDF {linkName} does NOT exist")
                    else:
                        if not matcherImages.match(linkName) is None:
                            if not os.path.exists(os.path.join(GetRootDir(), 'doc', linkName)):
                                self.log(f"{self.path}:{self.lineNumber} Image {linkName} does NOT exist")
                        else:
                            filename = linkName.split('#', 1)[0]
                            if not os.path.exists(os.path.join(os.path.dirname(self.path), filename)):
                                self.log(f"{self.path}:{self.lineNumber} Link {filename} does NOT exist")

                    #self.log(f"{self.path}:{self.lineNumber} Link {linkName}")

    def CheckLinksForFile(self, path):
        with open(path, 'r') as self.inFile:
            self.path = path
            self.lineNumber = 0
            line = self.inFile.readline()
            while not EmptyString(line):
                self.lineNumber = self.lineNumber + 1
                if not EmptyLine(line):
                    line = self.CheckLink(line)
                line = self.inFile.readline()
        return
    def CheckLinks(self):
        for (root,dirs,files) in os.walk(GetRootDir(),topdown=True):
            # self.log(f"Directory path: {root}")
            mdFiles = [x for x in files if HasExtension(x, '.md')]
            for file in mdFiles:
                self.CheckLinksForFile(os.path.join(root, file))
        return True

@click.command()
@click.option('--xmloutput', default='links.json', help='Output links to XML')
@click.option('--logfile', default='fix-links.log', help='Log output file')
@click.option('--verbose', default=False, help='Generate more detailed logging')
def FixLinks(xmloutput, logfile, verbose):
    logOutput = sys.stdout if logfile == None else open(logfile, 'w')
    print('Create links for markdown sections')
    markDownLinkFixer = MarkDownLinkFixer(logOutput, verbose)
    if not markDownLinkFixer.CreateLinksForMarkdownSections():
        print("Fail")
        exit(1)

    print('Create links for page entries')
    pageFinder = PageFinder(logOutput)
    if not pageFinder.UpdateLinks():
        print("Fail")
        exit(1)

    with open(f"{os.path.join(GetRootDir(), xmloutput)}", "w") as f:
        json.dump(existingLinks, f, indent=4)

    print('Check markdown links')
    markDownLinkChecker = MarkDownLinkChecker(logOutput)
    if not markDownLinkChecker.CheckLinks():
        print("Fail")
        exit(1)

    print('Check pdf and image links')
    contentLinkChecker = ContentLinkChecker(logOutput)
    if not contentLinkChecker.CheckLinks():
        print("Fail")
        exit(1)
    
if __name__ == '__main__':
    FixLinks()