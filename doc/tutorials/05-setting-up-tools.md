# Tutorial 05: Setting up tools {#TUTORIAL_05_SETTING_UP_TOOLS}

@tableofcontents

## New tutorial setup {#TUTORIAL_05_SETTING_UP_TOOLS_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/05-setting-up-tools`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_05_SETTING_UP_TOOLS_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- an application `output/<platform>/Debug/bin/05-setting-up-tools.elf`
- an image in `deploy/Debug/05-setting-up-tools-image` for each built platform

## An intermezzo: tools {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS}

This chapter is going to be an intermezzo, before we start to actually write more code.

We're going to discuss a couple of tools you may want to add for documentation, code quality and code formatting.

This tutorial will not add any code, the changes will be reflected in the overall project structure, so also no new tutorial output will be generated.

### Documenting code {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE}

Now that we have already created a bit of code, let's focus a bit on documentation.
If you're not interested in this part, feel free to skip to the next tutorial.
However, especially with development that needs a lot of detailed knowledge, it might be handy to have a clear, searchable source of information.

If well configured, `Doxygen` can be just that.
After running a script to collect all information configured for `Doxygen`, it can generate a HTML tree for you, that you can copy to a web server if desired, so you always have a source of information on all the platforms.
It can also generate other forms of documentation, such as LaTeX, XCode docset, Windows help files (CHM), QT help files, etc.

`Doxygen` is an open source tool to collect documentation from your code and other documents, and present it in a tree-like, searchable web structure.

For an example of how this can look see below.

<img src="images/doxygen-sample.png" alt="Doxygen sample" width="800"/>

#### Installing Doxygen {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_INSTALLING_DOXYGEN}

To install `Doxygen`, go to their [site](https://www.doxygen.nl/index.html), and click on Download. Then select the platform your wish to download for.
You can either download a tarball, that you can install somewhere, or for Windows, download the installer. On Linux, you can simply install the package.
We'll decide to install the package in both Windows and Linux, as in that case, the location is clear.

##### Windows {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_INSTALLING_DOXYGEN_WINDOWS}

We will install the latest `Doxygen` available at the moment, 1.12.0.
Download the installer [here](https://www.doxygen.nl/files/doxygen-1.12.0-setup.exe), and run it.
You might be warned that `Doxygen` is not commonly downloaded, you can safely ignore this, and decide to keep the file in your browser, and if Windows Defender pops up, again you can safely ignore this and run the file.

<img src="images/doxygen-install-1.png" alt="Installing Doxygen 1" width="500"/>

Click _I accept the agreement_, click __Next__

<img src="images/doxygen-install-2.png" alt="Installing Doxygen 2" width="500"/>

Click __Next__

<img src="images/doxygen-install-3.png" alt="Installing Doxygen 3" width="500"/>

Click __Install__

<img src="images/doxygen-install-4.png" alt="Installing Doxygen 4" width="500"/>

Click __Next__

<img src="images/doxygen-install-5.png" alt="Installing Doxygen 5" width="500"/>

Click __Finish__

To test, open a command line prompt, and execute:
```bat
doxygen -v
```

The command should execute without error, and print the version of `Doxygen`:

```text
1.12.0 (c73f5d30f9e8b1df5ba15a1d064ff2067cbb8267)
```

##### Linux {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_INSTALLING_DOXYGEN_LINUX}

On Linux, simply run apt to install (the latest version depends on your distribution, in my case it was 1.9.4):

```bash
sudo apt install doxygen
```

```text
Reading package lists... Done
Building dependency tree... Done
The following additional packages will be installed:
  libclang-cpp14 libclang1-14 libllvm14
Suggested packages:
  doxygen-latex doxygen-doc doxygen-gui
The following NEW packages will be installed:
  doxygen libclang-cpp14 libclang1-14 libllvm14
0 upgraded, 4 newly installed, 0 to remove and 0 not upgraded.
Need to get 43.7 MB of archives.
After this operation, 219 MB of additional disk space will be used.
Do you want to continue? [Y/n] y
Get:1 http://ftp.nl.debian.org/debian bookworm/main amd64 libllvm14 amd64 1:14.0.6-12 [21.8 MB]
Get:2 http://ftp.nl.debian.org/debian bookworm/main amd64 libclang-cpp14 amd64 1:14.0.6-12 [11.1 MB]
Get:3 http://ftp.nl.debian.org/debian bookworm/main amd64 libclang1-14 amd64 1:14.0.6-12 [6,157 kB]
Get:4 http://ftp.nl.debian.org/debian bookworm/main amd64 doxygen amd64 1.9.4-4 [4,611 kB]
Fetched 43.7 MB in 3s (14.3 MB/s)
Selecting previously unselected package libllvm14:amd64.
(Reading database ... 238270 files and directories currently installed.)
Preparing to unpack .../libllvm14_1%3a14.0.6-12_amd64.deb ...
Unpacking libllvm14:amd64 (1:14.0.6-12) ...
Selecting previously unselected package libclang-cpp14.
Preparing to unpack .../libclang-cpp14_1%3a14.0.6-12_amd64.deb ...
Unpacking libclang-cpp14 (1:14.0.6-12) ...
Selecting previously unselected package libclang1-14.
Preparing to unpack .../libclang1-14_1%3a14.0.6-12_amd64.deb ...
Unpacking libclang1-14 (1:14.0.6-12) ...
Selecting previously unselected package doxygen.
Preparing to unpack .../doxygen_1.9.4-4_amd64.deb ...
Unpacking doxygen (1.9.4-4) ...
Setting up libllvm14:amd64 (1:14.0.6-12) ...
Setting up libclang1-14 (1:14.0.6-12) ...
Setting up libclang-cpp14 (1:14.0.6-12) ...
Setting up doxygen (1.9.4-4) ...
Processing triggers for man-db (2.11.2-2) ...
Processing triggers for libc-bin (2.36-9+deb12u3) ...
```

To test, open a command line prompt, and execute:
```bash
doxygen -v
```

The command should execute without error, and print the version of `Doxygen`:

```text
1.9.4
```

#### Installing Graphviz {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_INSTALLING_GRAPHVIZ}

Graphviz (dot) is a visualization tool that will enable `Doxygen` to create diagrams for e.g. inheritance in the generated documentation.

##### Windows {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_INSTALLING_GRAPHVIZ_WINDOWS}

We will install the latest Graphviz available for Windows at the moment, 12.1.0.
Download the installer from their [site](https://graphviz.org/download/), or [here](https://gitlab.com/api/v4/projects/4207231/packages/generic/graphviz-releases/12.1.0/windows_10_cmake_Release_graphviz-install-12.1.0-win64.exe) for the current latest version, and run it.
You might be warned that `Graphviz` is not commonly downloaded, you can safely ignore this, and decide to keep the file in your browser, and if Windows Defender pops up, again you can safely ignore this and run the file.

<img src="images/graphviz-install-1.png" alt="Installing Graphviz 1" width="500"/>

Click __Next>__

<img src="images/graphviz-install-2.png" alt="Installing Graphviz 2" width="500"/>

Click __I Agree__

<img src="images/graphviz-install-3.png" alt="Installing Graphviz 3" width="500"/>

Select _Add Graphviz to the system PATH for all users__. Click __Next>__

<img src="images/graphviz-install-4.png" alt="Installing Graphviz 4" width="500"/>

Click __Next>__

<img src="images/graphviz-install-5.png" alt="Installing Graphviz 5" width="500"/>

Click __Install__

<img src="images/graphviz-install-6.png" alt="Installing Graphviz 6" width="500"/>

Click __Finish__

To test, open a command line prompt, and execute:
```bat
dot -V
```

The command should execute without error, and print the version of Graphviz:

```text
dot - graphviz version 12.1.0 (20240811.2233)
```

##### Linux {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_INSTALLING_GRAPHVIZ_LINUX}

On Linux, simply run apt to install (version dependent on your distribution, in my case 2.43.0):

```bash
sudo apt install graphviz
```

```text
sudo apt install graphviz
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following packages were automatically installed and are no longer required:
  gimp-data libamd2 libbabl-0.1-0 libcamd2 libccolamd2 libcholmod3 libgegl-0.4-0 libgegl-common libgimp2.0 libmetis5 libmng1 libmypaint-1.5-1 libmypaint-common libumfpack5 libwmf-0.2-7
  linux-headers-6.1.0-13-amd64 linux-headers-6.1.0-13-common linux-image-6.1.0-13-amd64
Use 'sudo apt autoremove' to remove them.
Suggested packages:
  graphviz-doc
The following NEW packages will be installed:
  graphviz
0 upgraded, 1 newly installed, 0 to remove and 0 not upgraded.
Need to get 611 kB of archives.
After this operation, 3,139 kB of additional disk space will be used.
Get:1 http://ftp.nl.debian.org/debian bookworm/main amd64 graphviz amd64 2.42.2-7+b3 [611 kB]
Fetched 611 kB in 0s (6,038 kB/s)
Selecting previously unselected package graphviz.
(Reading database ... 237950 files and directories currently installed.)
Preparing to unpack .../graphviz_2.42.2-7+b3_amd64.deb ...
Unpacking graphviz (2.42.2-7+b3) ...
Setting up graphviz (2.42.2-7+b3) ...
Processing triggers for man-db (2.11.2-2) ...
```

To test, open a command line prompt, and execute:
```bash
dot -V
```

The command should execute without error, and print the version of Graphviz:

```text
dot - graphviz version 2.43.0 (0)
```

#### Configuring Doxygen {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_CONFIGURING_DOXYGEN}

`Doxygen` has many options, which are just too much to cover here, so we'll pick out a few relevant ones. The full documentation can be found in [Docs](https://www.doxygen.nl/manual/index.html).

The main part is the configuration file.
You can let `Doxygen` generate a default configuration file for you, that you can then adapt to your wishes.
Let's place this file in the folder `doxygen` in the root of the project.

```bash
mkdir doxygen
doxygen -g doxygen\doxygen.conf
```

```text

Configuration file 'doxygen\doxygen.conf' created.

Now edit the configuration file and enter

  doxygen doxygen\doxygen.conf

to generate the documentation for your project
```

Everything you can change in the configuration file has good documentation, some things might take a bit to fully understand.
Included in this repository is the setup I chose (I kept the original generated one in `doxygen/doxygen-base.conf`. The examples below roughly show the choices I made.
Don't hesitate to play around and see the results.

Some items worth changing:

| Tag                    | Information                                          | Example |
|------------------------|------------------------------------------------------|---------|
| PROJECT_NAME           | The name of your project                             | `"Baremetal"`
| PROJECT_BRIEF          | A short description of the project                   | `"Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices"`
| PROJECT_LOGO           | A logo image for your project                        | `doxygen/logo.png`
| PROJECT_ICON           | An icon for your project                             | `doxygen/logo.ico`
| OUTPUT_DIRECTORY       | The location where Doxygen generates its output (this will output generated html under doxygen/html | `doxygen`
| ALWAYS_DETAILED_SEC    | Will force `Doxygen` to generate a detailed section in all cases, if REPEAT_BRIEF is also set | `YES`
| GROUP_NESTED_COMPOUNDS | Also generate information for nested class or struct | `YES`
| EXTRACT_PRIVATE        | Also generate information for private members of a class or struct | `YES`
| EXTRACT_PRIV_VIRTUAL   | Also generate information for private virtual methods of a class or struct | `YES`
| EXTRACT_STATIC         | Also generate information for static members of a class or struct | `YES`
| EXTRACT_ANON_NSPACES   | Also generate information for anonymous namespaces   | `YES`
| CASE_SENSE_NAMES       | Handles names case sensitive if set                  | `YES`
| WARN_NO_PARAMDOC       | Generate a warning if a parameter is not documented  | `YES`
| WARN_IF_UNDOC_ENUM_VAL | Generate a warning if an enum is not documented      | `YES`
| WARN_LOGFILE           | Write warnings to specified log file (stderr if not set) | `doxygen/doxygen.log`
| INPUT                  | root location to scan for code (either a file or a directory | `code doc README.md LICENSE.MD`
| RECURSIVE              | Search INPUT directory recursively                   | `YES`
| USE_MDFILE_AS_MAINPAGE | Sets a Markdown file as the main page for documentation | `README.md`
| GENERATE_HTML          | If set generates HTML output                         | `YES`
| HTML_HEADER            | Sets a HTML header if desired                        | `doxygen/header.html`
| HTML_FOOTER            | Sets a HTML footer if desired                        | `doxygen/footer.html`
| HTML_STYLESHEET        | Sets a HTML style sheet if desired                   | `doxygen/stylesheet.css`
| HTML_EXTRA_FILES       | Any additional files you wish to add. These can be multiple file types, such as HTML, PDF, icons, etc. End every line except the last with a backslash (\\)|
| HTML_COLORSTYLE_HUE    | Set a color hue for your pages                       | `120`
| HTML_COLORSTYLE_SAT    | Set a color saturation for your pages                | `160`
| GENERATE_TREEVIEW      | If set a tree-like structure in a side panel         | `YES`
| GENERATE_LATEX         | If set generates LaTeX output                        | `NO`
| PREDEFINED             | Any definitions you would like set for source code, much like the -D options for the compiler. Requires ENABLE_PREPROCESSING to be set | `BAREMETAL_RPI_TARGET=3`
| SKIP_FUNCTION_MACROS   | If set ignores function like macros. Requires ENABLE_PREPROCESSING to be set | `NO`
| HIDE_UNDOC_RELATIONS   | If set ignores inheritance and usage of items that are not documented | `NO`
| HAVE_DOT               | Set if you have dot (Graphviz) installed. This will enable generation of e.g. inheritance diagrams in the documentation | `YES`
| STRIP_CODE_COMMENTS    | If set hides comments from code in code blocks in markdown | `NO`

In addition to the settings described in the table above, we make sure to set the `HTML_EXTRA_FILES` to all additional files in our repo.
We don't need to add Markdown (`.md`) files, they will automatically be found if they are in the trees or files specified for `INPUT`.

#### A caveat with Doxygen {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_A_CAVEAT_WITH_DOXYGEN}

Doxygen does have one issue, related to Markdown and images.
In Markdown, we can use relative paths for all links, for example to other Markdown files, but also to say PDF files.
We use the standard link syntax for that:

```text
[Description](../abc.pdf)
```

If documents are exported as part of HTML_EXTRA_FILES, `Doxygen` will copy them to the root folder of the generated HTML.
This means that relative paths do not work.
The only exception is Markdown files themselves, they will be correctly de-referenced.
All full URLs work correctly of course, but often you will want to use local files, and organize them into folders.

To resolved this, we place all images in a single directory `doc/images`, and PDF files in `doc/pdf`, and always use the relative link to the `images` or `pdf` directory, so e.g. `images/xxx.png`.
In every folder that needs links to images, in this case:

- doc (is the actual root folder for images and pdf folders)
- doc/board
- doc/cpu
- doc/tutorials

We make sure to have the `images` and `pdf` directories available.
For the `doc` folder the subfolders `doc/images` and `doc/pdf` are present as they contain the actual files, so that is trivial.
For the other three, we create a symlink to the `doc/images` folder names `images`, and to the `doc/pdf` folder names `pdf`.
This only has to be done once.

##### Windows {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_A_CAVEAT_WITH_DOXYGEN_WINDOWS}

Windows 10 and later support this, but you need to run a command a administrator.

Create the file `tools\create-links.bat`

```bat
pushd doc\tutorials
mklink /D images ..\images
mklink /D pdf ..\pdf
popd

pushd doc\board
mklink /D images ..\images
mklink /D pdf ..\pdf
popd

pushd doc\cpu
mklink /D images ..\images
mklink /D pdf ..\pdf
popd
```

##### Linux {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_A_CAVEAT_WITH_DOXYGEN_LINUX}

On Linux symlinks are common:

Create the file `tools/create-links.sh`

```bash
pushd doc/tutorials
ln -s ../images images
ln -s ../pdf pdf
popd

pushd doc/board
ln -s ../images images
ln -s ../pdf pdf
popd

pushd doc/cpu
ln -s ../images images
ln -s ../pdf pdf
popd
```

#### Running Doxygen {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_RUNNING_DOXYGEN}

Running `Doxygen` is easy, but let's create a script for it.

##### Windows {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_RUNNING_DOXYGEN_WINDOWS}

Create the file `tools\run-doxygen.bat`

```bat
doxygen doxygen\doxygen.conf
robocopy doc\images doxygen\html\images /MIR
robocopy doc\pdf doxygen\html\pdf /MIR
```

Run doxygen:

```bat
tools\run-doxygen.bat
```

```text
Doxygen version used: 1.12.0 (c73f5d30f9e8b1df5ba15a1d064ff2067cbb8267)
Searching for include files...
Searching for example files...
Searching for images...
Searching for dot files...
Searching for msc files...
Searching for dia files...
Searching for files to exclude
Searching INPUT for files to process...
Searching for files in directory D:/Projects/baremetal.github.shadow/code
...
Reading and parsing tag files
Parsing files
Preprocessing D:/Projects/baremetal/code/applications/demo/src/main.cpp...
Parsing file D:/Projects/baremetal/code/applications/demo/src/main.cpp...
...
finalizing index lists...
writing tag file...
Running plantuml with JAVA...
Running dot...
type lookup cache used 4103/65536 hits=34733 misses=4329
symbol lookup cache used 4322/65536 hits=29644 misses=4322
finished...
```

##### Linux {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_RUNNING_DOXYGEN_LINUX}

Create the file `tools/run-doxygen.sh`

```bash
doxygen doxygen/doxygen.conf
rm -rf doxygen/html/images
mkdir -p doxygen/html/images
cp -f doc/images/* doxygen/html/images
rm -rf doxygen/html/pdf
mkdir -p doxygen/html/pdf
cp -f doc/pdf/* doxygen/html/pdf
```

Run doxygen:

```bat
tools/run-doxygen.sh
```

```text
warning: ignoring unsupported tag 'PROJECT_ICON' at line 70, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'MARKDOWN_ID_STYLE' at line 381, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'TIMESTAMP' at line 513, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'WARN_IF_UNDOC_ENUM_VAL' at line 894, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'INPUT_FILE_ENCODING' at line 972, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'FORTRAN_COMMENT_AFTER' at line 1177, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'HTML_COLORSTYLE' at line 1455, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'HTML_CODE_FOLDING' at line 1511, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'HTML_COPY_CLIPBOARD' at line 1522, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'HTML_PROJECT_COOKIE' at line 1531, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'SITEMAP_URL' at line 1671, file doxygen/doxygen.conf
warning: Tag 'GENERATE_SQLITE3' at line 2371 of file 'doxygen/doxygen.conf' belongs to an option that was not enabled at compile time.
         To avoid this warning please remove this line from your configuration file or upgrade it using "doxygen -u", or recompile doxygen with this feature enabled.
warning: Tag 'SQLITE3_OUTPUT' at line 2379 of file 'doxygen/doxygen.conf' belongs to an option that was not enabled at compile time.
         To avoid this warning please remove this line from your configuration file or upgrade it using "doxygen -u", or recompile doxygen with this feature enabled.
warning: Tag 'SQLITE3_RECREATE_DB' at line 2387 of file 'doxygen/doxygen.conf' belongs to an option that was not enabled at compile time.
         To avoid this warning please remove this line from your configuration file or upgrade it using "doxygen -u", or recompile doxygen with this feature enabled.
warning: ignoring unsupported tag 'DOT_COMMON_ATTR' at line 2592, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'DOT_EDGE_ATTR' at line 2601, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'DOT_NODE_ATTR' at line 2609, file doxygen/doxygen.conf
warning: ignoring unsupported tag 'MSCGEN_TOOL' at line 2912, file doxygen/doxygen.conf
Doxygen version used: 1.9.4
Searching for include files...
Searching for example files...
Searching for images...
Searching for dot files...
Searching for msc files...
Searching for dia files...
Searching for files to exclude
Searching INPUT for files to process...
Searching for files in directory /home/rene/repo/baremetal/code
...
Patching output file 33/33
lookup cache used 1097/65536 hits=6511 misses=1138
finished...
```

You may notice that some options are not supported by the Linux version, as it is slightly older than the Windows version.

You'll end up with a directory tree that looks as follows:

- doxygen
  - html: Contains HTML pages, including the main file `index.html`
    - images: All images that are copied from `doc/images`
    - pdf: All PDF files that are copied from `doc/pdf`

#### Viewing Doxygen output {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_VIEWING_DOXYGEN_OUTPUT}

Now open the file `doxygen/html/index.html` in your browser.

<img src="images/doxygen-page.png" alt="Doxygen in browser" width="800"/>

As you can you, for the documentation on the startup code is shown.

We'll be adding more information to our code and other documentation for `Doxygen`, so that the generated info will become richer.

As an (very simple) example, let's update `code/applications/demo/src/main.cpp` with some information.

```cpp
File: code/applications/demo/src/main.cpp
1: /// @file
2: /// Demo application main code
3:
4: /// <summary>
5: /// Demo application main code
6: /// </summary>
7: /// <returns>For now always 0</returns>
8: int main()
9: {
10:     return 0;
11: }
```

You will see some special tags in the source code, as well as in Markdown documents:

- `<summary></summary>`
- `<param name="..."></param>`
- `<returns></returns>`
- `@file`
- `@brief`
- `@param`
- `@return`
- `@ref`

There are some others worth mentioning:

- `@page`
- `@subpage`
- `@todo`
- `@tableofcontents`

Tags can start with `@` or `\` for Doxygen tags, so `@ref` and `\ref` are equivalent.

Comments recognized by `Doxygen` can be in several formats, I have chosen to use C++ comments using three forward slashes

The sections below will show information that is still to be added, to serve as an example of how things can look.

##### file tag {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_VIEWING_DOXYGEN_OUTPUT_FILE_TAG}

The tag `@file` will add the current file to the `Doxygen` index. This way it will show the symbols found linked with the file.
If the `@file` tag is used with a filename, that filename will be linked to symbols found, if left empty the current file is used.

The line after the `@file` tag will be used as a detailed description of the file:

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
...
42: /// @file
43: /// Raspberry Pi Timer
...
```

<img src="images/doxygen-file-list.png" alt="Doxygen file list for a directory" width="800"/>

You can see that the `Timer.h` file is highlighted, meaning there is a link for it.
The link will show the information collected for the file:
- classes
- functions
- usage
- detailed desription

<img src="images/doxygen-file-information.png" alt="Doxygen information for a file" width="800"/>

Clicking on the link `Go to the source code of this file` will bring you to a copy of the code, but with the tags filtered out, making the code easier to read.

<img src="images/doxygen-file-source.png" alt="Doxygen source of a file" width="800"/>

##### brief and summary tags {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_VIEWING_DOXYGEN_OUTPUT_BRIEF_AND_SUMMARY_TAGS}

The tag `@brief` is roughly equal to `<summary></summary>` Both allow an item (class, function, type, method, etc.) to be given a brief description, and a more detailed description.
Comment text will always be just before the item being described.
The comment text in the first lines before an empty comment line form the brief description, the comment lines after an empty comment line (but before a non-comment line) serve as a detailed description:

```cpp
/// <summary>
/// Timer class. For now only contains busy waiting methods
///
/// Note that this class is created as a singleton, using the GetTimer() function.
/// </summary>
class Timer
```

Adding a `@brief` or `<summary></summary>` entry will add the item to the index, and add the information to its page.

<img src="images/doxygen-class-list.png" alt="Doxygen class list" width="800"/>

You can see that the `Timer` class is highlighted just like others, meaning there is a link for it. The difference is that it has a brief description
The link will show the information collected for the class:
- inheritance
- public members, public static members and private members
- public and private attributes
- friend functions
- detailed desription
- detailed documentation on constructors, desctructors, members, friends and attributes

<img src="images/doxygen-class-reference.png" alt="Doxygen class reference" width="800"/>

##### param, return(s) tags {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_VIEWING_DOXYGEN_OUTPUT_PARAM_RETURNS_TAGS}

For every method or function, a `@param` or `<param name="..."></param>` tag can be added for each parameter of the function to describe it.
If the method or function returns a value, a `@return` or `<returns></returns>` tag can be added to describe the return value.

```cpp
...
92:     /// <summary>
93:     /// Wait for msec milliseconds using ARM timer registers (when not using physical counter) or BCM2835 system timer peripheral (when using physical counter). Busy wait
94:     ///
95:     /// The timer used is the ARM builtin timer.
96:     /// </summary>
97:     /// <param name="msec">Wait time in milliseconds</param>
98:     static void WaitMilliSeconds(uint64 msec);
...
110: /// <summary>
111: /// Retrieves the singleton Timer instance. It is created in the first call to this function.
112: /// </summary>
113: /// <returns>A reference to the singleton Timer</returns>
114: Timer& GetTimer();
```

Adding a `@param`, `<param></param>`, `@return` or `<returns></returns>` will add the function to the index, and add the information to its page.

<img src="images/doxygen-class-reference.png" alt="Doxygen class reference" width="800"/>

You can see that the `WaitMilliSeconds()` method and the `GetTimer()` friend function are highlighted just like others (all are documented), meaning there is a link for it.
Any method not described will appear in black and have no link.

The link will show the information collected for the class:
- the signature of the method
- description of parameters and return value
- the brief and detailed description

<img src="images/doxygen-function-reference.png" alt="Doxygen function reference" width="800"/>

##### ref tag {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_VIEWING_DOXYGEN_OUTPUT_REF_TAG}

A ref tag creates a link to another item in the repository, or a URL to an external document.

```cpp
/// @brief Enable IRQs. Clear bit 1 of DAIF register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_DAIF_REGISTER
```

In Markdown files, this is done using `[description](link)`

```
Configuration of the project is described later in [Setting up a project](02-setting-up-a-project.md).

See [Running using Netboot](#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_RUNNING_USING_NETBOOT).

Create a network boot SD card, e.g. using [CircleNetboot](https://github.com/probonopd/CircleNetboot) and start the system.
```

So there are three ways to reference an internal or external document:

- Using a full URL like `@ref url` in code or `[description](url)` in Markdown
- Using a reference tag like `@ref tag` in code or `[description](#tag)` in Markdown
- In Markdown only using a link to another Markdown file link `[description](file-name)`

##### page tag {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_VIEWING_DOXYGEN_OUTPUT_PAGE_TAG}

A page tag has roughly the same function as a header marker in Markdown, with a tag.

```
# Tutorial 11: Doxygen {#TUTORIAL_11_DOXYGEN}

\page TUTORIAL_11_DOXYGEN Tutorial 11: Doxygen
```

Both entries mean essentially the same for `Doxygen`, however the first one is more logical for Markdown.

##### subpage tag {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_VIEWING_DOXYGEN_OUTPUT_SUBPAGE_TAG}

A subpage tag is used to reference another document.
This can be used to create a hierarchy of pages.
It behaves similar as `\ref` in the sense that `\ref <name>` creates a reference to a page labeled `<name>` with the optional link text as specified in the second argument.

```
- @subpage TUTORIALS
```

In Markdown this has no meaning, but in Doxygen, it creates a tree with subpages underneath parent pages.

<img src="images/doxygen-page-tree.png" alt="Doxygen page tree" width="300"/>

##### todo tag {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_VIEWING_DOXYGEN_OUTPUT_TODO_TAG}

The todo tag is used to gather things still to do under one page. This is handy to collect open issues for example.

```
@todo Add register details
```

This will result in a page in the tree named `Todo List`.

<img src="images/doxygen-todo-page.png" alt="Doxygen page tree" width="1000"/>

##### tableofcontents tag {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_DOCUMENTING_CODE_VIEWING_DOXYGEN_OUTPUT_TABLEOFCONTENTS_TAG}

The `@tableofcontents` tag is useful in Markdown files. It will (if enabled) make `Doxygen` generate a table of contents for the file, which is shown at the top right of the page.

<img src="images/doxygen-table-of-contents.png" alt="Doxygen table of contents" width="1000"/>

### Static code checking {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_STATIC_CODE_CHECKING}

In order to improve code, it is advisable to use a static code checker. There are many good tools, but most are expensive to use.
However, cppcheck is a free code checker for C++ code, and does a good job.

#### Installing cppcheck {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_STATIC_CODE_CHECKING_INSTALLING_CPPCHECK}

To install `cppcheck`, go to their [site](http://cppcheck.net/), and follow the instructions.
For Windows, download the installer. On Linux, you can simply install the package.
We'll decide to install the package in both Windows and Linux, as in that case, the location is clear.

##### Windows {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_STATIC_CODE_CHECKING_INSTALLING_CPPCHECK_WINDOWS}

We will install the latest `cppcheck` available at the moment, 2.16.
Download the installer [here](https://github.com/danmar/cppcheck/releases/download/2.16.0/cppcheck-2.16.0-x64-Setup.msi), and run it.

<img src="images/cppcheck-install-1.png" alt="Doxygen table of contents" width="500"/>

Click `Next`

<img src="images/cppcheck-install-2.png" alt="Doxygen table of contents" width="500"/>

Click `Next`

<img src="images/cppcheck-install-3.png" alt="Doxygen table of contents" width="500"/>

Click `Install` and approve installation

<img src="images/cppcheck-install-4.png" alt="Doxygen table of contents" width="500"/>

Click `Finish`

To test, open a command line prompt, and execute:
```bat
cppcheck --version
```

The command should execute without error, and print the version of `cppcheck`:

```text
Cppcheck 2.16.0
```

##### Linux {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_STATIC_CODE_CHECKING_INSTALLING_CPPCHECK_LINUX}

On Linux, simply run apt to install (the latest version depends on your distribution, in my case it was 2.10):

```bash
sudo apt install cppcheck
```

```text
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following packages were automatically installed and are no longer required:
  libdbus-glib-1-2 linux-headers-6.1.0-18-amd64 linux-headers-6.1.0-18-common
  linux-headers-6.1.0-23-amd64 linux-headers-6.1.0-23-common
  linux-image-6.1.0-18-amd64 linux-image-6.1.0-23-amd64
Use 'sudo apt autoremove' to remove them.
The following additional packages will be installed:
  libtinyxml2-9
Suggested packages:
  cppcheck-gui clang-tidy
The following NEW packages will be installed:
  cppcheck libtinyxml2-9
0 upgraded, 2 newly installed, 0 to remove and 0 not upgraded.
Need to get 2,116 kB of archives.
After this operation, 9,993 kB of additional disk space will be used.
Do you want to continue? [Y/n] y
Get:1 http://ftp.nl.debian.org/debian bookworm/main amd64 libtinyxml2-9 amd64 9.                                                                                                                                                                                                                                             0.0+dfsg-3.1 [31.8 kB]
Get:2 http://ftp.nl.debian.org/debian bookworm/main amd64 cppcheck amd64 2.10-2                                                                                                                                                                                                                                              [2,084 kB]
Fetched 2,116 kB in 0s (10.6 MB/s)
Selecting previously unselected package libtinyxml2-9:amd64.
(Reading database ... 261607 files and directories currently installed.)
Preparing to unpack .../libtinyxml2-9_9.0.0+dfsg-3.1_amd64.deb ...
Unpacking libtinyxml2-9:amd64 (9.0.0+dfsg-3.1) ...
Selecting previously unselected package cppcheck.
Preparing to unpack .../cppcheck_2.10-2_amd64.deb ...
Unpacking cppcheck (2.10-2) ...
Setting up libtinyxml2-9:amd64 (9.0.0+dfsg-3.1) ...
Setting up cppcheck (2.10-2) ...
Processing triggers for man-db (2.11.2-2) ...
Processing triggers for libc-bin (2.36-9+deb12u8) ...
```

To test, open a command line prompt, and execute:
```bash
cppcheck --version
```

The command should execute without error, and print the version of `Doxygen`:

```text
Cppcheck 2.10
```

#### Running cppcheck {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_STATIC_CODE_CHECKING_RUNNING_CPPCHECK}

As cppcheck requires quite some parameters, let's create a script for running it.

##### Windows {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_STATIC_CODE_CHECKING_RUNNING_CPPCHECK_WINDOWS}

```bat
@echo off
set this_dir=%~dp0
set root_dir=%this_dir%..
set log_dir=%root_dir%\output\log

set build_dir=%root_dir%\cppcheck-build-dir
mkdir %build_dir%
mkdir %log_dir%
cppcheck --cppcheck-build-dir=%build_dir% --enable=all --check-level=normal --output-file=%log_dir%\cppcheck.log --checkers-report=%log_dir%\cppcheck.chk -DPLATFORM_BAREMETAL -DBAREMETAL_RPI_TARGET=3 --inline-suppr --platform=native --project=%root_dir%\cmake-BareMetal-RPI3-Debug\compile_commands.json --std=c++17
```

##### Linux {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_STATIC_CODE_CHECKING_RUNNING_CPPCHECK_LINUX}

```bash
this_dir=$(dirname "$0")
root_dir=$this_dir/..
log_dir=$root_dir/output/log

build_dir=$root_dir/cppcheck-build-dir
mkdir -p $build_dir
mkdir -p $log_dir
cppcheck --cppcheck-build-dir=$build_dir --enable=all --check-level=normal --output-file=$log_dir/cppcheck.log --checkers-report=$log_dir/cppcheck.chk -DPLATFORM_BAREMETAL -DBAREMETAL_RPI_TARGET=3 --inline-suppr --platform=native --project=$root_dir/cmake-BareMetal-RPI3-Debug/compile_commands.json --std=c++17
```

Explanation of options:
- cppcheck-build-dir: Specifies the build directory cppcheck will use to collect analysis data.
It will reuse the contents of this folder in subsequent runs, to improve performance
- enable: Determines which checks are performed. Using all means all check are performed:
  - warning: Enable warning messages
  - style: Enable all coding style checks.
  All messages with the severities 'style', 'performance' and 'portability' are enabled.
  - performance: Enable performance messages
  - portability: Enable portability messages
  - information: Enable information messages
  - unusedFunction: Check for unused functions. It is recommended to only enable this when the whole program is scanned
  - missingInclude: Warn if there are missing includes.
- check-level: Selectes between `normal` or `exhaustive` analysis
- output-file: Specifies path to log output file
- checkers-report: Specifies path to report of checks that are performed
- DPLATFORM_BAREMETAL: Specify a compiler definition (setting platform to Baremetal)
- DBAREMETAL_RPI_TARGET=3: Specify a compiler definition (setting target to Raspberry Pi 3)
- inline-suppr: Support inline suppression of warnings in code
- platform: Specifies platform to check for. Setting this to native means generic code
- project: Specifies where to find the definition of the project.
In this case we point to the generated compile_commands.json file
- std: Specifies C++ standard to check against. We are using c++17 in this code

Running cppcheck:
```bat
tools\run-cppcheck.bat
```

```text
A subdirectory or file D:\Projects\baremetal\tools\..\cppcheck-build-dir already exists.
A subdirectory or file D:\Projects\baremetal\tools\..\output\log already exists.
Checking D:\Projects\baremetal\code\applications\demo\src\main.cpp ...
1/2 files checked 50% done
Checking D:\Projects\baremetal\tutorial\04-setting-up-common-options\code\applications\demo\src\main.cpp ...
2/2 files checked 100% done
```

When looking at the log file `output\log\cppcheck.log` we see no warnings:

```text
nofile:0:0: information: Active checkers: 167/836 [checkersReport]

```

Now if we add a function to `main.cpp` that is not used:

```cpp
File: D:/Projects/baremetal/code/applications/demo/src/main.cpp
1: /// @file
2: /// Demo application main code
3:
4: static void dummy()
5: {
6:
7: }
8:
9: /// <summary>
10: /// Demo application main code
11: /// </summary>
12: /// <returns>For now always 0</returns>
13: int main()
14: {
15:     return 0;
16: }
17:
```

and then run cppcheck, we see:

```text
D:\Projects\baremetal\code\applications\demo\src\main.cpp:4:0: style: The function 'dummy' is never used. [unusedFunction]
static void dummy()
^
nofile:0:0: information: Active checkers: 167/836 [checkersReport]

```

### Code formatting {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_CODE_FORMATTING}

Visual Studio 2019 (and later) have built-in support for clang-format.
This tool is part of the LLVM suite also hosting the clang compiler.
clang-format supports a wide variety of coding styles, and can be used to keep your source code in a specific style.
Even though Visual Studio has built-in support for clang-format, it supports a slightly older version of the tool.
We're going to use the most recent version as it supports more features.
As of the moment of writing this tutorial, the latest version is 18.1.8.

Information on LLVM can be found on the [LLVM org site](https://llvm.org/).
Downloads and documentation can be found on the [download page](https://releases.llvm.org/download.html).
The latest version of LLVM for Windows can be downloaded directly [here](https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/LLVM-18.1.8-win64.exe).

#### Installing LLVM {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_CODE_FORMATTING_INSTALLING_LLVM}

Run the downloaded installer.

<img src="images/llvm-install-1.png" alt="Install LLVM" width="500"/>

Click `Next`.

<img src="images/llvm-install-2.png" alt="Install LLVM" width="500"/>

Click `I Agree`.

<img src="images/llvm-install-3.png" alt="Install LLVM" width="500"/>

Select `Add LLVM to the system PATH for all users`. Click `Next`.

<img src="images/llvm-install-4.png" alt="Install LLVM" width="500"/>

Click `Next`.

<img src="images/llvm-install-5.png" alt="Install LLVM" width="500"/>

Click `Install`.

<img src="images/llvm-install-6.png" alt="Install LLVM" width="500"/>

Click `Finish`.

To test, open a command line prompt, and execute:
```bat
clang-format --version
```

The command should execute without error, and print the version of `clang-tools`:

```text
clang-format version 18.1.8
```

#### Configure Visual Studio to use custom installation of clang-format {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_CODE_FORMATTING_CONFIGURE_VISUAL_STUDIO_TO_USE_CUSTOM_INSTALLATION_OF_CLANG_FORMAT}

We will need to tell Visual Studio to use the version of clang-format we just installed, and to switch on support for clang-format.

In Visual Studio, select `Tools->Options` from the menu.

<img src="images/visualstudio-options.png" alt="Configure Visual Studio for clang-format" width="500"/>

In the list scroll down and select `Text Editor`

<img src="images/visualstudio-options-texteditor.png" alt="Configure Visual Studio for clang-format" width="500"/>

Open the folder under `Text Editor` and navigate to `C/C++->Code Style->Formatting`

<img src="images/visualstudio-options-texteditor-formatting.png" alt="Configure Visual Studio for clang-format" width="500"/>

Select `Enable ClangFormat support`, then `Use custome clang-format.exe file.`
Click `Browse...`.

<img src="images/visualstudio-options-texteditor-formatting-select-clang-format.png" alt="Configure Visual Studio for clang-format" width="500"/>

Navigate to `C:\Program Files\LLVM\bin` and select `clang-format.exe`. Click `Open`.

<img src="images/visualstudio-options-texteditor-formatting-final.png" alt="Configure Visual Studio for clang-format" width="500"/>

Click `OK`

We can now use clang-format for formatting our source code.
This can be done using the following keyboard shortcuts:

- Ctrl-K, Ctrl-F: Format selection
- Ctrl-K, Ctrl-D: Format file

#### Customizing the code style. {#TUTORIAL_05_SETTING_UP_TOOLS_AN_INTERMEZZO_TOOLS_CODE_FORMATTING_CUSTOMIZING_THE_CODE_STYLE}

Visual studio also supports using the clang-format configuration file.
For CMake projects, this has to be in the project root directory.

There is a nice online to where you can play with different settings, and save the resulting configuration file named [clang-format-configurator](https://clang-format-configurator.site/).

There are quite a few options, so please visit the LLVM site for a description of all supported options.

The version of the file added to this project is my own personal style.

## Documentation {#TUTORIAL_05_SETTING_UP_TOOLS_DOCUMENTATION}

From here onwards, I will be adding documentation to code.
I'll also try to keep the formatting in line.

I will however not be showing the changes related to adding the documentation.

Next: [06-console-uart1](06-console-uart1.md)
