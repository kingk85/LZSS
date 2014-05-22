#include <iostream>
#include "compressor_c.cpp"
#include "fstream"

using namespace std;

long long GetfileSize( char* filePath ){
  long long begin, end;
  ifstream myfile (filePath);
  begin = myfile.tellg();
  myfile.seekg (0, ios::end);
  end = myfile.tellg();
  myfile.close();
  return (end-begin);
}

int main (int argc, char ** argv) {

int toread;
unsigned char buffer_in[900000];
unsigned char buffer_out[900000];

if (argc < 3 ){
printf("Usage\n Compress a file: compressor -c source dest\n Decompress a file: compressor -d source dest\n");
return 0;
}



if ( argv[1][0] == '-' && argv[1][1] == 'c' )
{
compressor c;

ifstream file_in;
file_in.open(argv[2],ios::in | ios::binary );
ofstream file_out;
file_out.open(argv[3], ios::out|ios::binary);

int recv = 0;
long long input_size = 0;
 input_size = GetfileSize(argv[2]);
long long compressed_size = 0;
long long processed_size = 0;
int print_after =0;
long long toread;
long long processed = 0;

cout<<"Compressing "<<argv[2]<<"( "<<input_size<<" bytes)"<<endl<<endl;

toread = 100000;

while ( processed < input_size )
{
	if (processed + 100000 > input_size)
		toread = input_size - processed;
		
	processed+= toread;	
	file_in.read ((char*)buffer_in, (int)toread);
	recv = c.compress_buffer(buffer_in, (int)toread, buffer_out, 100000 );
	compressed_size+= recv;
	
	if ( recv > 0 )
	file_out.write((char*)buffer_out, recv);
	
	if ( print_after >= 10 ){
	if ( input_size >= 100 )
	cout<<(processed/(input_size/100))<<" % "<<"(processed "<< processed<<" out of "<<input_size<<" bytes)\r";
	cout.flush();
	print_after = 0;
	}
	
	print_after++;
}

recv = c.compress_buffer(buffer_in, 0, buffer_out, 0 );
compressed_size+= recv;
if ( recv > 0 )
	file_out.write((char*)buffer_out, recv);


if ( input_size >= 100 )
cout<<(processed/(input_size/100))<<" % "<<"(processed "<< processed<<" out of "<<input_size<<" bytes)\r";
cout<<endl<<"All done!"<<endl;
	
file_in.close();
file_out.flush();
file_out.close();

if ( input_size >= 100 && compressed_size>= 100 )
cout<<endl<<"Compressed size: "<<compressed_size<<" - compression ratio "<<((compressed_size/(input_size/100)))<<" %"<<endl<<endl;
}


if ( argv[1][0] == '-' && argv[1][1] == 'd' )
{
compressor c;

ifstream file_in;
file_in.open(argv[2],ios::in | ios::binary );
ofstream file_out;
file_out.open(argv[3], ios::out|ios::binary);

int recv = 0;
long long input_size = GetfileSize(argv[2]);
long long compressed_size = 0;
long long processed_size = 0;
int print_after =0;

long long processed = 0;
cout<<"Decompressing "<<argv[2]<<"( "<<input_size<<" bytes)"<<endl<<endl;
long long toread = 1000;
while ( processed < input_size )
{
	if (processed + 1000 > input_size)
		toread = input_size - processed;
		
	processed+= toread;
	file_in.read ((char*)buffer_in, toread);
	recv = c.decompress_buffer(buffer_in, (int)toread, buffer_out, 4096 );
	compressed_size+= recv;
	file_out.write((char*)buffer_out, recv);

	if ( print_after >= 10000 ){
	if ( input_size >= 100 )
	cout<<(processed/(input_size/100))<<" % "<<"(processed "<< processed<<" out of "<<input_size<<" bytes)\r";
	cout.flush();
	print_after = 0;
	}
	
	print_after++;
}
file_in.close();
file_out.flush();
file_out.close();

if ( input_size >= 100 )
cout<<(processed/(input_size/100))<<" % "<<"(processed "<< processed<<" out of "<<input_size<<" bytes)\r";
cout<<endl<<"All done!"<<endl;
}
 

}
