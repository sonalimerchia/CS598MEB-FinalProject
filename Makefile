#If you use threads, add -pthread here.
COMPILERFLAGS = -g -Wall -Wextra -Wno-sign-compare 

#Any libraries you might need linked in.
LINKLIBS = -lpthread

#The components of each program. When you create a src/foo.c source file, add obj/foo.o here, separated
#by a space (e.g. SOMEOBJECTS = obj/foo.o obj/bar.o obj/baz.o).
OBJECTS = obj/cnt.o obj/utils.o obj/read_csv.o obj/zcnt.o obj/compare.o obj/cn_profile.o

#Every rule listed here as .PHONY is "phony": when you say you want that rule satisfied,
#Make knows not to bother checking whether the file exists, it just runs the recipes regardless.
#(Usually used for rules whose targets are conceptual, rather than real files, such as 'clean'.
#If you DIDNT mark clean phony, then if there is a file named 'clean' in your directory, running
#`make clean` would do nothing!!!)
.PHONY: all clean

#The first rule in the Makefile is the default (the one chosen by plain `make`).
#Since 'all' is first in this file, both `make all` and `make` do the same thing.
#(`make obj server client talker listener` would also have the same effect).
#all : obj server client talker listener
all : obj compare

#$@: name of rule's target: server, client, talker, or listener, for the respective rules.
#$^: the entire dependency string (after expansions); here, $(SERVEROBJECTS)
#CC is a built in variable for the default C compiler; it usually defaults to "gcc". (CXX is g++).
compare: $(OBJECTS)
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)


#RM is a built-in variable that defaults to "rm -f".
clean :
#	$(RM) obj/*.o server client talker listener
	$(RM) obj/*.o compare

#$<: the first dependency in the list; here, src/%.c. (Of course, we could also have used $^).
#The % sign means "match one or more characters". You specify it in the target, and when a file
#dependency is checked, if its name matches this pattern, this rule is used. You can also use the % 
#in your list of dependencies, and it will insert whatever characters were matched for the target name.
obj/%.o: src/%.c
	$(CC) $(COMPILERFLAGS) -c -o $@ $<
obj:
	mkdir -p obj