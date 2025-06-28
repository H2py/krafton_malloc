#####################################################################
# CS:APP Malloc Lab
# Handout files for students
#
# Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
# May not be used, modified, or copied without permission.
#
######################################################################

***********
Main Files:
***********

mm.{c,h}	
	Your solution malloc package. mm.c is the file that you
	will be handing in, and is the only file you should modify.

mdriver.c	
	The malloc driver that tests your mm.c file

short{1,2}-bal.rep
	Two tiny tracefiles to help you get started. 

Makefile	
	Builds the driver

**********************************
Other support files for the driver
**********************************

config.h	Configures the malloc lab driver
fsecs.{c,h}	Wrapper function for the different timer packages
clock.{c,h}	Routines for accessing the Pentium and Alpha cycle counters
fcyc.{c,h}	Timer functions based on cycle counters
ftimer.{c,h}	Timer functions based on interval timers and gettimeofday()
memlib.{c,h}	Models the heap and sbrk function

*******************************
Building and running the driver
*******************************
To build the driver, type "make" to the shell.

To run the driver on a tiny test trace:

	unix> mdriver -V -f short1-bal.rep

The -V option prints out helpful tracing and summary information.

To get a list of the driver flags:

	unix> mdriver -h


1. Makefile 수정하기
컴파일 옵션에 -pg 추가:

make

CFLAGS = -g -Wall -pg
2. 컴파일 후 실행
sh


make clean
make
./mdriver
실행이 끝나면 **gmon.out**라는 파일이 생성됨

3. gprof로 결과 확인
sh


gprof ./mdriver > analysis.txt
4. 결과 예시
analysis.txt를 열면 다음과 같은 분석 정보가 나옴:

sql

Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls   s/call   s/call  name
 30.0      0.30     0.30     12000     0.000    0.000  find_fit
 25.0      0.55     0.25     12000     0.000    0.000  place
 15.0      0.70     0.15     10000     0.000    0.000  coalesce