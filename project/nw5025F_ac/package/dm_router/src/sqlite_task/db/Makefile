MODULE := database

SRCS := db_mm_pool.c nas_db.c db_base.c file_table.c media_table.c user_table.c hdisk_table.c device_table.c\
		db_backup.c db_check.c file_table_query.c version_table.c backup_file_table.c db_update.c partition_table.c
SRCS_EXT := sqlite3.c
OBJS := $(SRCS:.c=.o)
OBJS_EXT := $(SRCS_EXT:.c=.o)

#CFLAGS += -g -Wall -I../tools
#LDFLAGS += -lpthread -ldl

all: ${MODULE}
	echo "compile ${MODULE} done!"

${MODULE}: ${OBJS} ${OBJS_EXT}
	cp -rf $(OBJS) $(OBJ_DIR)
	cp -rf $(OBJS_EXT) $(OBJ_DIR)

%.o : %.c
	$(CC) $(CFLAGS) -c $^

.PHONY : clean

clean:
	rm -rf $(OBJS) $(MODULE)
