/** newline (a tool for the linux build of Prophecy sdk)
    (C) 2001 - Edmond Cote (edmond.cote@twilight3d.com)

    see README file for more information.

    Little program to add newlines at the end of each file, since gcc 2.96 and
    above will give out warnings when a newline is not found at the end of a
    file. Occassionally, gcc will also crash when too many of those errors
    are found.

    On a final note, there is no doubt in my find that it would of been more
    efficient to use 'x' scripting language. As it stood, I did try to do 
    something in Perl, but it didn't work out. So if evreyone as a nicer way
    to do this, please let me know

    April/13/2003 - Modularized gccfix(), and added crfix() - JTL
*/
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>


static int exists(const char* filename)
{
	DIR* dir = opendir(filename);
	if ( dir != NULL )
	{
		closedir(dir);
	}
	else
	{
		FILE* file = fopen(filename,"ro");
		if ( file != NULL )
		{
			fclose(file);
			return 1;
		}
	}
	return 0;
}


static void gccfix(const char* filename)
{
	FILE* file = fopen(filename,"a");
	if ( file != NULL )
	{
		fprintf(file,"\n");
		fclose(file);
	}
}


static void crfix(const char* filename)
{
	FILE* file = fopen(filename,"rb");
	if ( file != NULL )
	{
		char* buffer0;
		char* buffer1;
		char* p;
		int size;
		int i;

		fseek(file,0,SEEK_END);	size = (int)ftell(file);
		fseek(file,0,SEEK_SET);

		buffer0 = (char*)malloc(size);
		buffer1 = (char*)malloc(size);

		fread(buffer0,1,size,file);
		fclose(file);

		p = buffer1;
		for ( i=0; i<size; ++i )
		{
			char v = buffer0[i];
			if ( v != 0xd )
				*p++ = v;
		}

		size = p - buffer1;
		file = fopen(filename,"w+b");
		fwrite(buffer1,1,size,file);
		fclose(file);

		free(buffer0);
		free(buffer1);
	}
}


int main(int argc, char** argv)
{
	int i;
	FILE* file;

	if ( argc < 2 )
	{
		fprintf(stderr,"newline: not enough arguments\n");
		exit(1);
	}

	if ( strcmp(argv[1],"--help") == 0 )
	{
		fprintf(stderr,
			"newline: a tool for the linux build of prophecy sdk\n"
			"see README file for more information\n");
		exit(1);
	}

	for( i=1; i<argc; ++i )
	{
		const char* filename = argv[i];

		fprintf(stdout,"%s\n",filename);

		if ( exists(filename) == 1 )
		{
			gccfix(filename);
			crfix(filename);
		}
		else
		{
			fprintf(stderr,"file: %s, does not exist!\n",filename);
			exit(1); 
		}
	} 

	exit(0);
}
