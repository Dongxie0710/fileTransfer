VPATH = ./src:./src/tools

SRCPATH = ./src
BUILDPATH = ./build

CC = gcc
CFLAGS = -lpthread 

all: server client
	@echo "Make Succeed !"

server:
	@$(CC) $(SRCPATH)/server.c $(SRCPATH)/tools/tools.c $(SRCPATH)/tools/sha1.c $(CFLAGS) -o $(BUILDPATH)/$@

client:
	@$(CC) $(SRCPATH)/client.c $(SRCPATH)/tools/tools.c $(SRCPATH)/tools/sha1.c $(CFLAGS) -o $(BUILDPATH)/$@


.PHONY:clean all
clean:
	rm -rf $(BUILDPATH)/client $(BUILDPATH)/server 