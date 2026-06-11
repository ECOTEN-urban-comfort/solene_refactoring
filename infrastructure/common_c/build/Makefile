CC += -fcommon

REPSRC	= ./src
REPOBJ	= ./obj
REPEXE	= ./exe

SRCLIB	= \
	face_op_face.c \
	geomutile.c \
	lib_solene_94.c \
	pers_util.c \
	poly_op_poly.c \
	traite.c \
	solaire.c \
	solutile.c

SRC	= $(filter-out $(SRCLIB), $(notdir $(wildcard $(REPSRC)/*.c)))

OBJ	= $(patsubst %.c, %.o, $(SRC))
OBJ	+= $(patsubst %.c, %.o, $(SRCLIB))
OBJLIB	= $(patsubst %.c, $(REPOBJ)/%.o, $(SRCLIB))
EXE	= $(patsubst %.c, %.exe, $(SRC))

vpath	%.c	$(REPSRC)
vpath	%.o	$(REPOBJ)
vpath	%.exe	$(REPEXE)

.PHONY	= all clean exe obj prerequis ln_exe scanexe scanobj

all: exe ln_exe clean

exe: obj $(EXE)
obj: prerequis $(OBJ)

prerequis:
#	-ln -s $(REPSRC) $(REPOBJ)
#	-ln -s $(REPSRC) $(REPEXE)
	-\mkdir -p $(REPOBJ) $(REPEXE)

%.exe: %.o
	$(CC) -o $(REPEXE)/$@ $< $(OBJLIB) -lm
%.o : %.c
	$(CC) -o $(REPOBJ)/$@ -I$(REPSRC) -I$(REPSRC)/include -c $<


ln_exe:
	cd $(REPEXE) && for f in *.exe; do ln -s $$f `basename $$f .exe`; done

#scanexe:
#	@echo "Il manque les executables suivants :"
#	@echo $(shell for f in $(EXE); do [ -e $(REPEXE)/$$f ] || echo -e "$(REPEXE)/$$f"; done )

#scanobj:
#	@echo "Il manque les objets suivants :"
#	@echo $(shell for f in $(OBJ); do [ -e $(REPOBJ)/$$f ] || echo -e "$(REPOBJ)/$$f"; done )

clean:
	rm -fr $(REPOBJ)
