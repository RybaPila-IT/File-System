# Project Overview

File-System program is (as the name suggests) the system of files!<br>
It has been developed in a way, which strongly resembles Linux 
file system. Files and directories present in the File-System are stored in 
regular .txt file. This file has special format, which is readable for
File-System parser (invoked at the loading of the program).

---

# How to run
**NOTE: makefile runs only at unix-like operating systems** <br><br>
In order to run File-System one has to download this repository.
After that, one should open the *src* folder and type in it following commands: <br>
`make` <br>
`make move` <br>
After that, file system program should be compiled and it will be moved into
*build* directory. After that one has to type in *build* directory: <br>
`./main file_with_file_system.txt` <br>
*file_with_file_system* is the name of the file from which the file system
will be loaded and later stored. <br>
In case if file does not exist, one will be asked to specify the file system
size and then empty file system will be created. <br>
**Example** <br>
`./main file_system.txt` Will try to read file system properties from file named
file_system.txt. If file does not exist, empty system will be created in this file.

---

# Commands
This file system uses absolute path naming convention. It means, that
one has to specify the whole path of the file from root in order to manage it. 
There is no *current directory*. <br> <br>
**Note: root folder is named /** <br>
<br>
Below are presented all commands interpreted by File-System-Parser:

### cat file_path/file
This command will show the content of the file or the directory. <br>
*Examples*: <br>
cat / (shows root dir), cat file1 (shows file1 in root directory) <br>
cat a/b/file1 (shows file1 stored under path a/b)

---

### touch file_path/file
Creates file in specified path. <br>
*Examples* <br>
touch file1 (creates file1 at root directory), touch a/file1 (creates file1 in directory a)

---

### mkdir dir_path/dir
Creates directory at specified path.

---

### quit
Shuts the file system down. File system will save itself inside the file specified 
as the program argument.

---

### erase path/file:directory
Deletes specified file or directory. <br>
*Examples* <br>
erase file1 (deletes file1 at root directory), erase a/b (deletes file or directory b from directory a).

---

### info file : directory : memory : inodes
Gives statistical information about specified directory or file. <br>
If one uses *info memory* or *info inodes*, then statistics about memory blocks and
inodes will be presented. <br>
*Examples* <br>
info memory, info inodes, info file1 (information about file1 at root), info a/b/c 

---

### link file_path/file link_path/link
Creates link to the specified file. <br>
*Examples* <br>
link file1 a/b/link1 (file1 will be linked to link1 created in a/b), link a/b/file2 c/link1

---

### echo file_path/file text
Appends specified text to file. <br>
*Examples* <br>
echo a/file1 hehehehe (appends text hehehehe to file1 stored in directory a)

### copy file_path/file source
Copies file named source into file named file, which is part of File-System. <br>
*Examples* <br>
copy a/file1 my_text.txt (my_text.txt content will be copied and saved inside file1 stored in a directory).

---

### get file_path/file destination
Gets file content and stores it in destination file. <br>
*Examples* <br>
get a/b/file2 what_is_it.txt (content of a/b/file2 will be saved into what_is_it.txt file).

---

### cut file_path/file int
Cuts file content by number of bytes specified by int. <br>
*Examples* <br>
cut file1 10 (If ex. file1 content was "Witam cieplutko", then after performing this command content will be "Witam").
