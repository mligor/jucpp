include prepare.mk

.PHONY: jucpp jucpp-mysql example angular-example mysql-example

all: jucpp example angular-example
all-mysql: jucpp jucpp-mysql example angular-example mysql-example

jucpp:
	$(MAKE) -C jucpp jucpp

jucpp-mysql:
	$(MAKE) -C jucpp jucpp_mysql

example:
	$(MAKE) -C example

example-debug:
	$(MAKE) -C example debug

angular-example:
	$(MAKE) -C angular-example
	
mysql-example:
	$(MAKE) -C mysql-example

clean:
	$(MAKE) -C jucpp clean
	$(MAKE) -C example clean
	$(MAKE) -C angular-example clean
	$(MAKE) -C mysql-example clean

distclean:
	$(MAKE) -C jucpp distclean
	$(MAKE) -C example distclean
	$(MAKE) -C angular-example distclean
	$(MAKE) -C mysql-example distclean

test:
	echo BIN_DIR=$(BUILD_DIR), BIN_DIR=$(BIN_DIR), mode=$(mode), ARCH=$(ARCH) 