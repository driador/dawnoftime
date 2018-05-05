MAKE	= "make"
CPP     = g++
PROF	= -g -O
OBJDIR  = obj
VPATH   = .:obj
NOCRYPT	=
C_FLAGS = -Wall  $(PROF) $(NOCRYPT)
L_FLAGS = $(PROF)
LIBS	= -lz 

O_FILES = \
	act_comm.o act_ente.o act_info.o act_move.o act_obj.o \
	act_wiz2.o act_wiz.o aedit.o affects.o alias.o areas.o autostat.o \
	banedit.o ban.o base64.o bit.o cedit.o channels.o chardata.o \
	clanedit.o clan.o class.o colour.o comedit.o comm.o const.o \
	connect.o corpse.o cust_col.o d2magsys.o dawnlib.o dawnlog.o dawnstat.o \
	db2.o db.o debug.o deity.o duel.o dynamics.o dyntable.o effects.o \
	entity.o events.o exitlist.o ey_crypt.o fight.o flags.o ftp.o gamble.o \
	gameedit.o gio.o global.o grpedit.o handler.o healer.o hedit.o help.o \
	herbedit.o hreboot.o ictime.o immquest.o interp.o intro_ex.o \
	intro.o ispell.o jail.o langedit.o language.o laston.o letters.o lockers.o \
    lookup.o magic_ce.o magic_da.o magic_ja.o magic_ke.o magic.o magic_ob.o \
	magic_qu.o magic_ra.o magic_re.o magic_sb.o magic_ti.o map.o match.o \
    medit.o mem.o mining.o mixedit.o mix.o mp_cmds.o mud_prog.o mount.o \
    msp.o mud2web.o mxp.o namegen.o nanny.o netio.o network.o noble.o notenet.o \
	note.o obdb.o objdata.o obskill.o oedit.o offmoot.o olc_act.o olc_ex.o \
	olc_mpcd.o olc.o olc_save.o o_lookup.o password.o pushdrag.o pipe.o \
    pload.o qedit.o raceedit.o races.o recycle.o redit.o remort.o resolve.o \
	roles.o rp.o save.o saymote.o scan.o score.o scripts.o sedit.o shop.o \
	skill_ke.o skills.o skill_ti.o skill_tr.o sk_type.o socedit.o socials.o \
	special.o statset.o string.o support.o tables.o textsrch.o \
	tokens.o track.o trap.o update.o websrv.o whofmt.o who.o wizlist.o

all: dawn resolver 

# GNU related make 
#dawn: $(sort $(O_FILES))
#	$(CPP) -c $(C_FLAGS) com_time.cpp -o $(OBJDIR)/com_time.o
#	-rm -f ../dawn
#	$(CPP) $(L_FLAGS) -o ../dawn \
#		$(addprefix $(OBJDIR)/,$(O_FILES)) $(OBJDIR)/com_time.o $(LIBS)

# BSD related make 
dawn: $(O_FILES)
	$(CPP) -c $(C_FLAGS) $(.CURDIR)/com_time.cpp -o com_time.o
	-rm -f $(.CURDIR)/../dawn
	$(CPP) $(L_FLAGS) -o $(.CURDIR)/../dawn \
		$(O_FILES) $(.OBJDIR)/com_time.o $(LIBS)

# STD related make 
#dawn: $(O_FILES)
#	$(CPP) -c $(C_FLAGS) $(.CURDIR)/com_time.cpp -o $(.OBJDIR)/com_time.o
#	-rm -f ../dawn
#	$(CPP) $(L_FLAGS) -o ../dawn \
#		$(addprefix $(OBJDIR)/,$(O_FILES)) $(OBJDIR)/com_time.o $(LIBS)

# GNU resolver make
#../resolver: extras/resolver.cpp
#	$(CPP) $(L_FLAGS) -o ../resolver extras/resolver.cpp $(LIBS)

# BSD resolver make
../resolver: ../../resolver
../../resolver: ../extras/resolver.cpp
	$(CPP) $(L_FLAGS) -o ../../resolver ../extras/resolver.cpp $(LIBS)

# STD resolver make
#../resolver: extras/resolver.cpp
#	$(CPP) $(L_FLAGS) -o ../resolver extras/resolver.cpp $(LIBS)

resolver: ../resolver
	
clean:
#	rm -f $(OBJDIR)/*.o
	rm -f *.o
#	rm -f *.o

cleanall:
	rm -f ../dawn
	rm -f ../resolver

resetconfig:
	rm -f $(OBJDIR)/*.o
	rm -f configure/config.cache
	rm -f configure/config.log
	rm -f configure/config.status
	rm -f configure/configure.lineno
	rm -Rf configure/autom4te.cache 
	rm -f ./config.h
	rm -f ./makefile
	cp -f configure/makefile.original ./makefile
	d2u *.cpp *.h
	d2u configure/*
	chmod +x configure/configure

rebuild: clean dawn

rebuildall: cleanbin all

preparerelease:
	d2u *.cpp *.h
	rm -Rf _release
	mkdir _release
	for file in *.cpp *.h; do \
		unifdef -UMFCODE $$file > _release/$$file; \
	done
	rm -f _release/_*.cpp _release/_*.h
	mkdir _release/configure
	cp configure/aclocal.m4 _release/configure/
	cp configure/config.hin _release/configure/
	cp configure/configure.in _release/configure/
	sed -e 's/[ ]_[a-z]*\.o//g' configure/makefile.in > _release/configure/makefile.in
	cp configure/makefile.original _release/configure/
	cd _release/configure; d2u *; autoconf; rm -Rf autom4te.cache
	cp -f _release/configure/makefile.original _release/makefile
	mkdir _release/obj
	cp obj/compile_folder.txt _release/obj/
	d2u _release/obj/*
	mkdir _release/docs
	cp docs/* _release/docs/
	d2u _release/docs/*
	mkdir _release/extras
	cp extras/* _release/extras/
	d2u _release/extras/*
	mkdir _release/logs
	-cp logs/code.txt _release/logs/
	-d2u _release/logs/*
	@echo =============================
	@echo TODO - code log cleanup, version.h, license review, header renames
	@echo		 directory structure, tgz, test test test, release
	@echo =============================

.cpp.o: dawn.h
#	$(CPP) -c $(C_FLAGS) $< -o $(OBJDIR)/$(basename $<).o
	$(CPP) -c $(C_FLAGS) $< -o $*.o
#	$(CPP) -c $(C_FLAGS) $< -o $*.o

%.o : %.cpp
#	$(CPP) -c $(C_FLAGS) $< -o $(OBJDIR)/$(basename $<).o
	$(CPP) -c $(C_FLAGS) $< -o $*.o
#	$(CPP) -c $(C_FLAGS) $< -o $*.o

beginhelp:
	@echo =============================================================================
	@echo Change into the configure directory and run ./configure to create a makefile
	@echo 
	@echo Notes: The makefile configure generates requires a version of make that
	@echo "       has the sort function amoung other things.  If you find that make"
	@echo "       is only compiling com_time.cpp when you type 'make rebuild' then you"
	@echo "       need to use a newer version of make or manually remove the use"
	@echo "       of sort from the makefile."
	@echo 
	@echo "       If the makefile has problems compiling the resolver, manually compile"
	@echo "       extras/resolver.cpp as resolver in the directory above the source and"
	@echo "       remove the resolver reference from the 'all' line in the makefile."
	@echo
	@echo =============================================================================

