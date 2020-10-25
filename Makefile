# Ninshiki Makefile
CC := make
MODULES := ocr gui

.PHONY: all ocr gui

all: $(MODULES)

$(MODULES):
	cd $@ && $(CC)

clean:
	for module in $(MODULES); do cd $$module && $(CC) $@ && cd .. ; done

mrproper:
	for module in $(MODULES); do cd $$module && $(CC) $@ && cd .. ; done
