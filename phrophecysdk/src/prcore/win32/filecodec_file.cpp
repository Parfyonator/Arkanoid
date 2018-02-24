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
*/
#include <cstddef>
#include <cstdio>
#include <io.h>
#include <sys/stat.h>
#include <prcore/filestream.hpp>
#include <prcore/filename.hpp>

using namespace prcore;


// =================================================
// stream implementation
// =================================================

class File : public Stream
{
	public:

	File(FILE* file)
	: mpFile(file)
	{
	}

	~File()
	{
		if ( mpFile )
			fclose(mpFile);
	}

	int Seek(int delta, SeekMode mode)
	{
		switch( mode )
		{
			case START:		fseek(mpFile,delta,SEEK_SET); break;
			case CURRENT:	fseek(mpFile,delta,SEEK_CUR); break;
			case END:		fseek(mpFile,delta,SEEK_END); break;
		}
		return static_cast<int>(ftell(mpFile));
	}

	int Read(void* target, int bytes)
	{
		return static_cast<int>(fread(target,1,bytes,mpFile));
	}

	int Write(const void* source, int bytes)
	{
		return static_cast<int>(fwrite(source,1,bytes,mpFile));
	}

	int GetOffset() const
	{
		return static_cast<int>(ftell(mpFile));
	}

	int GetSize() const
	{
		int offset = static_cast<int>(ftell(mpFile));
	
		fseek(mpFile,0,SEEK_END);
		int size = static_cast<int>(ftell(mpFile));
	
		fseek(mpFile,offset,SEEK_SET);
	
		return size;
	}

	bool IsOpen() const
	{
		return mpFile != NULL;
	}

	bool IsEOS() const
	{
		int offset = static_cast<int>(ftell(mpFile));
		fseek(mpFile,0,SEEK_END);

		int size = static_cast<int>(ftell(mpFile));
		fseek(mpFile,offset,SEEK_SET);

		return offset >= size;
	}

	private:

	FILE* mpFile;
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
	struct _finddata_t c_file;
	long hFile;

	// Find first file in current directory
	String filespec(path);
	filespec << '*';
	if( ( hFile = (long)_findfirst(filespec,&c_file)) != -1L )
	{
		do
		{
			String name(c_file.name);

			// discard current and parent dirs
			if (strcmp(name, ".") && strcmp(name, ".."))
			{
				// check if we got a real folder or a supported file codec
				if ((c_file.attrib & _A_SUBDIR) ||
					FileCodecManager::manager.FindInterface(GetFilenameEXT(name)))
				{
					name << '/';
					folders.PushBack(name);
				}
				else // we got an ordinary file
				{
					files.PushBack(name);
				}
			}
		}
		while (_findnext(hFile,&c_file) == 0);

	   _findclose(hFile);
	}
}


static bool is_folder(Stream* archive, const String& path)
{
	struct _stat buf;
	
	// get data associated with "path"
	if ( _stat(path,&buf) != 0 )
		return false; // unable to retreive data

	return (buf.st_mode & _S_IFDIR) != 0;
}


// =================================================
// CreateCodecFILE()
// =================================================
	
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
	// in windows this is trivial as CWD is folder where executable is at :)
	String filename = FileStream::GetBasePath() + userfile;
	return filename;
}

