#!/bin/bash
#
# LSA Uninstallation script
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

# !!! Please fix me!

### constants

I_ITEMS="/usr/{bin/mida{,rm},share/man/man1/mida{,rm}.1.gz,\
share/{licenses,doc}/mida}"

### functions

bad_exit() # prints error message and exits the program
{
    echo "FAILED." 1>&2
    exit 1
}

### main

echo '-> MIDA uninstallation has been started;'

# 1. check if actual user is root (must be root to uninstall the software)

echo -n '=> actual user must be root......'
test $(id -u) -gt 0 && bad_exit
echo 'OK,'

# 2. removing files

echo -n '=> removing MIDA.................'
eval rm -rv $I_ITEMS > /dev/null 2>&1
if test $? -eq 0
then echo 'OK,'
else bad_exit
fi

# 3. done

echo '=> done.'