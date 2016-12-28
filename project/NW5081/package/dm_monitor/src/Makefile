MSGCLIENT = dm_monitor
MSGCLIENT_FILES = dm_monitor.c udp_broadcast.c uci_for_cgi.c msg.c hidisk_tcp_monitor.c
TARGET = $(MSGCLIENT) 
all: $(TARGET)
$(MSGCLIENT): $(MSGCLIENT_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS)  $(MSGCLIENT_FILES) -ljson-c -luci -o  $(MSGCLIENT)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
