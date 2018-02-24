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
		zip filestream implementation

	revision history:
		Jan/31/2001 - Timo Saarinen - initial revision/renaissance build
		Feb/27/2002 - Jukka Liimatta - rewrite the codec to be endianess aware
		Mar/03/2002 - Francesco Montecuccoli - rewrite the codec to use a Stream object
		Apr/03/2003 - Jukka Liimatta - bigendian support
*/
#include "../../src/extlib/zlib/zlib.h"
#include <prcore/configure.hpp>
#include <prcore/exception.hpp>
#include <prcore/memorystream.hpp>
#include <prcore/filestream.hpp>
#include <prcore/filename.hpp>
#include <prcore/endian.hpp>

using namespace prcore;


// =================================================
// zip structures
// =================================================

struct LocalFileHeader
{
	uint32	signature;			// 0x04034b50 ("PK..")
	uint16	versionNeeded;		// version needed to extract
	uint16	flags;				// 	
	uint16	compression;		// compression method
	uint16	lastModTime;		// last mod file time
	uint16	lastModDate;		// last mod file date
	uint32	crc;				//
	uint32	compressedSize;		// 
	uint32	uncompressedSize;	// 
	uint16	filenameLen;		// length of the filename field following this structure
	uint16	extraFieldLen;		// length of the extra field following the filename field

	void UpdateHeader(Stream& buffer)
	{
		signature			= ReadLittleEndian<uint32>(buffer);
		versionNeeded		= ReadLittleEndian<uint16>(buffer);
		flags				= ReadLittleEndian<uint16>(buffer);
		compression			= ReadLittleEndian<uint16>(buffer);
		lastModTime			= ReadLittleEndian<uint16>(buffer);
		lastModDate			= ReadLittleEndian<uint16>(buffer);
		crc					= ReadLittleEndian<uint32>(buffer);
		compressedSize		= ReadLittleEndian<uint32>(buffer);
		uncompressedSize	= ReadLittleEndian<uint32>(buffer);
		filenameLen			= ReadLittleEndian<uint16>(buffer);
		extraFieldLen		= ReadLittleEndian<uint16>(buffer);

		if ( signature != 0x04034b50 )
			PRCORE_EXCEPTION("ZIP decoder, incorrect LocalFileHeader.");
	}
};

struct DirFileHeader
{
	uint32	signature;			// 0x02014b50
	uint16	versionUsed;		//
	uint16	versionNeeded;		//
	uint16	flags;				//
	uint16	compression;		// compression method
	uint16	lastModTime;		//
	uint16	lastModDate;		//
	uint32	crc;				//
	uint32	compressedSize;		//
	uint32	uncompressedSize;	//
	uint16	filenameLen;		// length of the filename field following this structure
	uint16	extraFieldLen;		// length of the extra field following the filename field
	uint16	commentLen;			// length of the file comment field following the extra field
	uint16	diskStart;			// the number of the disk on which this file begins
	uint16	internal;			// internal file attributes
	uint32	external;			// external file attributes
	uint32	localOffset;		// relative offset of the local file header

	String	filename;

	void UpdateHeader(Stream& buffer)
	{
		signature			= ReadLittleEndian<uint32>(buffer);
		versionUsed			= ReadLittleEndian<uint16>(buffer);
		versionNeeded		= ReadLittleEndian<uint16>(buffer);
		flags				= ReadLittleEndian<uint16>(buffer);
		compression			= ReadLittleEndian<uint16>(buffer);
		lastModTime			= ReadLittleEndian<uint16>(buffer);
		lastModDate			= ReadLittleEndian<uint16>(buffer);
		crc					= ReadLittleEndian<uint32>(buffer);
		compressedSize		= ReadLittleEndian<uint32>(buffer);
		uncompressedSize	= ReadLittleEndian<uint32>(buffer);
		filenameLen			= ReadLittleEndian<uint16>(buffer);
		extraFieldLen		= ReadLittleEndian<uint16>(buffer);
		commentLen			= ReadLittleEndian<uint16>(buffer);
		diskStart			= ReadLittleEndian<uint16>(buffer);
		internal			= ReadLittleEndian<uint16>(buffer);
		external			= ReadLittleEndian<uint32>(buffer);
		localOffset			= ReadLittleEndian<uint32>(buffer);

		if ( signature != 0x02014b50 )
			PRCORE_EXCEPTION("ZIP decoder, incorrect DirFileHeader.");
	}
};

struct DirEndRecord
{
	uint32	signature;			// 0x06054b50
	uint16	thisDisk;			// number of this disk
	uint16	dirStartDisk;		// number of the disk containing the start of the central directory
	uint16	numEntriesOnDisk;	// # of entries in the central directory on this disk
	uint16	numEntriesTotal;	// total # of entries in the central directory
	uint32	dirSize;			// size of the central directory
	uint32	dirStartOffset;		// offset of the start of central directory on the disk where the central directory begins
	uint16	commentLen;			// zip file comment length

	void UpdateRecord(Stream& buffer)
	{
		signature			= ReadLittleEndian<uint32>(buffer);
		thisDisk			= ReadLittleEndian<uint16>(buffer);
		dirStartDisk		= ReadLittleEndian<uint16>(buffer);
		numEntriesOnDisk	= ReadLittleEndian<uint16>(buffer);
		numEntriesTotal		= ReadLittleEndian<uint16>(buffer);
		dirSize				= ReadLittleEndian<uint32>(buffer);
		dirStartOffset		= ReadLittleEndian<uint32>(buffer);
		commentLen			= ReadLittleEndian<uint16>(buffer);

		if ( signature != 0x06054b50 )
			PRCORE_EXCEPTION("ZIP decoder, incorrect DirEndRecord.");
	}
};


// =================================================
// zip functions
// =================================================

enum
{
	dckeysize           = 12, // decrypt header size
	DirEndRecordSize    = 22, // sizeof(DirEndRecord)
	LocalFileHeaderSize = 30  // sizeof(LocalFileHeader)
};

static char zip_password[256];
static int  zip_password_length = 0;
static const unsigned long* l_crcTable = get_crc_table();


static uint32 crc32(uint32 crc, unsigned char ch)
{
	return l_crcTable[((int)crc ^ (ch)) & 0xff] ^ (crc >> 8);
}


static void UpdateKeys(uint32* keys, int ch)
{
	keys[0] = crc32(keys[0],ch);
	keys[1] = (keys[1] + (keys[0] & 0xff)) * 134775813L + 1;
	keys[2] = crc32(keys[2],keys[1] >> 24);
}


static int DecryptByte(uint32* keys)
{
	uint32 temp = (keys[2] & 0xffff) | 2;
	return (((temp * (temp ^ 1)) >> 8) & 0xff);
}


static uint8* zipDecrypt(const uint8* src, int size, uint8 dcheader[], int version, uint32 crc)
{
	if ( zip_password_length < 1 )
		return NULL;
		
	// init keys with password
	uint32 key[3] = { 305419896L, 591751049L, 878082192L };

	int i;
	for ( i=0; i<zip_password_length; ++i )
	{
		UpdateKeys(key,zip_password[i]);
	}

	// decrypt the 12 byte encryption header
	uint8 fileKey[dckeysize];
	memcpy(fileKey,dcheader,dckeysize);
	for ( i=0; i<dckeysize; ++i )
	{
		uint8 ch = fileKey[i] ^ DecryptByte(key);
		UpdateKeys(key,ch);
		fileKey[i] = ch;
	}

	// check that password is correct one
	if ( version < 20 )
	{
		// version prior 2.0
		if ( *((uint16*)&fileKey[10]) != (crc >> 16) )
			return NULL;
	} 
	else 
	{
		// version 2.0+
		if ( fileKey[11] != (crc >> 24) )
			return NULL;
	}

	// read compressed data & decrypt
	uint8* buffer = new uint8[size];
	memcpy(buffer,src,size);

	for ( i=0; i<size; ++i )
	{
		uint8 ch = buffer[i];
		ch ^= DecryptByte(key);
		buffer[i] = ch;
		UpdateKeys(key,ch);
	}

	return buffer;
}


uint32 zipDecompress(uint8* compressed, uint8* uncompressed, uint32 compressedLen, uint32 uncompressedLen)
{
    int err;
    z_stream d_stream; // decompression stream

	memset(&d_stream,0,sizeof(d_stream));

	d_stream.zalloc = (alloc_func)0;
    d_stream.zfree	= (free_func)0;
    d_stream.opaque = (voidpf)0;
	
    d_stream.next_in  = compressed;
    d_stream.avail_in = compressedLen + 1;//TODO: correct?

	err = inflateInit2(&d_stream,-MAX_WBITS);
	if ( err != Z_OK )
	{
		PRCORE_EXCEPTION("ZIP file decompression (inflateInit2) failed.");
		return 0;
	}

	for ( ;; )
	{
		d_stream.next_out  = uncompressed;
		d_stream.avail_out = uncompressedLen;

		err = inflate(&d_stream,Z_FINISH);
		if ( err == Z_STREAM_END )
			break;
      
		if ( err != Z_OK )
		{
			switch( err )
			{
				case Z_MEM_ERROR:	PRCORE_EXCEPTION("ZIP file decompression failed (memory error).");
				case Z_BUF_ERROR:	PRCORE_EXCEPTION("ZIP file decompression failed (buffer error).");
				case Z_DATA_ERROR:	PRCORE_EXCEPTION("ZIP file decompression failed (data error).");
				default:			PRCORE_EXCEPTION("ZIP file decompression failed (unknown error).");
			}
			return 0;
		}
	}

	err = inflateEnd( &d_stream );
	if ( err != Z_OK )
	{
		PRCORE_EXCEPTION("ZIP file decompression (inflateEnd) failed.");
		return 0;
	}

	return d_stream.total_out;
}


// =================================================
// unzip
// =================================================

// checks (case insensitive) whether "string" starts with, but
// is not equal to, "prefix"
static inline bool strisprefix(const char* string, const char* prefix)
{
	while ( *string && *prefix )
	{
		if ( tolower(*string++) != tolower(*prefix++) )
			return false;
	}
	return !*prefix && *string;
}


class unzip
{
	public:

	unzip(Stream* archive)
	: mIsOpen(false),mpData(archive)
	{
		if ( !archive )
			return;

		// read relevant structures from zip
		if ( !ReadEndRecord() )
			return;

		ReadDirFileHeaders();
		mIsOpen = true;
	}

	~unzip()
	{
		CloseFile();
	}

	int GetFileIndex(const String& path)
	{
		int numfile = mFileInfoArray.GetSize();
		for ( int i=0; i<numfile; ++i )
		{
			if ( StringCompare(path,mFileInfoArray[i].filename,false) )
				return i;
		}
		return -1;	
	}

	uint8* LoadFileFromZIP(int fileIndex, uint32& rSize)
	{
		if ( fileIndex < 0 || fileIndex >= mFileInfoArray.GetSize() )
			return 0;

		// current file header
		DirFileHeader header = mFileInfoArray[fileIndex];

		// local file header	
		LocalFileHeader lhdr;
		mpData->Seek(header.localOffset,Stream::START);
		lhdr.UpdateHeader(*mpData);

		// seek stream to correct offset
		int offset = (lhdr.compressedSize != header.compressedSize) ? 0 :
			header.localOffset + LocalFileHeaderSize + lhdr.filenameLen + lhdr.extraFieldLen;

		mpData->Seek(offset,Stream::START);

		// read decrypting header, if any..
		uint8 dcheader[dckeysize];
		if ( header.flags & 1 )
		{
			mpData->Read(dcheader,dckeysize);
		}

		// read source buffer
		bool srcAllocated = false;
		uint32 srcLen     = header.compressedSize;
		uint8* pSrcData   = new uint8[srcLen];
		uint8* pSrc       = pSrcData;
		mpData->Read(pSrc,srcLen);

		// allocate destination buffer
		uint32 destLen = header.uncompressedSize;
		uint8* pDest   = new uint8[destLen];

		// encrypted? 
		if ( header.flags & 1 )
		{
			// NOTE: the compressed data stream must be read to memory and decrypted
			pSrc = zipDecrypt(pSrc,srcLen,dcheader,(header.versionUsed & 0xff),header.crc);
			srcAllocated = true;
		}

		// decompress
		switch ( header.compression )
		{
			case 0:	// no compression - stored 
			{		
				memcpy(pDest,pSrc,destLen);
				break;
			}

			case 8: // deflated
			{
				if ( zipDecompress(pSrc,pDest,srcLen,destLen) != destLen )
					return 0;
				break;
			}

			default:
				return 0;
		}

		// if we allocated source buffer to memory (in decrypting), free it
		if ( srcAllocated ) 
			delete[] pSrc;

		// free source buffer;
		delete[] pSrcData;

		// CRC check
		if ( crc32(0,pDest,destLen) != header.crc )
		{
			delete[] pDest;
			return 0;
		}

		// done
		rSize = destLen;
		return pDest;
	}

	bool IsOpen() const
	{
		return mIsOpen;
	}

	void GetIndexList(const char* filepath, Array<String>& files, Array<String>& folders)
	{
		// seek last '/' in filepath (in case filepath ends with
		// part of a filename)
		const char* p = filepath + strlen(filepath);
		while ( p > filepath && *(p - 1) != '/' )
			p--;

		int numfile = mFileInfoArray.GetSize();
		for ( int n=0; n<numfile; ++n )
		{
			if ( strisprefix(mFileInfoArray[n].filename,filepath) )
			{
				// extract complete filename from fullpath
				char* filename = mFileInfoArray[n].filename + static_cast<int>(p - filepath);

				// do not recurse subdirs
				char* pos = strstr(filename, "/");
				if ( !pos )
				{
					if ( FileCodecManager::manager.FindInterface(GetFilenameEXT(filename)) )
					{
						// let it look like a folder
						String foldername(filename);
						foldername += "/";
						folders.PushBack(foldername);
					}
					else
					{
						files.PushBack(filename);
					}
				}
				else if ( pos[1] == 0 )
				{
					folders.PushBack(filename);
				}
			}	
		}
	}

	bool IsFolder(const String& path)
	{
		int numfile = mFileInfoArray.GetSize();
		for ( int n=0; n<numfile; ++n )
		{
			if ( strisprefix(mFileInfoArray[n].filename,path) )
			{
				// extract complete filename from fullpath
				char* filename = mFileInfoArray[n].filename + path.GetLength();

				// it is a folder if last char is "/"
				char* pos = strstr(filename, "/");
				return pos && pos[1] == 0;
			}	
		}
		return false;
	}

	private:

	typedef prcore::Array<DirFileHeader> FileInfoArray;

	bool			mIsOpen;
	Stream*			mpData;
	FileInfoArray	mFileInfoArray;
	DirEndRecord	mEndRecord;

	void CloseFile()
	{
		if ( mpData )
			mpData->Release();

		mIsOpen = false;
	}

	bool ReadEndRecord()
	{
		// find central directory end record signature ( 0x06054b50 )
		// by scanning backwards from the end of the file (TODO)
		uint32 filesize = mpData->GetSize();
		for ( uint32 pos = filesize - DirEndRecordSize; pos > 0; pos-- )
		{
			mpData->Seek(pos,Stream::START);
			uint32 ders = ReadLittleEndian<uint32>(*mpData);
			if ( ders == 0x06054b50 )
			{
				// found, read the end record and return
				mpData->Seek(pos,Stream::START);
				mEndRecord.UpdateRecord(*mpData);

				// fail, if multi-volume zip
				if ( mEndRecord.thisDisk != 0 || mEndRecord.dirStartDisk != 0 ||
					mEndRecord.numEntriesOnDisk != mEndRecord.numEntriesTotal )
					return false;

				return true;
			}
		}
		return false;
	}

	void ReadDirFileHeaders()
	{
		int numfile = mEndRecord.numEntriesTotal;
		mFileInfoArray.SetSize(numfile);

		// read file header for each file
		mpData->Seek(mEndRecord.dirStartOffset,Stream::START);

		for ( int n=0; n<numfile; ++n )
		{
			DirFileHeader& header = mFileInfoArray[n];
			header.UpdateHeader(*mpData);

			// read filename
			int size = header.filenameLen;
			char* text = header.filename.SetLength(size);
			mpData->Read(text,size);

			// seek over extra field + comment
			mpData->Seek(header.extraFieldLen + header.commentLen, Stream::CURRENT);
		}
	}
};


// =================================================
// codec
// =================================================

static Stream* create_stream(Stream* archive, const String& filename, Stream::AccessMode mode, void* password)
{
	if ( mode != Stream::READ )
		return NULL;

	// create unzip
	unzip* u = new unzip(archive);

	if ( !u->IsOpen() )
	{
		delete u;
		return NULL;
	}

	// get fileindex
	int index = u->GetFileIndex(filename);
	if( index < 0 )
	{
		delete u;
		return NULL;
	}

	// set decrypt password
	if ( password )
	{
		char* pass = reinterpret_cast<char*>(password);
		strcpy(zip_password,pass);
		zip_password_length = static_cast<int>(strlen(pass));
	}

	// decompress whole file to memory
	uint32 size = 0;
	uint8* data = u->LoadFileFromZIP(index,size);
	if ( !data || !size )
	{
		delete u;
		return NULL;
	}

	// release unzip
	delete u;

	// create zip interface
	MemoryStream* z = new MemoryStream(data,size);
	return z;
}


static void create_index(Stream* archive, const String& path, Array<String>& files, Array<String>& folders)
{
	// create unzip
	unzip* u = new unzip(archive);

	if ( u->IsOpen() )
	{
		u->GetIndexList(path,files,folders);
	}

	// release unzip
	delete u;
}


static bool is_folder(Stream* archive, const String& path)
{
	// create unzip
	unzip* u = new unzip(archive);

	bool isfolder = false;
	if ( u->IsOpen() )
	{
		isfolder = u->IsFolder(path);
	}

	// release unzip
	delete u;
	return isfolder;
}


// =================================================
// CreateCodecZIP()
// =================================================

FileCodec CreateCodecZIP()
{
	static const String ext[] = { "zip", "pk3" };

	FileCodec codec;
	
	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.stream    = create_stream;
	codec.index     = create_index;
	codec.isfolder  = is_folder;

	return codec;
}
