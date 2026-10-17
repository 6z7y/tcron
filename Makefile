CC := cc

PROG_NAME := tcron

SRC_DIR := src
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
INSTALL_PATH := /usr/local/bin

INIT := $(shell \
				if [ -d /run/systemd/system ]; then echo systemd; \
				elif [ -d /etc/runit ]; then echo runit; \
				else echo none; fi)

all: $(PROG_NAME)

$(PROG_NAME): $(SRC_FILES)
	$(CC) $^ -o $@

install: $(PROG_NAME)
	install -m 755 $(PROG_NAME) $(INSTALL_PATH)
ifeq ($(INIT),systemd)
   install -Dm644 services/systemd/tcron.service $(DESTDIR)/usr/lib/systemd/user/tcron.service
   @echo "systemd detected: systemctl --user enable --now tcron"
else ifeq ($(INIT),runit)
	install -d $(DESTDIR)/etc/sv/tcron
	install -m755 services/runit/tcron/run $(DESTDIR)/etc/sv/tcron/run
	@echo "runit detected: sudo ln -s /etc/sv/tcron /var/service/"
else
	@echo "no supported init system found, service not installed"

endif

clean:
	rm -f $(PROG_NAME)

uninstall:
	rm -f $(INSTALL_PATH)/$(PROG_NAME)
	rm -f $(DESTDIR)/etc/systemd/system/tcron.service
	rm -rf $(DESTDIR)/etc/sv/tcron
