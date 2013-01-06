#http://www.gnu.org/prep/standards/html_node/Standard-Targets.html#Standard-Targets

all: server

OS:=$(shell uname -s)

server:
	@npm install

clean:
	@rm -rf ./build

distclean: clean
	@rm -rf ./node_modules/

rebuild: clean server


.PHONY: clean rebuild
