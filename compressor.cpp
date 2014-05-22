#include <cstdlib>
#include <iostream>
#include "compressor_c.cpp"
#include "compressor.h"
#include <fstream>

using namespace std;
struct dict * dictionary;

compressor::compressor()
{
	compression_level = 300;
	dflag_bit_position = -1;
	addr = 0;
	len = 0;
	maxlen_size = 0;
	last_addr = 0;
	last_len = 0;
	outcurs = 0;
	dic_next = 0;
	fi = 0;
	flag_bit_position = 7;
	flag = 0;
	compress_buffer_curs = 0;
	hash = 0;

	dictionary = (dict*)malloc( sizeof(dict)*window_size );
	l_hash = (int*)malloc( sizeof(int)*hash_table_size );
	f_hash = (int*)malloc( sizeof(int)*hash_table_size );

	hash_index = -3;
	dic_start = 0;
	dic_end = 0;
	dic_decompress_end = 0;
	dic_decompress_start = 0;

	for ( i = 0; i < 256; i++ ){
		fst_pos[i] = -1;
		last_pos[i] = -1;
		}

		//reset dictionary
		for (  i = 0; i < window_size; i++ ){
			dictionary[i].hash = -1;
			dictionary[i].hash_next = -1;
			dictionary[i].is_hash_first = -1;
		}

		//reset hash table
		for ( i = 0; i < hash_table_size; i++ ){
		l_hash[i] = -1;
		f_hash[i] = -1;
		}

}
void compressor::SetCompressionLevel(int level)
{
compression_level = level;
}

int compressor::add_to_decompress_dic(unsigned char chr)
{
decompress_dictionary[dic_decompress_end] = chr;
dic_decompress_end++;
if ( dic_decompress_end == window_size )
dic_decompress_end = 0;
if ( dic_decompress_end == dic_decompress_start )
dic_decompress_start++;
if ( dic_decompress_start == window_size )
dic_decompress_start = 0;
return 0;
}


int compressor::add_to_dic(unsigned char chr)
{
hash = hash << 8;
hash = hash + chr;
hash = ((hash<<8) >> 8);
hash_index++;

if ( hash_index == window_size )
	hash_index = 0;

//hash handling
if ( hash_index >= 0 )
{
// We are removing a first hash from the table
if ( dictionary[dic_end].is_hash_first != -1 )
{
	if (dictionary[dic_end].hash_next == -1){
		l_hash[dictionary[dic_end].hash] = dictionary[dic_end].hash_next;
	}

	f_hash[dictionary[dic_end].hash] = dictionary[dic_end].hash_next;

	if ( dictionary[dic_end].hash_next != -1 )
		dictionary[dictionary[dic_end].hash_next].is_hash_first = dictionary[dic_end].is_hash_first;

	dictionary[dic_end].is_hash_first = -1;
}

//1st hash occurrence
if ( f_hash[hash] == -1 )
{
	f_hash[hash] = hash_index;
	dictionary[hash_index].is_hash_first = 1;
}
else
	dictionary[hash_index].is_hash_first = -1;

	//Update linked list
	if ( l_hash[hash] != -1 )
		dictionary[l_hash[hash]].hash_next = hash_index;

	dictionary[hash_index].hash = hash;
	l_hash[hash] = hash_index;
	dictionary[hash_index].hash_next = -1;
}


dictionary[dic_end].chr = chr;
last_pos[chr] = dic_end;
dic_end++;


if ( dic_end == window_size )
dic_end = 0;

if ( dic_end == dic_start )
dic_start++;

if ( dic_start == window_size )
dic_start = 0;
}

long totalout = 0;
int totalen = 0;
long wasted = 0;

int compressor::decompress_buffer(unsigned char *in, int leni, unsigned char *out, int leno)
{
int out_curs = 0;
if (gogo == 2 ){
	gogo = 0;
	i = 0;
	goto dgt2;
	}

	if (gogo == 3 ){
	gogo = 0;
	i = 0;
	goto dgt3;
	}


	if (gogo == 5 ){
	gogo = 0;
	i = 0;
	goto dgt5;
	}


	gogo = 0;
	len = 0;
	addr = 0;
	action = 0;

	for ( i = 0; i < leni;  )
	{

		if ( dflag_bit_position == -1 ){
		flag = in[i];
		i++;
		dflag_bit_position = 7;
	
		if ( i >= leni ){
			gogo = 5;
			return out_curs;
			}		

		}

		dgt5:

    	action = GetBitFlag(flag, dflag_bit_position);
		dflag_bit_position--;

		if ( action == 1 )
		{
		len = in[i];
		i++;
		len += min_len;

		if ( i >= leni ){
			gogo =2;
			return out_curs;
		}

		dgt2:
		

		addr = in[i]*256;
		i++;

		if ( i >= leni ){
		gogo = 3;
		return out_curs;
		}

		dgt3:

		addr += in[i];
		i++;

		
		addr = dic_decompress_start + addr;
		if ( addr >= window_size )
			addr -= window_size;
		
		for ( int j = 0; j < len; j++ )
		{
			out[out_curs] = decompress_dictionary[addr];
			out_curs++;
			add_to_decompress_dic( decompress_dictionary[addr] );
			addr++;
			if ( addr >= window_size )
			addr = 0;
			}

		}
		else
		{
		out[out_curs] = in[i];
		out_curs++;
		add_to_decompress_dic( in[i] );
		i++;
		}

	}

	return out_curs;
}

//Get bit 0-7 flag return 1 or 0
int compressor::GetBitFlag( unsigned char MyByte, int BitN )
{
MyByte = MyByte<<(7-BitN);
MyByte = MyByte>>(7-BitN+BitN);
return MyByte;
}


//Set bit 0-7 flag, return a setted Byte..
unsigned char compressor::SetBitFlag(unsigned char MyByte, int BitN, unsigned char value)
{
	if (GetBitFlag(MyByte, BitN) == value)
		return MyByte; // It?s Already setted
	else //Need to set it!
	{
		unsigned char xors[] = {0x01, 0x02,0x04,0x08,0x10,0x20,0x40,0x80};
		MyByte = ( MyByte ^ xors[BitN]); // Need to set the bit
		return MyByte;
	}
}


int compressor::compress_buffer(unsigned char *in, int leni, unsigned char *out, int leno)
{



	addr = 0;
	len = 0;
	last_addr = 0;

	outcurs = 0;
	dic_next = 0;
	fi = 0;

	if ( leni == 0 )
	{
		if ( compress_buffer_curs > 0 ){
			out[outcurs++] = flag;
			for ( int z = 0; z <compress_buffer_curs; z++)
				out[outcurs++] = compression_buffer[z];
		}
		return outcurs;
	}

	


for ( i = 0; i < leni;  )
{
		if ( (i+3) <= leni )
		{
		hashp = 0;
	
   		//Next hash code...
			for ( int h = i; h < (i+3); h++ )
			{
			hashp = hashp << 8;
			hashp = hashp + in[h];
			}
		}
		else //End reached
		{
		add_to_dic(in[i]);
		add_to_decompress_dic(in[i]);
		flag_bit_position--;
		compression_buffer[compress_buffer_curs++] = in[i];

		i++;
	if ( flag_bit_position == -1 ){
			flag_bit_position = 7;
			out[outcurs++] = flag;
			flag = 0;
			for ( int z = 0; z <compress_buffer_curs; z++)
				out[outcurs++] = compression_buffer[z];

			compress_buffer_curs = 0;
			}
		continue;
		}

		if ( f_hash[hashp] == -1 )
		{
			addr = 0;
			len = 0;
			add_to_dic(in[i]);
			add_to_decompress_dic(in[i]);
			flag_bit_position--;
			compression_buffer[compress_buffer_curs++] = in[i];
			i++;
		}
		else
		{
			maxlen_size = maxlen;
			dic_next = f_hash[hashp];
			last_len = 0;
			fi = i;
			brk = 0;
			rb_start = dic_decompress_start;
			rb_end = dic_decompress_end;
			int old_dic_start = dic_start;
			ttry = 0;
			do
			{
				if ( ttry ==  compression_level)
				break;
				ttry++;
				i = fi;
				last_addr = dic_next;
				len = 1;
				
				if ( last_len < len ){
				if ( last_len < min_len )
				{
					to_add[last_len] = in[i];
					rollback[last_len] = decompress_dictionary[dic_decompress_end];			
					rollback_addr[last_len] = dic_decompress_end;	
				}
				else
				{
					add_to_dic(in[i]);
				}

				add_to_decompress_dic(in[i]);
				last_len++;
				addr = last_addr;
				
				if ( last_len == min_len )
				{
					for (int x = 0; x < min_len; x++ )
						add_to_dic(to_add[x]);
				}
				}

				i++;
				dic_next++;

				if ( dic_next == window_size )
					dic_next = 0;

				if ( last_len >= min_len )
				{
				i++;
				dic_next++;
				if ( dic_next == window_size )
					dic_next = 0;
				i++;
				dic_next++;
				if ( dic_next == window_size )
					dic_next = 0;
					len++;
					len++;
				}

				while ( (in[i] == decompress_dictionary[dic_next]) && (i<leni) && (last_len < maxlen_size) &&  (dic_next != dic_end) )
				{
				len++;
				if ( (last_len < len) && (last_len < window_size)  ){
				if ( last_len < min_len )
				{
					to_add[last_len] = in[i];
					rollback[last_len] = decompress_dictionary[dic_decompress_end];			
					rollback_addr[last_len] = dic_decompress_end;	
				}
				else
				{
					add_to_dic(in[i]);
				}

				add_to_decompress_dic(in[i]);
				last_len++;
				totalout++;
				addr = last_addr;

				if ( last_len == min_len )
				{
					for (int x = 0; x < min_len; x++ )
						add_to_dic(to_add[x]);
				}
				}
				i++;
				dic_next++;
					if ( dic_next == window_size )
					dic_next = 0;
				}

			}
			while( ((dic_next = dictionary[last_addr].hash_next) != -1 ) && (last_len < maxlen_size) && (i < leni)  );
			i = fi + last_len;
			
		if ( last_len >= min_len ){
			last_len-= min_len;
			flag = SetBitFlag(flag,flag_bit_position,1);
			flag_bit_position--;
			compression_buffer[compress_buffer_curs++] = (last_len);
			addr = addr - old_dic_start;
			if ( addr < 0 )
				addr+= window_size;
				
			compression_buffer[compress_buffer_curs++] = (addr/256);
			compression_buffer[compress_buffer_curs++] = (addr%256);
			}
			else
			{
			
			flag_bit_position--;
			i = fi;
			for ( int x = 0; x < last_len; x++ )
			{
				decompress_dictionary[rollback_addr[x]]=rollback[x];
			}
			dic_decompress_start =  rb_start;
			dic_decompress_end	= rb_end;		
			
			compression_buffer[compress_buffer_curs++] = in[i];
			add_to_dic(in[i]);		
			add_to_decompress_dic(in[i]);
			i++;		
			}
			
			}
			
			if ( flag_bit_position == -1 ){
			flag_bit_position = 7;
			out[outcurs++] = flag;
			flag = 0;

			for ( int z = 0; z <compress_buffer_curs; z++)
				out[outcurs++] = compression_buffer[z];
			compress_buffer_curs = 0;
			}
			
	}
	return outcurs;
}