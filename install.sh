#!/bin/bash
#
# LSA Installation Script
#
# Copyright © 2014, 2015 Mark Karpov
#
# LSA Installation Script is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or (at
# your option) any later version.
#
# LSA Installation Script is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

### constants

I_DIRS="/usr/share/licenses/lsa"

### functions

bad_exit() # prints error message and exits the program
{
    echo "failed" 1>&2
    exit 1
}

### main

echo 'LSA installation has been started;'

# 1. check if actual user is root (must be root to install the software)

echo -n 'actual user must be root...'
test $(id -u) -gt 0 && bad_exit
echo 'ok'

# 2. check if there is compiled executable

echo -n 'searching for executable...'
test -f build/lsa || bad_exit
echo 'ok'

# 3. creating directories

echo 'creating directories...'
eval install -vdm755 $I_DIRS
if test $? -eq 0
then echo 'creating directories: ok'
else bad_exit
fi

# 4. copying new files

echo 'copying new files...'
install -vsDm755 build/lsa   /usr/bin/
install -vDm644 LICENSE.md   /usr/share/licenses/lsa/
install -vDm644 doc/lsa.1.gz /usr/share/man/man1/
echo 'copying new files: ok'

# 5. done

echo 'done.'
