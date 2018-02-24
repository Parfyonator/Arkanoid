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
		filestream implementation

	revision history:
		Jan/24/2001 - Jukka Liimatta - initial revision/renaissance build
		Jan/30/2001 - Timo Saarinen - read cache
		Feb/01/2001 - Jukka Liimatta - filefactory
		Oct/22/2001 - Jukka Liimatta - added stristr() to make detecting filesystem non-casesensitive
		Feb/08/2002 - Francesco Montecuccoli - fixed error in file(-system) extension detection
		Feb/15/2002 - Jukka Liimatta - fixed write seek bug (writes don't go through cache, so seek must be forwarded to the actual filesystem plugin)
		Feb/23/2002 - Jukka Liimatta - created FileCodecManager
		Feb/23/2002 - Francesco Montecuccoli - created FileIndex
		Mar/03/2002 - Francesco Montecuccoli - added support for nested file codecs
		Sep/17/2002 - Jukka Liimatta - fixed "numbytes read" return value for ::Read() method in case of overflow
		                               fixed "current offset" return value in ::Seek() method in case of under-/overflow
		                               added "streamsize" member to FileStream for cached lookup of the value
		Mar/05/2003 - Francesco Montecuccoli - fixed a bug in codec selection order
*/
#include <prcore/exception.hpp>
#include <prcore/linkedlist.hpp>
#include <prcore/filestream.hpp>
#include <prcore/filename.hpp>

using namespace prcore;


// ========================================
// stristr()
// ========================================

static const char* stristr(const char* string, const char* ext)
{
	for ( ;; )
	{
		const char* s = string;

		// substring search loop
		for ( const char* e=ext; *e; ++e,++s )
		{
			if ( !*s )
				return NULL;

			if ( prcore::tolower(*s) != prcore::tolower(*e) )
			{
				// the substring is not same, abort search
				s = NULL;
				break;
			}
		}

		// the substring search loop succeeded
		if ( s )
			return string;

		++string;
	}
}


// ========================================
// global strings
// ========================================

static String password;
static String basepath;


// ========================================
// FileCodecManager
// ========================================

	// the manager instance
	FileCodecManager FileCodecManager::manager;

	// forward declarations
	FileCodec CreateCodecFILE();
	FileCodec CreateCodecZIP();


FileCodecManager::FileCodecManager()
{
	manager.RegisterInterface(CreateCodecZIP());
}


int FileCodecManager::GetVersion() const
{
	return 1;
}


// ========================================
// FindFileCodec
// ========================================

static const FileCodec* FindFileCodec(Stream* archive, const FileCodec* current, const String& fullpath, int& pos)
{
	// find a codec which is suitable for the leftmost (in fullpath) archive
	// e.g. in "data.pk3/data.zip/" we must return a codec for pk3 and not for zip

	const FileCodec* found = NULL;

	// position in fullpath after archive name
	pos = fullpath.GetLength();
	const char* fp = static_cast<const char*>(fullpath);

	// iterate looking for a filefactory, last registered are checked first
	for ( int i=FileCodecManager::manager.GetInterfaceCount()-1; i>=0; --i )
	{
		const FileCodec* codec = FileCodecManager::manager.GetInterface(i);

		for ( int j=0; j<codec->count; ++j )
		{
			// next supported extension
			String ext;
			ext << "." << codec->extension[j];

			// find extension as substring of filename
			const char* p = stristr(fullpath,ext);
			
			// be sure you actually got the extension
			while ( p )
			{
				// found extension, skip it to get filename
				p += ext.GetLength();

				// filename must be prefixed with slash
				if ( *p=='/' || *p=='\\' )
				{
					// newpos is where the archive name ends
					// and filename starts...
					int newpos = static_cast<int>(p - fp);

					if ( newpos < pos )
					{
						// be sure we have not found a real folder
						if ( archive )
							archive->AddRef(); // don't want to release this stream

						if ( !current->isfolder(archive,fullpath.SubString(0,pos)) )
						{
							// mission impossible complete
							pos = newpos;
							found = codec;
							break;
						}
					}
				}

				// try again...
				p = stristr(p,ext);
			}
		}
	}

	if ( found )
	{
		return found;
	}
	else
	{
		// not found any other codecs, keep current
		pos = -1;
		return current;
	}
}


// WARNING: parameter filename can be modified to represent the path and file name
// relative to the archive (that's why it is not const)
static const FileCodec RecursiveFindFileCodec(Stream*& archive, String& filename)
{
	// find appropriate codec for the filename
	int pos = 0;
	FileCodec defcodec = CreateCodecFILE();
	FileCodec codec    = *FindFileCodec(archive,&defcodec,filename,pos);

	// cycle through inner codecs
	// pos == -1 means we have not found any other codecs
	while ( pos != -1 )
	{
		archive = defcodec.stream(archive,filename.SubString(0,pos),Stream::READ,password);
		if ( !archive )
		{
			String error = String("cannot access file: ") + filename;
			PRCORE_EXCEPTION(error);
		}

		defcodec = codec;
		filename = String(static_cast<char*>(filename) + pos + 1);
		codec    = *FindFileCodec(archive,&defcodec,filename,pos);
	}
	return codec;
}


// ========================================
// RemapFilename()
// ========================================

String RemapFilename(const String& filename);


// ========================================
// FileStream
// ========================================

FileStream::FileStream(const String& filename_user, AccessMode mode)
: stream(NULL),streamsize(0),offset(0),page(-1)
{
	// read cache setup
	iscache = (mode == Stream::READ);

	// create filename at basepath
	String filename = RemapFilename(filename_user);

	// store stream/filename
	SetName(filename);

	// find appropriate codec for the filename
	Stream* archive = NULL;
	const FileCodec codec = RecursiveFindFileCodec(archive,filename);

	// finally create stream
	stream = codec.stream(archive,filename,mode,password);
	if ( !stream )
	{
		String error = String("cannot access file: ") + filename;
		PRCORE_EXCEPTION(error);
	}

	// cache streamsize
	streamsize = stream->GetSize();
}


FileStream::~FileStream()
{
	delete stream;
}


int FileStream::Seek(int delta, SeekMode mode)
{
	switch ( mode )
	{
		case START:		offset = delta; break;
		case CURRENT:	offset += delta; break;
		case END:		offset = streamsize - delta; break;
	}
		
	// assert offset range
	assert( offset >= 0 && offset <= streamsize );
		
	if ( !iscache )
		stream->Seek(offset,Stream::START);

	return offset;
}


int FileStream::Read(void* target, int bytes)
{
	// adjust bytes for overflow
	int oversize = offset + bytes - streamsize;
	if ( oversize > 0 )
	{
		bytes -= oversize;
		if ( bytes < 1 )
			return 0;
	}

	int offset0 = offset;

	if ( bytes > CACHE_SIZE )
	{
		stream->Seek(offset,START);
		stream->Read(target,bytes);
		offset += bytes;

		// invalidate cache after blockread
		page = -1;
	}
	else
	{
		char* cbuffer = reinterpret_cast<char*>(target);
		int cpos = offset;
		int cpage = cpos >> CACHE_BITS;

		// read page, if not cached
		if ( cpage != page )
		{
			stream->Seek(cpage<<CACHE_BITS,START);
			stream->Read(cache,CACHE_SIZE);
			page = cpage;
		}

		// cache boundry cross
		int bytes_left = CACHE_SIZE - (cpos & (CACHE_SIZE-1));
		while ( bytes > bytes_left )
		{
			memcpy(cbuffer,cache+(cpos & (CACHE_SIZE-1)),bytes_left);
			cpos += bytes_left;
			cbuffer += bytes_left;
			bytes -= bytes_left;

			stream->Seek(cpos,START);
			stream->Read(cache,CACHE_SIZE);
			page = cpos >> CACHE_BITS;
			bytes_left = CACHE_SIZE;
		}

		// leftovers
		memcpy(cbuffer,cache+(cpos & (CACHE_SIZE-1)),bytes);
		offset = cpos + bytes;
	}

	return offset - offset0;
}


int FileStream::Write(const void* source, int bytes)
{
	int delta = stream->Write(source,bytes);
	offset += delta;
	return delta;
}


int FileStream::GetOffset() const
{
	return offset;
}


int FileStream::GetSize() const
{
	return streamsize;
}


bool FileStream::IsOpen() const
{
	return stream->IsOpen();
}


bool FileStream::IsEOS() const
{
	return offset >= streamsize;
}


void FileStream::SetBasePath(const String& path)
{
	basepath = path;
}


void FileStream::SetPassword(const String& pass)
{
	password = pass;
}


String FileStream::GetBasePath()
{
	return basepath;
}


// =================================================
// FileIndex
// =================================================

FileIndex::FileIndex()
{
	CreateIndex("");
}


FileIndex::FileIndex(const String& userpath)
{
	CreateIndex(userpath);
}


FileIndex::~FileIndex()
{
}


void FileIndex::CreateIndex(const String& userpath)
{
	// apply basepath to userpath
	String path = basepath + userpath;

	// reset index
	mFileArray.ResetIndex();
	mFolderArray.ResetIndex();

	// find appropriate codec for the filename
	Stream* archive = NULL;
	const FileCodec codec = RecursiveFindFileCodec(archive,path);
	
	// finally access index
	codec.index(archive,path,mFileArray,mFolderArray);
}


int FileIndex::GetFileCount() const
{
	return mFileArray.GetSize();
}


int FileIndex::GetFolderCount() const
{
	return mFolderArray.GetSize();
}


const String& FileIndex::GetFileName(int index) const
{
	return mFileArray[index];
}


const String& FileIndex::GetFolderName(int index) const
{
	return mFolderArray[index];
}


bool FileIndex::IsFile(const String& filename) const
{
	int count = GetFileCount();
	for ( int i=0; i<count; ++i )
	{
		if ( StringCompare(mFileArray[i],filename) )
			return true;
	}
	return false;
}


bool FileIndex::IsFolder(const String& foldername) const
{
	int count = GetFolderCount();
	for ( int i=0; i<count; ++i )
	{
		if ( StringCompare(mFolderArray[i],foldername) )
			return true;
	}
	return false;
}

