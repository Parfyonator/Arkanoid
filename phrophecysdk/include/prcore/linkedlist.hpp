/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRCORE_LINKEDLIST_HPP
#define PRCORE_LINKEDLIST_HPP


#include <cassert>
#include <cstddef>


namespace prcore
{

	template <typename T>
	class LinkedList
	{
		public:

		struct Iterator
		{
			Iterator*	prev;
			Iterator*	next;
			T			data;

			operator const T& () const
			{
				return data;
			}

			operator T& ()
			{
				return data;
			}

			const T& operator -> () const
			{
				return data;
			}

			T& operator -> ()
			{
				return data;
			}
		};

		LinkedList()
		: msize(0),mphead(NULL),mptail(NULL),
		  mpBlockCache(NULL),mpIteratorCache(NULL)
		{
		}

		LinkedList(const LinkedList& v)
		: msize(0),mphead(NULL),mptail(NULL),
		  mpBlockCache(NULL),mpIteratorCache(NULL)
		{
			*this = v;
		}

		~LinkedList()
		{
			ClearIteratorCache();
		}

		void operator = (const LinkedList& v)
		{
			// clear
			ClearIteratorCache();

			// copy data
			Iterator* node = v.Begin();
			while ( node )
			{
				PushBack(node->data);
				node = node->next;
			}
		}

		bool IsEmpty() const
		{
			return GetSize() == 0;
		}

		int GetSize() const
		{
			return msize;
		}

		Iterator* Begin() const
		{
			return mphead;
		}

		Iterator* End() const
		{
			return mptail;
		}

		T& PushBack(const T& item)
		{
			Iterator* node = PushNodeBack();
			node->data = item;
			return node->data;
		}

		T& PushFront(const T& item)
		{
			Iterator* node = PushNodeFront();
			node->data = item;
			return node->data;
		}

		T& PushBack()
		{
			Iterator* node = PushNodeBack();
			return node->data;
		}

		T& PushFront()
		{
			Iterator* node = PushNodeFront();
			return node->data;
		}

		void InsertBefore(Iterator* i, const T& item)
		{
			assert( i != NULL );
			Iterator* node = GetIterator();
			++msize;

			Iterator* prev = i->prev;
			i->prev = node;

			if ( prev )
				prev->next = node;
			else
				mphead = node;

			node->next = i;
			node->prev = prev;
			node->data = item;
		}

		void InsertAfter(Iterator* i, const T& item)
		{
			assert( i != NULL );
			Iterator* node = GetIterator();
			++msize;

			Iterator* next = i->next;
			i->next = node;

			if ( next )
				next->prev = node;
			else
				mptail = node;

			node->prev = i;
			node->next = next;
			node->data = item;
		}

		void PopBack()
		{
			Iterator* node = mptail;

			if ( node )
			{
				Iterator* prev = node->prev;

				if ( prev )
					prev->next = NULL;
				else
					mphead = NULL;

				mptail = prev;
				ReleaseIterator(node);
				--msize;
			}
		}

		void PopFront()
		{
			Iterator* node = mphead;
			if ( node )
			{
				Iterator* next = node->next;

				if ( next )
					next->prev = NULL;
				else
					mptail = NULL;

				mphead = next;
				ReleaseIterator(node);
				--msize;
			}
		}

		void Remove(Iterator* node)
		{
			assert( node != NULL );
	
			Iterator* next = node->next;
			Iterator* prev = node->prev;

			if ( node == mphead )
				mphead = next;

			if ( node == mptail )
				mptail = node->prev;

			if ( prev )
				prev->next = next;

			if ( next )
				next->prev = prev;

			ReleaseIterator(node);
			--msize;
		}

		void Remove(const T& item)
		{
			Iterator* node = mphead;
			while ( node )
			{
				Iterator* next = node->next;

				if ( node->data == item )
					Remove(node);

				node = next;
			}
		}

		void Erase(Iterator* first, Iterator* last)
		{
			if ( IsEmpty() || !first || !last )
				return;

			if ( first == mphead )
				mphead = last->next;

			if ( last == mptail )
				mptail = first->prev;

			Iterator* node = first;
			while ( node )
			{
				Iterator* next = node->next;
				Iterator* prev = node->prev;

				if ( prev )
					prev->next = next;

				if ( next )
					next->prev = prev;

				ReleaseIterator(node);
				--msize;
		
				if ( node==last )
					break;
			
				node = next;
			}
		}

		void Clear()
		{
			ClearIteratorCache();
		}

		private:

		int			msize;
		Iterator*	mphead;
		Iterator*	mptail;

		// iterator block manager

		struct IteratorBlock
		{
			enum { blocksize = 16 };
 
			IteratorBlock*	next;
			Iterator		itb[blocksize];

			IteratorBlock(Iterator*& cache)
			{
				// initialize the iterators
				const int endoff = blocksize - 1;
				for ( int i=0; i<endoff; ++i )
					itb[i].next = itb + i + 1;

				// append the block iterators to the iterator cache
				itb[endoff].next = cache;
				cache = itb;
			}
		};

		IteratorBlock*	mpBlockCache;
		Iterator*		mpIteratorCache;

		Iterator* GetIterator()
		{
			if ( !mpIteratorCache )
			{
				// allocate new block of iterators
				IteratorBlock* block = new IteratorBlock(mpIteratorCache);

				// add the block to release chain
				block->next = mpBlockCache;
				mpBlockCache = block;
			}

			Iterator* node = mpIteratorCache;
			mpIteratorCache = mpIteratorCache->next;

			return node;
		}

		void ReleaseIterator(Iterator* node)
		{
			assert( node != NULL );

			node->next = mpIteratorCache;
			mpIteratorCache = node;
		}

		void ClearIteratorCache()
		{
			IteratorBlock* block = mpBlockCache;
			while ( block )
			{
				IteratorBlock* next = block->next;
				delete block;
				block = next;
			}

			msize = 0;
			mphead = NULL;
			mptail = NULL;
			mpBlockCache = NULL;
			mpIteratorCache = NULL;
		}

		Iterator* PushNodeBack()
		{
			Iterator* node = GetIterator();
			++msize;

			node->prev = mptail;
			node->next = NULL;

			if ( mptail )
				mptail->next = node;
			else
				mphead = node;

			mptail = node;

			return node;
		}

		Iterator* PushNodeFront()
		{
			Iterator* node = GetIterator();
			++msize;

			node->prev = NULL;
			node->next = mphead;

			if ( mphead )
				mphead->prev = node;
			else
				mptail = node;

			mphead = node;

			return node;
		}
	};

} // namespace prcore


#endif