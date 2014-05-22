#include "compressor.h"

struct dict {
	unsigned char chr;
	int addr;
	int len;
	unsigned int hash;
	int hash_next;
	int is_hash_first;
};

class compressor {
private:
			int compression_level;
			int dic_start, dic_end;
			int dic_decompress_size;
			int	dic_decompress_end, dic_decompress_start;
			int fst_pos[256];
			int last_pos[256];
			unsigned char decompress_dictionary[window_size];	
			unsigned int h[window_size];
			int hc;
			unsigned compression_buffer[compress_buffer_size];	
			int compress_buffer_curs;
			unsigned char rollback[min_len];	
			unsigned char to_add[min_len];			
			int rollback_addr[min_len];		
			int rb_start;
			int rb_end;
			unsigned char flag;
			int flag_bit_position;
			int dflag_bit_position;
			unsigned char chr;
			int gogo;
			int i;
			int addr;
			int len;
			int last_addr;
			int last_len;
			int outcurs;
			int dic_next;
			int fi;
			int action;
			int brk;
			int * l_hash;
			int * f_hash;
			unsigned int hash;
			unsigned int hashp;
			int hash_index;
			int ttry;
			int maxlen_size;
public:
		compressor();
		int compress_buffer(unsigned char *, int, unsigned char *, int);
		int add_to_dic(unsigned char);
		int add_to_decompress_dic(unsigned char);
		int decompress_buffer(unsigned char *, int, unsigned char *, int);
		int GetBitFlag( unsigned char , int  );
		void SetCompressionLevel( int );
		unsigned char SetBitFlag(unsigned char, int, unsigned char);
};