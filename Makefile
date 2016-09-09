include setenv.mk

.PHONY: jucpp jucpp-mysql example angular-example mysql-example

all: jucpp example angular-example
all-mysql: jucpp jucpp-mysql example angular-example mysql-example

jucpp:
	$(MAKE) -C jucpp jucpp

jucpp-mysql:
	$(MAKE) -C jucpp jucpp_mysql

example:
	$(MAKE) -C examples/example

example-debug:
	$(MAKE) -C examples/example debug

angular-example:
	$(MAKE) -C examples/angular-example
	
mysql-example:
	$(MAKE) -C examples/mysql-example

clean:
	$(MAKE) -C jucpp clean
	$(MAKE) -C examples/example clean
	$(MAKE) -C examples/angular-example clean
	$(MAKE) -C examples/mysql-example clean

distclean:
	$(MAKE) -C jucpp distclean
	$(MAKE) -C examples/example distclean
	$(MAKE) -C examples/angular-example distclean
	$(MAKE) -C examples/mysql-example distclean

test:
	echo BIN_DIR=$(BUILD_DIR), BIN_DIR=$(BIN_DIR), mode=$(mode), ARCH=$(ARCH) 