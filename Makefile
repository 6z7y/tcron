CC := cc

PROG_NAME := tcron

SRC_DIR := src
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
INSTALL_PATH := /usr/local/bin

all: $(PROG_NAME)

$(PROG_NAME): $(SRC_FILES)
	$(CC) $^ -o $@

install: $(PROG_NAME)
	install -m 755 $(PROG_NAME) $(INSTALL_PATH)

clean:
	rm -f $(PROG_NAME)

uninstall:
	rm -f $(INSTALL_PATH)/$(PROG_NAME)
