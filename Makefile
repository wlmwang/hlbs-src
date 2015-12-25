
###############################
# Copyright (C) Anny.
# Copyright (C) Disvr, Inc.
###############################

#
# Makefile.
#

SERVER := routerserver agentserver agentcmd

all:
	for d in $(SERVER); do echo; echo "compile $$d now"; echo; $(MAKE) dir clean all -C $$d; done

release:
	for d in $(SERVER); do sed -i 's/\s\+-D_DEBUG_/ #-D_DEBUG_/' $$d/Makefile; done

debug:
	for d in $(SERVER); do sed -i 's/#-D_DEBUG_/-D_DEBUG_/' $$d/Makefile; done

clean:
	for d in $(SERVER); do echo; echo "clean $$d now"; echo; $(MAKE) dir clean -C $$d; done
