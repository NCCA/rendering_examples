Environment Variables:
Make sure the following environment variables are set:
IGLDIR: path to libigl (see below)
EIGENDIR: path to eigen (see below)
NGLDIR: path to your NGL installation (often $HOME/NGL)
NOISEDIR: path to libnoise installation (see below)

To set your environment variables, add a line like the one below to your
$HOME/.bashrc:
export NGLDIR=$HOME/NGL

Installing Dependencies:
libigl
Call the following from root path you want to install it in:
git clone --recursive https://github.com/libigl/libigl.git
Documentation and tutorial here: http://libigl.github.io/libigl/
Note that this is a header only library - no compilation is required.

libnoise
The actual repository for this is available here:
http://libnoise.sourceforge.net/
However I've found the build process to be unreliable, so I've provided
a qmake-able version of this within the packages directory. You still need
to set your NOISEDIR path however, which will be something like 
export NOISEDIR=$(PATH_TO_WORKSHOPS)/common/packages/noise

eigen
This should be installed on the lab build. If not, it can be retrieved
using mercurial:
hg clone https://bitbucket.org/eigen/eigen/
or downloaded from http://eigen.tuxfamily.org/
This is a header only library - no compilation is required.

soil2
Download from https://bitbucket.org/SpartanJ/soil2 using the link. Rename
the resulting directory to "soil2" and move it to the "packages"directory.
This library does need to be compiled before it can be used. Instructions
are available at the above link.





