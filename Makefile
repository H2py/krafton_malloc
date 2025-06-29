# TEAM = bovik
# VERSION = 1
# HANDINDIR = /afs/cs.cmu.edu/academic/class/15213-f01/malloclab/handin

# CC = gcc
# # CFLAGS = -Wall -O2 -m32
# CFLAGS = -Wall -O2

# OBJS = mdriver.o mm.o memlib.o fsecs.o fcyc.o clock.o ftimer.o

# mdriver: $(OBJS)
# 	$(CC) $(CFLAGS) -o mdriver $(OBJS)

# mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
# memlib.o: memlib.c memlib.h
# mm.o: mm.c mm.h memlib.h
# fsecs.o: fsecs.c fsecs.h config.h
# fcyc.o: fcyc.c fcyc.h
# ftimer.o: ftimer.c ftimer.h config.h
# clock.o: clock.c clock.h

# handin:
# 	cp mm.c $(HANDINDIR)/$(TEAM)-$(VERSION)-mm.c

# clean:
# 	rm -f *~ *.o mdriver



#
# Students' Makefile for the Malloc Lab
#
TEAM = bovik
VERSION = 1
HANDINDIR = /afs/cs.cmu.edu/academic/class/15213-f01/malloclab/handin

CC = gcc

# -ggdb3 : GDB에서 매크로 함수 호출 및. 확인 가능
# DDRIVER : #define DRIVER와 같은 효과, #ifdef DRIVER조건 만족시킴
# -std=gnu99 : c99표준 + GNU 확장 기능을 사용하겠다는 의미 : inline 함수 사용 가능

CFLAGS = -Wall -Wextra -ggdb3 -DDRIVER -pg -g -m64 -std=gnu99
OBJDIR = .out

OBJS = $(OBJDIR)/mdriver.o $(OBJDIR)/mm.o $(OBJDIR)/memlib.o \
        $(OBJDIR)/fsecs.o $(OBJDIR)/fcyc.o $(OBJDIR)/clock.o $(OBJDIR)/ftimer.o

# default rule
mdriver: $(OBJDIR) $(OBJS)
	$(CC) $(CFLAGS) -o mdriver $(OBJS)

# compile .c to .o into .out/
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# make .out folder if not exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

# dependencies
$(OBJDIR)/mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
$(OBJDIR)/memlib.o: memlib.c memlib.h
$(OBJDIR)/mm.o: mm.c mm.h memlib.h
$(OBJDIR)/fsecs.o: fsecs.c fsecs.h config.h
$(OBJDIR)/fcyc.o: fcyc.c fcyc.h
$(OBJDIR)/ftimer.o: ftimer.c ftimer.h config.h
$(OBJDIR)/clock.o: clock.c clock.h

handin:
	cp mm.c $(HANDINDIR)/$(TEAM)-$(VERSION)-mm.c

clean:
	rm -f mdriver
	rm -rf $(OBJDIR)
