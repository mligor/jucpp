all: jucpplib

jucpplib:
	$(MAKE) -C jucpp
         
clean:
	$(MAKE) -C jucpp clean

distclean:
	$(MAKE) -C jucpp distclean