/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_BITMAP_HPP
#define PRCORE_BITMAP_HPP


#include "codecmanager.hpp"
#include "stream.hpp"
#include "surface.hpp"
#include "string.hpp"


namespace prcore
{

	class Bitmap : public Surface
	{
		public:

		Bitmap();
		Bitmap(const Bitmap& bitmap);
		Bitmap(const Surface& surface);
		Bitmap(int width, int height, const PixelFormat& pxf);
		Bitmap(const String& filename);
		Bitmap(const String& filename, const PixelFormat& pxf);
		~Bitmap();

		void	operator = (const Bitmap& bitmap);
		Bitmap	GetBitmap(const Rect& area) const;
		void	LoadImage(Stream& stream, const String& filename, const PixelFormat& pxf);
		void	LoadImage(Stream& stream, const String& filename);
		void	LoadImage(const String& filename);
		void	LoadImage(const String& filename, const PixelFormat& pxf);
		void	SetImage(int width, int height, const PixelFormat& pxf, void* image);
		void	ResizeImage(int width, int height, bool filter = true);
		void	ReformatImage(const PixelFormat& pxf);
		void	QuantizeImage(float quality = 0.35f);
	};


	struct BitmapCodec : CodecInterface
	{
		bool (*decode)(Bitmap& target, Stream& stream);
		bool (*encode)(Stream& target, Surface& bitmap, float quality);
	};


	class BitmapCodecManager : public CodecManager<BitmapCodec>
	{
		public:
		
		friend class prcore::Bitmap;

		static BitmapCodecManager manager;
		
		int GetVersion() const;

		private:

		BitmapCodecManager();
		BitmapCodecManager(const BitmapCodecManager&);
		BitmapCodecManager& operator = (const BitmapCodecManager&);
	};


	inline const BitmapCodec* FindBitmapCodec(const String& ext)
	{
		return BitmapCodecManager::manager.FindInterface(ext);
	}

} // namespace prcore


#endif