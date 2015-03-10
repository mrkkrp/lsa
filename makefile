#
# LSA Makefile
#
# Copyright (c) 2014 Mark Karpov
#
# LSA Makefile is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# LSA Makefile is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.

.PHONY : clear

build/lsa : src/main.o src/analyze.o
	gcc -msse -msse2 -laudiofile -lpthread -lm -o build/lsa \
	build/main.o build/analyze.o

src/main.o :
	mkdir -p build
	gcc -c -o build/main.o src/main.c

src/analyze.o :
	mkdir -p build
	gcc -c -o build/analyze.o src/analyze.c

clear :
	rm -vr build
