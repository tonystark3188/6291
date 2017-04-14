CRYPTO_BIN = dm_crypto
TARGET_LIB_SHARE = libdm_crypto.so

LIB_SOURCE_FILES = md5.c dm_crypto_utils.c aes_openssl.c aes_hardware.c
CRYPTO__FILES = test_main.c md5.c dm_crypto_utils.c aes_openssl.c aes_hardware.c

TARGET = $(TARGET_LIB_SHARE) $(CRYPTO_BIN)
all: $(TARGET)
$(TARGET_LIB_SHARE): $(TARGET_LIB_SHARE)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -o $(TARGET_LIB_SHARE) $(LIB_SOURCE_FILES)

$(CRYPTO_BIN): $(CRYPTO__FILES)
	$(CC) $(CFLAGS) $(LDFLAGS)  $(CRYPTO__FILES) -o  $(CRYPTO_BIN) #-ldm_crypto 

.PHONY: clean
clean:
	rm -f *.o $(TARGET)