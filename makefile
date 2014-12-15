#
# LSA Makefile
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

.PHONY : clear

built/lsa : src/main.o src/analyze.o
	gcc -o built/lsa -laudiofile -lpthread built/main.o built/analyze.o

src/main.o :
	mkdir -p built
	gcc -c -o built/main.o src/main.c

src/analyze.o :
	mkdir -p built
	gcc -c -o built/analyze.o src/analyze.c

clear :
	rm -vr built