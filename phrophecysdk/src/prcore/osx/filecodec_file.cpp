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
 
    December/25/2003 - Stefan Karlsson based on UNIX version of this file:
 */

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <prcore/string.hpp>
#include <prcore/filestream.hpp>
#include <prcore/filename.hpp>
#include <prcore/exception.hpp>

namespace prcore
{
    

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


}

