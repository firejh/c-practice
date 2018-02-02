
.PHONY: all

all: clean color

color:
	gcc -Wall -DCOLOR_TEST -O2 -o color -I./ color.c
	./color

clean:
	@rm -rf ./color
