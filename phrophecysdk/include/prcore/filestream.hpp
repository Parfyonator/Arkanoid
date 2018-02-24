/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_FILESTREAM_HPP
#define PRCORE_FILESTREAM_HPP


#include "stream.hpp"
#include "string.hpp"
#include "codecmanager.hpp"
#include "array.hpp"


namespace prcore
{

	class FileStream : public Stream
	{
		public:
		
		FileStream(const String& filename, AccessMode mode = READ);
		~FileStream();
		
		int			Seek(int delta, SeekMode mode);
		int			Read(void* target, int bytes);
		int			Write(const void* source, int bytes);
		int			GetOffset() const;
		int			GetSize() const;
		bool		IsOpen() const;
		bool		IsEOS() const;

static	void		SetPassword(const String& password);
static	void		SetBasePath(const String& path);
static	String		GetBasePath();

		private:
		
		enum { CACHE_BITS = 12, CACHE_SIZE = (1 << CACHE_BITS) };
		
		Stream*		stream;
		int			streamsize;

		bool		iscache;
		char		cache[CACHE_SIZE];
		int			offset;
		int			page;
	};
	
	
	class FileIndex
	{
		public:

		FileIndex();
		FileIndex(const String& userpath);
		~FileIndex();

		void			CreateIndex(const String& userpath);

		int				GetFileCount() const;
		int				GetFolderCount() const;
		const String&	GetFileName(int index) const;
		const String& 	GetFolderName(int index) const;
		bool			IsFile(const String& filename) const;
		bool			IsFolder(const String& foldername) const;

		private:

		Array<String>	mFileArray;
		Array<String>	mFolderArray;
	};


	struct FileCodec : public CodecInterface
	{
		Stream* (*stream)  (Stream* archive, const String& filename, Stream::AccessMode mode, void* password);
		void    (*index)   (Stream* archive, const String& path, Array<String>& filename, Array<String>& foldername);
		bool    (*isfolder)(Stream* archive, const String& path);
	};


	class FileCodecManager : public CodecManager<FileCodec>
	{
		public:

		friend class prcore::FileStream;

		static FileCodecManager manager;

		int GetVersion() const;

		private:

		FileCodecManager();
		FileCodecManager(const FileCodecManager&);
		FileCodecManager& operator = (const FileCodecManager&);
	};


	inline const FileCodec* FindFileCodec(const String& ext)
	{
		return FileCodecManager::manager.FindInterface(ext);
	}

} // namespace prcore


#endif