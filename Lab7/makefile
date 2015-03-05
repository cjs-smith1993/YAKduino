#####################################################################
# ECEn 425 Lab 5 Makefile

LAB_APP_NAME := lab7app
BIN := bin
SRC := src
APP := $(SRC)/app
CLIB := $(SRC)/clib
INT := $(SRC)/int
KERNEL := $(SRC)/kernel

#Binary file
$(BIN)/out.bin:	$(BIN)/final.s
		@nasm $(BIN)/final.s -o $(BIN)/out.bin -l $(BIN)/out.lst

#Combined assembly file
$(BIN)/final.s:	$(CLIB)/clib.s $(BIN)/int.s $(BIN)/kernel.s $(BIN)/$(LAB_APP_NAME).s
		@cat $(CLIB)/clib.s $(BIN)/int.s $(BIN)/kernel.s $(BIN)/$(LAB_APP_NAME).s > bin/final.s

#clib file unnecessary

#Interrupt Handler/ISR file
$(BIN)/int.s:	$(BIN)/myinth.s $(INT)/myisr.s
		@cat $(BIN)/myinth.s $(INT)/myisr.s > $(BIN)/int.s

$(BIN)/myinth.s:	$(INT)/myinth.c
		@cpp $(INT)/myinth.c $(BIN)/myinth.i
		@c86 -g $(BIN)/myinth.i $(BIN)/myinth.s

#Kernel file
$(BIN)/kernel.s: $(BIN)/yakc.s $(KERNEL)/yaks.s
		@cat $(BIN)/yakc.s $(KERNEL)/yaks.s > $(BIN)/kernel.s

$(BIN)/yakc.s:	$(KERNEL)/yakc.c
		@cpp $(KERNEL)/yakc.c $(BIN)/yakc.i
		@c86 -g $(BIN)/yakc.i $(BIN)/yakc.s

#Application file
$(BIN)/$(LAB_APP_NAME).s:	$(APP)/$(LAB_APP_NAME).c
		@cpp $(APP)/$(LAB_APP_NAME).c $(BIN)/$(LAB_APP_NAME).i
		@c86 -g $(BIN)/$(LAB_APP_NAME).i $(BIN)/$(LAB_APP_NAME).s

.PHONY: clean
clean:
		@rm -f $(BIN)/*
