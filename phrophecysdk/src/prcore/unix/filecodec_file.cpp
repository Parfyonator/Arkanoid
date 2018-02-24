/*
 * getexename.c
 *
 * written by Nicolai Haehnle <prefect_@gmx.net>
 * I hereby release this trivial piece of code to the public domain.
 *
 * The function getexename() returns the filename of the currently loaded
 * executable.
 *
 * Intended use of this function is to facilitate easier packaging of 
 * third-party software for the Linux operating system. The FHS mandates
 * that files that belong to one package are scattered throughout the
 * file system. This works as long as packages are maintained by a 
 * package management program. However, it is impossible for application
 * developers to provide packages for every Linux distribution out there.
 * Finding the file locations is also difficult when an application is
 * installed locally by a user inside her own home directory.
 *
 * The simplest and most straight-forward solution to this problem is to
 * put all files belonging to a package into the same directory. The program
 * executable can then reference the necessary data files by using paths
 * relative to the executable location.
 * To give an example:
 *
 *  A simple game, consisting of an executable and a number of data files
 *  (e.g. images), resides entirely in one directory, with absolute filenames
 *  like this:
 *    /the/path/foogame
 *    /the/path/images/hero.png
 *    /the/path/images/badass.png
 *  The game executable can use getexename() to find its own location, strip
 *  off the last component to get the directory the executable is located in,
 *  and append the relative paths "images/hero.png" and "images/badass.png"
 *  to reference the data files.
 *  The game will be completely position independent. The user is free to 
 *  move it somewhere else in the filesystem, and it will just work; it will
 *  no longer be necessary to change configuration files or even recompile the
 *  executable.
 *
 * If you are concerned about executables showing up in a user's PATH, you 
 * should somehow arrange for symlinks to be made. For example, if 
 * /usr/games/foogame is a symlink to /the/path/foogame, the user can run the
 * game simply by typing "foogame" in the shell (provided that /usr/games is in
 * the user's PATH); since symlinks cannot fool getexename(), the game will
 * still work. (Do note that a hard link will defeat getexename()).
 *
 * Note that while it is possible to reference data files based on the current
 * working directory, this technique only works if the user explicitly sets
 * the CWD to the application's base directory. Therefore, using the executable
 * name as a base is more robust.
 *
 * Also note that while argv[0] can be used as the executable name in many 
 * cases as well, it is easily fooled by symlinks and may not contain an 
 * absolute filename. argv[0] can also be set to something entirely different
 * from the executable filename by the executing process, either delibaretly
 * or by invoking scripts.
 *
 * Note that this function relies on the layout of the /proc file system, so
 * portability is an issue. While I assume that this part of /proc is fairly
 * stable, I have no documentation whatsoever about potential differences
 * between Linux kernel versions in this area.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>


/*
 * getexename - Get the filename of the currently running executable
 *
 * The getexename() function copies an absolute filename of the currently 
 * running executable to the array pointed to by buf, which is of length size.
 *
 * If the filename would require a buffer longer than size elements, NULL is
 * returned, and errno is set to ERANGE; an application should check for this
 * error, and allocate a larger buffer if necessary.
 *
 * Return value:
 * NULL on failure, with errno set accordingly, and buf on success. The 
 * contents of the array pointed to by buf is undefined on error.
 *
 * Notes:
 * This function is tested on Linux only. It relies on information supplied by
 * the /proc file system.
 * The returned filename points to the final executable loaded by the execve()
 * system call. In the case of scripts, the filename points to the script 
 * handler, not to the script.
 * The filename returned points to the actual exectuable and not a symlink.
 *
 */
char* getexename(char* buf, size_t size)
{
	char linkname[64]; /* /proc/<pid>/exe */
	pid_t pid;
	int ret;
	
	/* Get our PID and build the name of the link in /proc */
	pid = getpid();
	
	if (snprintf(linkname, sizeof(linkname), "/proc/%i/exe", pid) < 0)
		{
		/* This should only happen on large word systems. I'm not sure
		   what the proper response is here.
		   Since it really is an assert-like condition, aborting the
		   program seems to be in order. */
		abort();
		}

	
	/* Now read the symbolic link */
	ret = readlink(linkname, buf, size);
	
	/* In case of an error, leave the handling up to the caller */
	if (ret == -1)
		return NULL;
	
	/* Report insufficient buffer size */
	if (ret >= (int)size)
		{
		errno = ERANGE;
		return NULL;
		}
	
	/* Ensure proper NUL termination */
	buf[ret] = 0;
	
	return buf;
}


/*
  Twilight Prophecy SDK
  A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
  
  
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
    
    
    source:
    default filestream implementation
    
    revision history:
    Jan/24/2001 - Jukka Liimatta - initial revision/renaissance build
    Feb/20/2002 - Francesco Montecuccoli - added support for FileIndex
    Mar/03/2002 - Francesco Montecuccoli - added support for nested file codecs
    May/12/2002 - Edmond Cote - UNIX implementation
    Apr/08/2003 - Jukka Liimatta - upgraded UNIX implementation
*/
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <prcore/string.hpp>
#include <prcore/filestream.hpp>
#include <prcore/filename.hpp>
#include <prcore/exception.hpp>

using namespace prcore;


// =================================================
// stream implementation
// =================================================

class File : public Stream
{
public:

	File(FILE* file)
	: mpfile(file)
	{
	}

	~File()
	{
		if ( mpfile )
			fclose(mpfile);
	}

	int Seek(int delta, SeekMode mode)
	{
		switch( mode )
		{
			case START:		fseek(mpfile,delta,SEEK_SET); break;
			case CURRENT:	fseek(mpfile,delta,SEEK_CUR); break;
			case END:		fseek(mpfile,delta,SEEK_END); break;
		}
		return ftell(mpfile);
	}

	int Read(void* target, int bytes)
	{
		return static_cast<int>(fread(target,1,bytes,mpfile));
	}

	int Write(const void* source, int bytes)
	{
		return static_cast<int>(fwrite(source,1,bytes,mpfile));
	}

	int GetOffset() const
	{
		return static_cast<int>(ftell(mpfile));
	}

	int GetSize() const
	{
		int offset = GetOffset();

		fseek(mpfile,0,SEEK_END);
		int size = GetOffset();

		fseek(mpfile,offset,SEEK_SET);

		return size;
	}

	bool IsOpen() const
	{
		return mpfile != NULL;
	}

	bool IsEOS() const
	{
		int offset = GetOffset();

		fseek(mpfile,0,SEEK_END);
		int size = GetOffset();

		fseek(mpfile,offset,SEEK_SET);

		return offset >= size;
	}

private:

	FILE* mpfile;
};


// =================================================
// codec
// =================================================

static Stream* create_stream(Stream* archive, const String& filename, Stream::AccessMode mode, void* password)
{
	FILE* file = NULL;

	switch( mode )
	{
		case Stream::READ:		file = fopen(filename,"rb"); break;
		case Stream::WRITE:		file = fopen(filename,"wb"); break;
		case Stream::READWRITE:	file = fopen(filename,"r+b"); break;
		default: break;
	}

	if ( !file )
		return NULL;

	File* stream = new File(file);
	return stream;
}


static void create_index(Stream* archive, const String& path, Array<String>& files, Array<String>& folders)
{
	DIR* dirp = opendir(path);
	if( !dirp )
		PRCORE_EXCEPTION("Unable to open directory.");

	dirent* dp;
	while ( (dp = readdir(dirp)) )
	{
		String name(dp->d_name);

		static String s1(".");
		static String s2("..");

		if ( !StringCompare(name,s1) && !StringCompare(name,s2) )
		{
			// check if we are dealing with a folder or file
			String testname = path + name;

			DIR* tempdir = opendir(testname);
			if ( tempdir )
			{
				closedir(tempdir);

				name << '/';
				folders.PushBack(name);
			}
			else
			{
				files.PushBack(name);
			}
		}
	}   

	closedir(dirp);
}


static bool is_folder(Stream* archive, const String& path)
{
	struct stat buf;

	// get data associated with "path"
	if ( stat(path,&buf) != 0 )
		return false;

	return (buf.st_mode & S_IFDIR) != 0;
}


FileCodec CreateCodecFILE()
{
	static const String ext[] = { "" };

	FileCodec codec;

	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.stream    = create_stream;
	codec.index     = create_index;
	codec.isfolder  = is_folder;

	return codec;
}


// =================================================
// RemapFilename
// =================================================

String RemapFilename(const String& userfile)
{
	// absolute path in userfile name
	if ( !userfile.IsEmpty() && userfile[0] == '/' )
		return userfile;

	// absolute path in basepath
	String basepath = FileStream::GetBasePath();
	if ( !basepath.IsEmpty() && userfile[0] == '/' )
		return basepath + userfile;

	// executable path
	char buffer[1024];
	char* exename = getexename(buffer,1024);
	String exepath = GetFilenamePATH(exename);

	// finally..
	return exepath + basepath + userfile;
}
