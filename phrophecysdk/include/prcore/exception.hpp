/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_EXCEPTION_HPP
#define PRCORE_EXCEPTION_HPP

#include "configure.hpp"
#include "string.hpp"

namespace prcore
{

	class Bitmap;

	class Exception
	{
		public:

		Exception();
		Exception(const String& method, const String& message, const String& filename = "", int linenr = -1);
		Exception(const Exception& other);
		virtual	~Exception();

		void operator = (const Exception& other);

		// set user's own logo for the exception dialog (128 x 94 x 256c)
		static bool SetLogo(const Bitmap& logo);

		// draw the exception dialog
		void DrawDialog();

		const String& GetFilename() const
		{
			// source file which did throw the exception
			return mFilename;
		}

		const String& GetMethod() const
		{
			// method/function which did throw the exception
			return mMethod;
		}

		const String& GetMessage() const
		{
			// user message attached to the exception
			return mMessage;
		}

		int GetLinenr() const
		{
			// line number the exception was thrown from
			return mLinenr;
		}

		private:

		String	mMethod;
		String	mMessage;
		String	mFilename;
		int		mLinenr;
	};

	// exception macros

	#define PRCORE_EXCEPTION(message) \
        do { \
            throw prcore::Exception(PRCORE_FUNCTION,message,PRCORE_FILENAME,PRCORE_LINE); \
        } while ( 0 )

    #define PRCORE_ASSERT_EXCEPTION( expression ) \
        do { \
            if(expression) \
                throw prcore::Exception(PRCORE_FUNCTION,"N/A",PRCORE_FILENAME,PRCORE_LINE); \
        } while( 0 )

} // namespace prcore

#endif
