#--------------------------------------------------------------------------#
#  This makefile is self generating and will make across directories
#  
#  Developed By Ravichandran Jagannathan
#--------------------------------------------------------------------------#
#  related directories
#
BASE=$(HOME)/src
INC=$(BASE)/h
BNC=$(HOME)/bard/src
SRN=$(BASE)/screen
#--------------------------------------------------------------------------#
#  library names
#
LLIB=/u/mfc/lib/libocaps.a
#--------------------------------------------------------------------------#
#  link libraries - leave order of LIBS as is !!!
#
#--------------------------------------------------------------------------#
#  compile parameters
#
INCS=-I$(INC) -I$(SRN) -I$(BNC)
FLAGS= -c -O $(CDEFINES)
CFLAGS=$(FLAGS) $(INCS)
#--------------------------------------------------------------------------#
#  link parameters
#
LFLAGS=-g
#--------------------------------------------------------------------------#
#  print parameters
#
LPR=lpr
#  default rules
.c.a:
	@echo "Compile [$*.c] From [$*.pc]"
	$(ORACLE_HOME)/bin/proc iname=$*.pc SQLCHECK=SEMANTICS userid=/ 
	@echo "Compile [$*.o] From [$<]"
	cc -I${ORACLE_HOME}/precomp/public -I/usr/include  -c $*.c -lc >>err
	echo "Archive [$*.o] To [$@]"
	ar -ruv  $@  $*.o
	rm -f $*.o  $*.lis 
	echo
.c  : 
	@echo "Compile [$*.c] From [$*.pc]"
	@proc iname=$*.pc
#--------------------------------------------------------------------------#
#  dependencies  
#
all:: begin
new:: begin
#--------------------------------------------------------------------------#
#  self constructing makefile
#
#all:: Makeinc
#
#Makeinc: Makedeps
#	@Bmake <Makedeps >Makeinc
#	@echo "Makefile is up to date"
#	@$(MAKE) all
#	
#--------------------------------------------------------------------------#
#  user dependencies
#
include Makeinc
#---------------------------------------------------------------------------#
# universal functions - leave at end of Makefile
#
begin:
	@echo "--------------------------"
	@date '+ %m/%d/%y         %H:%M:%S'
	@echo " Making `pwd`"
	@echo "--------------------------"
	@date >err

new:: cleanall all
	@echo All Have Been Remade; echo

all::
	@echo All are up to date; echo
	
cleanall::
	@echo All are cleaned; echo

printall::
	@echo Print is up to date; echo
