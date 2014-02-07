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
#SLIB=/usr/lib/libsocket.a
#--------------------------------------------------------------------------#
#  link libraries - leave order of LIBS as is !!!
#
LIBS=$(CLIB) $(BLIB) $(SLIB) $(HOME)/lib/libocaps.a
#--------------------------------------------------------------------------#
#  compile parameters
#
INCS=-I/usr/include -iquote$(INC) -I$(SRN)
#INCS=-I/usr/include -I$(INC) -I$(SRN)
FLAGS=-c $(CDEFINES)-g  #-Xlinker -zmuldefs
#FLAGS=-c $(CDEFINES) -w 0
CFLAGS=$(FLAGS) $(INCS)
#--------------------------------------------------------------------------#
#  link parameters
#
#LFLAGS=-g # -Xlinker -zmuldefs
#--------------------------------------------------------------------------#
#  print parameters
#
LPR=lpr
#--------------------------------------------------------------------------#
#  default rules
#
.c.a:
	@echo "Compile [$*.o] From [$<]"
	@cc $(CFLAGS)  $<  2>err
	@echo "Archive [$*.o] To [$@]"
	@ar -rv  $@  $*.o
	@rm -f $*.o
	@echo

.c.o:
	@echo "Compile [$*.o] From [$<]"; echo
	@cc $(CFLAGS)  $<  2>err

.o:
	@echo "2 Link   [$@] From [$@.o]"; echo
	@cc $(LFLAGS)  -o$@  $@.o   $(LIBS) 2>err
	@cp $@ $(BIN)

COMPILE=\
	echo "1-Compile [$@] From [`basename $(@F) .o`.c]";\
	echo "1-cc $(CFLAGS)  `basename $(@F) .o`.c  ";\
	cc $(CFLAGS)  `basename $(@F) .o`.c  2>err;\
   	cat err;\
	echo "1-Move [$(@F)] To [$(OBJ)]";echo;\
	mv $(@F) $(OBJ)

LINK=\
	echo "3 Link    [$@]   From [$(OBJ)/$(@F).o]";echo;\
	cc  $(LFLAGS) -o$@ $(OBJ)/$(@F).o  $(LIBS) -L$(LD_LIBRARY_PATH)/ -lclntsh  -lclient11 $(LD_LIBRARY_PATH)/libsql11.a $(LD_LIBRARY_PATH)/scorept.o $(LD_LIBRARY_PATH)/sscoreed.o $(LD_LIBRARY_PATH)/nautab.o $(LD_LIBRARY_PATH)/naeet.o $(LD_LIBRARY_PATH)/naect.o $(LD_LIBRARY_PATH)/naedhs.o -lnbeq11  -lnhost11 -lnus11   -lldapclnt11 -lnsslb11  -ln11 -ln11 -lnoname11 -lntcp11 -lntcps11 -lnsslb11 -lntcp11 -lntns11 -ln11 -lnl11 -lnro11 -lnbeq11  -lnhost11 -lnus11   -lldapclnt11 -lnsslb11  -ln11 -ln11 -lnoname11 -lntcp11 -lntcps11 -lnsslb11 \
-lntcp11 -lntns11 -lnbeq11  -lnhost11 -lnus11   -lldapclnt11 -lnsslb11  -ln11 -ln11 -lnoname11 -lntcp11 -lntcps11 -lnsslb11 -lntcp11 -lntns11 -ln11 -lnl11 -lclient11 -lvsn11 -lcommon11 -lskgxp11 -lgeneric11 -lpls11 -lnls11  -lcore11 -lnls11 -lcore11  -lnls11 -lnbeq11  -lnhost11 -lnus11   -lldapclnt11 -lnsslb11  -ln11 -ln11 -lnoname11 -lntcp11 -lntcps11 -lnsslb11 -lntcp11 -lntns11 -ln11 -lnl11 -lnro11 -lnbeq11  -lnhost11 -lnus11   -lldapclnt11 -lnsslb11  -ln11 -ln11 -lnoname11 -lntcp11 -lntcps11 -lnsslb11 -lntcp11 -lntns11 -ln11 -lnl11 -lclient11 -lvsn11 -lcommon11 -lskgxp11 -lgeneric11 -lpls11    $(LD_LIBRARY_PATH)/libpls11.a $(LD_LIBRARY_PATH)/libplp11.a $(LD_LIBRARY_PATH)/libpls11.a  -lnls11  -lcore11 -lnls11 -lcore11  -lnls11  -lclient11 -lvsn11 -lcommon11 -lskgxp11 -lgeneric11 -lpls11    -lnls11  -lcore11 -lnls11 -lcore11  -lnls11 -lnsl -ldl -lm -lpthread 2> err;
LLINK=\
	echo "4 Link    [$@]   From [$(OLIB)($(@F).o)]"; echo;\
	ar -x $(OLIB) $(@F).o;\
	cc  $(LFLAGS) -o$@ $(OBJ)/$(@F).o  $(LIBS) -L$(LD_LIBRARY_PATH)/ -lclntsh -lclient11 $(LD_LIBRARY_PATH)/libsql11.a $(LD_LIBRARY_PATH)/scorept.o $(LD_LIBRARY_PATH)/sscoreed.o $(LD_LIBRARY_PATH)/nautab.o $(LD_LIBRARY_PATH)/naeet.o $(LD_LIBRARY_PATH)/naect.o $(LD_LIBRARY_PATH)/naedhs.o -lnbeq11  -lnhost11 -lnus11   -lldapclnt11 -lnsslb11  -ln11 -ln11 -lnoname11 -lntcp11 -lntcps11 -lnsslb11 -lntcp11 -lntns11 -ln11 -lnl11 -lnro11 -lnbeq11  -lnhost11 -lnus11   -lldapclnt11 -lnsslb11  -ln11 -ln11 -lnoname11 -lntcp11 -lntcps11 -lnsslb11 -lntcp11 -lntns11 -lnbeq11  -lnhost11 -lnus11   -lldapclnt11 -lnsslb11  -ln11 -ln11 -lnoname11 -lntcp11 -lntcps11 -lnsslb11 -lntcp11 -lntns11 -ln11 -lnl11 -lclient11 -lvsn11 -lcommon11 -lskgxp11 -lgeneric11 -lpls11 \
-lnls11  -lcore11 -lnls11 -lcore11  -lnls11 -lnbeq11  -lnhost11 -lnus11   -lldapclnt11 -lnsslb11  -ln11 -ln11 -lnoname11 -lntcp11 -lntcps11 -lnsslb11 -lntcp11 -lntns11 -ln11 -lnl11 -lnro11 -lnbeq11 -lnhost11 -lnus11   -lldapclnt11 -lnsslb11  -ln11 -ln11 -lnoname11 -lntcp11 -lntcps11 -lnsslb11 -lntcp11 -lntns11 \
-ln11 -lnl11 -lclient11 -lvsn11 -lcommon11 -lskgxp11 -lgeneric11 -lpls11    $(LD_LIBRARY_PATH)/libpls11.a $(LD_LIBRARY_PATH)/libplp11.a $(LD_LIBRARY_PATH)/libpls11.a -lnls11  -lcore11 -lnls11 -lcore11  -lnls11  -lclient11 -lvsn11 -lcommon11 -lskgxp11 -lgeneric11 -lpls11    -lnls11  -lcore11 -lnls11 -lcore11  -lnls11 -lnsl -ldl -lm  -lpthread 2> err;\
	rm $(@F).o

CCLINK=\
	echo "Link    [$@]   From [$(OBJ)/$(@F).o]";echo;\
	cc  $(LFLAGS) -o$@ $(OBJ)/$(@F).o  $(LIBS) 2>err;

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
	@echo All are up to date; echo
	
cleanall::
	@echo All are cleaned; echo

printall::
	@echo Print is up to date; echo
