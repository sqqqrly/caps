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
BNC=$(HOME)/bard/src
SRN=$(BASE)/screen
#--------------------------------------------------------------------------#
#  library names
#
#
LLIB=/u/mfc/lib/libocaps.a
#--------------------------------------------------------------------------#

#  link libraries - leave order of LIBS as is !!!
#
LIBS=
#--------------------------------------------------------------------------#
#  compile parameters
#
#  #
INCS=-I/usr/include -iquote -I$(INC) -I$(SRN) -I$(BNC)
#INCS=-I/usr/include -iquote-I$(INC) -I$(SRN) -I$(BNC)
FLAGS= -c -O $(CDEFINES)
CFLAGS=$(FLAGS) $(INCS)
#--------------------------------------------------------------------------#
#  link parameters
#
LFLAGS= -g
#--------------------------------------------------------------------------#
#  print parameters
#
LPR=lpr
#--------------------------------------------------------------------------#
#  default rules
#
.c.a:
	echo "Compile [$*.o] From [$<]"
	cc $(CFLAGS)  $<  2>>err
	echo "Archive [$*.o] To [$@]"
	ar -ruv  $@  $*.o
	rm -f $*.o
	echo

.c.o:
	echo "Compile [$*.o] From [$<]"; echo
	cc $(CFLAGS)  $<  2>err

.o:
	echo "Link   [$@] From [$@.o]"; echo
	cc $(LFLAGS)  -o$@  $@.o  $(LIBS) 2>err
	cp $@ $(BIN)

COMPILE=\
	echo "R Compile [$@] From [`basename $(@F) .o`.c]";\
	cc $(CFLAGS)  `basename $(@F) .o`.c  2>err;\
	echo "Move [$(@F)] To [$(OBJ)]";echo;\
	mv $(@F) $(OBJ)

LINK=\
	echo "Link    [$@]   From [$(OBJ)/$(@F).o]"; echo;\
	cc $(LFLAGS)  -o$@  $(OBJ)/$(@F).o  $(LIBS) 2>err;

LLINK=\
	echo "Link    [$@]   From [$(OLIB)($(@F).o)]"; echo;\
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
	mv $(DBPATH)/$(@F) $@;\
	mv $(DBPATH)/file.h .

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
