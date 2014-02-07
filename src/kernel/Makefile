#--------------------------------------------------------------------------#
#  This makefile is self generating and will make across directories
#  
#	Requires program Bmake to construct details of Makefile.
#  see also models Makelib, Makeolib, Makebinolib, Makeheaders, Makebard ..
#--------------------------------------------------------------------------#
#  related directories
#
BASE=$(HOME)/src
INC=$(BASE)/h
SRN=$(BASE)/screen
OBJ=$(BASE)/obj
BIN=$(HOME)/bin
#--------------------------------------------------------------------------#
#  library names
#
LLIB=$(HOME)/src/kernel/kernlib.a
#LLIB=$(HOME)/src/kernel/kernlib.a  $(HOME)/lib/libocaps.a
#--------------------------------------------------------------------------#
#  link libraries - leave order of LIBS as is !!!
#
LIBS=$(LLIB) $(CLIB) $(MLIB) $(HOME)/lib/libocaps.a
#--------------------------------------------------------------------------#
#  compile parameters
#
INCS=-I/usr/include -iquote$(INC)
FLAGS= -c -O $(CDEFINES)
CFLAGS=$(FLAGS) $(INCS)
#--------------------------------------------------------------------------#
#  link parameters
#
LFLAGS= -g -Xlinker -zmuldefs
#--------------------------------------------------------------------------#
#  print parameters
#
LPR=lpr
#--------------------------------------------------------------------------#
#  default rules
#
.c.a:
	@echo "--Compile [$*.o] From [$<]"
	echo $(CFLAGS)
	cc $(CFLAGS)  $<  2>err
	echo "Archive [$*.o] To [$@]"
	ar -rv  $@  $*.o
	rm -f $*.o
	echo

.c.o:
	echo "-----Compile [$*.o] From [$<]"; echo
	cc $(CFLAGS)  $<  2>err

.o:
	echo "Link   [$@] From [$@.o]"; echo
	cc $(LFLAGS)  -o$@  $@.o  $(LIBS) 2>err
	cp $@ $(BIN)

COMPILE=\
	echo "1- Compile [$@] From [`basename $(@F) .o`.c]";\
	echo "1- cc $(CFLAGS)  `basename $(@F) .o`.c ";\
	cc $(CFLAGS)  `basename $(@F) .o`.c  2>err;\
	echo "1- mv $(@F) $(OBJ)";\
	mv $(@F) $(OBJ)

LINK=\
	echo "1- Link    [$@]   From [$(OBJ)/$(@F).o]"; echo;\
	echo "1- cc $(LFLAGS)  -o$@  $(OBJ)/$(@F).o  $(LIBS) ";\
	cc $(LFLAGS)  -o$@  $(OBJ)/$(@F).o  $(LIBS) 2>err;

LLINK=\
	echo "2- Link    [$@]   From [$(OLIB)($(@F).o)]"; echo;\
	ar -x $(OLIB) $(@F).o;\
	cc $(LFLAGS)  -o$@  $(@F).o  $(LIBS) 2>err;\
	rm	$(@F).o

SCREEN=\
	echo "Screen [$@] From [$?]"; echo;\
	scrproc $@ $?

BUILDB=\
	echo "Buildb  [$@] From [$?]";\
	Buildb `basename $(@F) .h` -d;\
	Buildb $? -awvy;\
	mv $(DBPATH)/$(@F) $@

TOUCH=\
	echo "Touch Header [$@] From [$?]";\
	touch $@

PRINT=\
	hprint -n -3 $? | $(LPR); touch lastprint

#--------------------------------------------------------------------------#
#  dependencies  
#
all:: begin
new:: begin
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

new:: cleanall all
	@echo All Have Been Remade; echo

all::
	@echo All Are Up To Date; echo
	
cleanall::
	@echo All are cleaned; echo

printall::
	@echo Print is up to date; echo
