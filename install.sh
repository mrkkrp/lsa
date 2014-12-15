#!/bin/bash
#
# LSA Installation script
#
# Copyright (c) 2014 Mark Karpov
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# !!! Fix me!

### constants

SCRIPT_DIR="$( cd "$( dirname "{BASH_SOURCE[0]}" )" && pwd )"
I_DIRS="/usr/share{,/licenses,/doc}/mida/"
I_ITEMS="/usr/{bin/mida{,rm},share/man/man1/mida{,rm}.1.gz,\
share/{licenses,doc}/mida}"

### functions

bad_exit() # prints error message and exits the program
{
    echo "FAILED." 1>&2
    exit 1
}

### main

echo '-> MIDA installation has been started;'

# 1. check if actual user is root (must be root to install the software)

echo -n '=> actual user must be root....'
test $(id -u) -gt 0 && bad_exit
echo 'OK,'

# 2. check if there is compiled executable

echo -n '=> searching for executable....'
test -f $SCRIPT_DIR/dist/build/mida/mida || bad_exit
echo 'OK,'

# 3. creating directories

echo -n '=> creating directories........'
eval mkdir -p $I_DIRS > /dev/null 2>&1
if test $? -eq 0
then echo 'OK,'
else bad_exit
fi

# 4. copying new files

echo -n '=> copying new files...........'
cp -u $SCRIPT_DIR/dist/build/mida/mida /usr/bin/
cp -u $SCRIPT_DIR/midarm               /usr/bin/
cp -u $SCRIPT_DIR/LICENSE.md           /usr/share/licenses/mida/
cp -u $SCRIPT_DIR/doc/*.{html,css}     /usr/share/doc/mida/
cp -u $SCRIPT_DIR/doc/mida{,rm}.1.gz   /usr/share/man/man1/
echo 'OK,'

# 5. settting permissions

echo -n '=> setting permissions.........'
eval chmod -R 755 $I_ITEMS
if test $? -eq 0
then echo 'OK,'
else bad_exit
fi

# 6. done

echo '=> done.'