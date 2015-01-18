darwin
======

Track changes as a genome evolves over time (hence the name). More information and project page on the [igem wiki](http://2014.igem.org/Team:Vanderbilt_Software).

#### Build

To build, run 'scons' in the root directory; the executable will be placed there when compilation finishes. Run 'scons --clean' to remove all build files.

Currently only builds on Linux or Mac OS X.

#### Dependencies

* [scons](http://www.scons.org/)
* [GLib](https://developer.gnome.org/glib/stable/)
* [gmp](https://gmplib.org)
* a C compiler
* an sh-compatible shell (bash works just fine)

#### Description

A full-featured version-control system (VCS) for biological parts. Formats standard genetic files (currently just fasta) to be processed extremely efficiently by a standard VCS (currently just git) and throws a CLI/GUI layer on top of it so an unskilled user doesn't need to worry about the underlying abstraction.

#### Usage

##### Setup

Add the repository to your PATH. This allows you to run the scripts 'darwin', 'dwndiff_helper', and 'dwndiff'. If you do not wish to add this to your PATH permanently, you can set it temporarily by running:
```
PATH=$PATH:<path-to-repo>
```
Setting up the repository in the first place is the difficult part. Future work will allow for easier setup. Currently, to begin a darwin repository, run:
```
darwin init git
```
Then, run
```cd .dwn```

From within this folder, one can run all the usual git commands: setting up a remote tracker, creating a server, etc. Further documentation can be found at http://git-scm.com/documentation and https://help.github.com/ .

Once the remote has been set up, though, the process is relatively simple. As with a standard git repository, you can run
```
darwin clone git <remote url>
```
This will clone the remote repository and all its files to your directory. For all fasta files in the repository, darwin will automatically unzip them from the intermediate format in the .dwn folder and place them into the present working directory.


##### Usage

Typically, the workflow goes something like:

1. Make changes to files.
2. Run ```darwin add <file>```.
3. When you've made enough changes, run ```darwin commit <message>```. The message is required for every commit.
4. After running a commit, you can run ```darwin status``` or ```darwin log``` to check out what changes have been made to the repository.
5. When you are sure your efforts are complete, run ```darwin push``` to push back to the remote repository.


##### Pitfalls

The software, and especially the interface, is not yet complete. However, most of its functionality can be felt out with knowledge of git and dwndiff. You can review the documentation for dwndiff by running
```
dwndiff --help
```
