#ifndef ___encrypt_file__
#define ___encrypt_file__

#define AES_BLOCK_SIZE 16
#define AES_KEY_LEN 32
#define AES_BLOCK_SIZE	16
#define MAX_DMA_LEN	16384
#define MAX_FILE_PATH_LEN 1024
#define MAX_FILE_NAME_LEN 512
#define ENCRYPTED_FILE_NAME_LENGTH 20
#define SAFE_BOX_PATH  "/tmp/mnt/USB-disk-1/safebox"
#define ENC_DATA_PATH  "/tmp/mnt/USB-disk-1/safebox/.e"
typedef struct encrypted_file{
	struct encrypted_file *next;
	char name[MAX_FILE_NAME_LEN];
	char src_path[MAX_FILE_PATH_LEN];
	char dest_path[MAX_FILE_PATH_LEN];
	off_t size;
	time_t mod_time;
}EncryptFile;

struct	encrypted_file *e_file_list;
/*
typedef struct VfileInfoTable
{
	char name[MAX_FILE_NAME_LEN];
 	struct dl_list next;
}v_file_info_t;
*/
int encrypt_file_init();
int encrypt_file_exit();
//int get_enc_path_info(struct conn *c, char *path, struct stat *stp);
#endif
