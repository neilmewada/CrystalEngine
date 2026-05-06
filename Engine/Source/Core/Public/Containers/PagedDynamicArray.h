#pragma once

#include "Core.h"

/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * Used under Apache 2.0 license. https://github.com/o3de/o3de/blob/development/LICENSE.TXT
 */

namespace CE
{

    template<typename ValueType>
    class PagedDynamicArrayHandle;

    template<typename T, SIZE_T ElementsPerPage = 512, class Allocator = OSAllocator>
    class PagedDynamicArray
    {
    private:
        //static_assert((ElementsPerPage % 64 == 0) && ElementsPerPage > 0, "ElementsPerPage must be a multiple of 64");
        constexpr static SIZE_T PageSize = ElementsPerPage * sizeof(T);

        using ValueType = T;
        using AllocatorType = Allocator;

        struct Page;

        class iterator;
        class const_iterator;

        friend iterator;
        friend const_iterator;

        friend PagedDynamicArrayHandle<T>;

    public:

        class PageIterator;
        struct IteratorRange
        {
            PageIterator begin;
            PageIterator end;
        };

        using ParallelRanges = Array<IteratorRange>;
        using Handle = PagedDynamicArrayHandle<T>;

        ~PagedDynamicArray();

        Handle Insert(const T& value);

        void Remove(Handle& value);

        SIZE_T GetCount() const { return itemCount; }

        SIZE_T GetPageCount() const { return pageCounter; }

        ParallelRanges GetParallelRanges();

        iterator begin();
        iterator end();

        const_iterator cbegin() const;
        const_iterator cend() const;

    private:

        Page* AddPage();

        AllocatorType allocator{};

        Page* firstPage = nullptr;
        Page* firstAvailablePage = nullptr;

        SIZE_T pageCounter = 0;
        SIZE_T itemCount = 0;
    };

    template<typename T, SIZE_T ElementsPerPage, class Allocator>
    struct PagedDynamicArray<T, ElementsPerPage, Allocator>::Page
    {
        static constexpr SIZE_T InvalidPage = NumericLimits<SIZE_T>::Max();
        static constexpr u64 FullBits = NumericLimits<u64>::Max();

        Page();

        ~Page() = default;

        SIZE_T Reserve();

        void Free(T* item);

        T* GetItem(SIZE_T indexInPage);

        bool IsFull() const;

        bool IsEmpty() const;

        SIZE_T bitStartIndex = 0;
        Page* nextPage = nullptr;
        PagedDynamicArray* container = nullptr;
        SIZE_T pageIndex = 0;
        SIZE_T itemCount = 0;
        BitSet<ElementsPerPage> slots{}; // Bits representing: 1 = occupied ; 0 = free
        AlignedStorage<PageSize, alignof(T)> data;
    };

    template<typename T, SIZE_T ElementsPerPage, class Allocator>
    class PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator
    {
        using this_type = iterator;
        using container_type = PagedDynamicArray;
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;  // or also value_type*
        using reference = T&;  // or also value_type&

        iterator() = default;

        explicit iterator(Page* firstPage);

        reference operator*() const;
        pointer operator->() const;

        bool operator==(const this_type& rhs) const;
        bool operator!=(const this_type& rhs) const;

        this_type& operator++();
        this_type operator++(int);

    protected:

        bool SkipEmptyPages();
        void AdvanceIterator();

        Page* page = nullptr;
        T* item = nullptr;
        SIZE_T bitIndex = 0;
    };

    template<typename T, SIZE_T ElementsPerPage, class Allocator>
    class PagedDynamicArray<T, ElementsPerPage, Allocator>::const_iterator : public PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator
    {
        using this_type = const_iterator;
        using base_type = iterator;

    public:

        using reference = const T&;
        using pointer = const T*;

        const_iterator() = default;
        explicit const_iterator(Page* firstPage);

        reference operator*() const;
        pointer operator->() const;

        this_type& operator++();
        this_type operator++(int);
    };

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    class PagedDynamicArray<T, ElementsPerPage, Allocator>::PageIterator
    {
        using this_type = PageIterator;
        using container_type = PagedDynamicArray;
    public:

        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using reference = T&;
        using pointer = T*;

        PageIterator() = default;
        explicit PageIterator(Page* page);

        reference operator*() const;
        pointer operator->() const;

        bool operator==(const this_type& rhs) const;
        bool operator!=(const this_type& rhs) const;

        this_type& operator++();
        this_type operator++(int);

        SIZE_T GetPageIndex() const;

    private:

        void SkipEmptyBits();
        void SetItemAndAdvanceIterator();

        Page* page = nullptr;
        SIZE_T bitIndex = 0;
        T* item = nullptr;
    };

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    PagedDynamicArray<T, ElementsPerPage, Allocator>::PageIterator::PageIterator(Page* page)
	    : page(page)
    {
        if (page != nullptr)
        {
            SkipEmptyBits();
        }
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::PageIterator::operator*() const -> reference
    {
        return *item;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::PageIterator::operator->() const -> pointer
    {
        return item;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    bool PagedDynamicArray<T, ElementsPerPage, Allocator>::PageIterator::operator==(const this_type& rhs) const
    {
        return rhs.item == item;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    bool PagedDynamicArray<T, ElementsPerPage, Allocator>::PageIterator::operator!=(const this_type& rhs) const
    {
        return !operator==(rhs);
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::PageIterator::operator++() -> this_type&
    {
        SkipEmptyBits();

        return *this;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::PageIterator::operator++(int) -> this_type
    {
        this_type temp = *this;
        this->operator++();
        return temp;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    SIZE_T PagedDynamicArray<T, ElementsPerPage, Allocator>::PageIterator::GetPageIndex() const
    {
        return page->pageIndex;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    void PagedDynamicArray<T, ElementsPerPage, Allocator>::PageIterator::SkipEmptyBits()
    {
        for (; bitIndex < ElementsPerPage && !page->slots.Test(bitIndex); bitIndex++)
        {}

        if (bitIndex >= ElementsPerPage) // Done with this page, so it is end of page iterator.
        {
            item = nullptr;
            return;
        }

        SetItemAndAdvanceIterator();
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    void PagedDynamicArray<T, ElementsPerPage, Allocator>::PageIterator::SetItemAndAdvanceIterator()
    {
        item = page->GetItem(bitIndex);
        bitIndex++;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    PagedDynamicArrayHandle<T> PagedDynamicArray<T, ElementsPerPage, Allocator>::Insert(const T& value)
    {
        while (firstAvailablePage)
        {
            SIZE_T indexInPage = firstAvailablePage->Reserve();
            if (indexInPage != Page::InvalidPage)
            {
                T* item = firstAvailablePage->GetItem(indexInPage);
                new (item) T(value);
                //*item = value;
                itemCount++;
                return Handle(item, firstAvailablePage);
            }
            if (!firstAvailablePage->nextPage)
            {
                // No new pages available
                break;
            }

            firstAvailablePage = firstAvailablePage->nextPage;
        }

        Page* page = AddPage();

        if (firstAvailablePage)
        {
            firstAvailablePage->nextPage = page;
        }
        else
        {
            firstPage = page;
        }

        firstAvailablePage = page;

        SIZE_T indexInPage = page->Reserve();
        T* item = firstAvailablePage->GetItem(indexInPage);
        new (item) T(value);
        //*item = value;
        itemCount++;
        return Handle(item, firstAvailablePage);
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    PagedDynamicArray<T, ElementsPerPage, Allocator>::~PagedDynamicArray()
    {
        Page* page = firstPage;

        while (page != nullptr)
        {
            Page* next = page->nextPage;
            allocator.deallocate(page, sizeof(Page), alignof(Page));
            page = next;
        }

        firstPage = firstAvailablePage = nullptr;
        pageCounter = itemCount = 0;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::GetParallelRanges() -> ParallelRanges
    {
        ParallelRanges ranges{};
        Page* page = firstPage;

        while (page)
        {
            if (!page->IsEmpty())
            {
                ranges.Add({ PageIterator(page), PageIterator(nullptr) });
            }
            page = page->nextPage;
        }

        return ranges;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::begin() -> iterator
    {
        return iterator(firstPage);
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::end() -> iterator
    {
        return iterator();
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::cbegin() const -> const_iterator
    {
        return const_iterator(firstPage);
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::cend() const -> const_iterator
    {
        return const_iterator();
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    typename PagedDynamicArray<T, ElementsPerPage, Allocator>::Page*
        PagedDynamicArray<T, ElementsPerPage, Allocator>::AddPage()
    {
        void* pageBlock = (Page*)allocator.allocate(sizeof(Page), alignof(Page));
        Page* page = new (pageBlock) Page();
        page->pageIndex = pageCounter++;
        page->container = this;
        return page;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    PagedDynamicArray<T, ElementsPerPage, Allocator>::Page::Page()
    {
        slots.Reset();
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    SIZE_T PagedDynamicArray<T, ElementsPerPage, Allocator>::Page::Reserve()
    {
        for (; bitStartIndex < ElementsPerPage; bitStartIndex++)
        {
	        if (!slots.Test(bitStartIndex)) // false = free ; true = occupied
	        {
                slots.Set(bitStartIndex, true);
                itemCount++;
                return bitStartIndex;
	        }
        }

        return InvalidPage;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    void PagedDynamicArray<T, ElementsPerPage, Allocator>::Page::Free(T* item)
    {
        SIZE_T index = item - reinterpret_cast<T*>(&data);
        slots.Set(index, false);
        bitStartIndex = index;
        itemCount--;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    T* PagedDynamicArray<T, ElementsPerPage, Allocator>::Page::GetItem(SIZE_T indexInPage)
    {
        return reinterpret_cast<T*>(&data) + indexInPage;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    bool PagedDynamicArray<T, ElementsPerPage, Allocator>::Page::IsFull() const
    {
        return itemCount == ElementsPerPage;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    bool PagedDynamicArray<T, ElementsPerPage, Allocator>::Page::IsEmpty() const
    {
        return itemCount == 0;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator::iterator(Page* firstPage)
	    : page(firstPage)
    {
        // SkipEmptyPages() will move the iterator past any empty pages at the beginning of the list of pages
        // and return false if it runs out of pages and they're all empty. If this happens, then don't alter
        // anything in the iterator so it's equivalent to the .end() iterator.
        if (SkipEmptyPages())
        {
            // Set up the item pointer and increments the bits.
            AdvanceIterator();
        }
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    typename PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator::reference
		PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator::operator*() const
    {
        return *item;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    typename PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator::pointer
		PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator::operator->() const
    {
        return item;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    bool PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator::operator==(const this_type& rhs) const
    {
        return rhs.item == item;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    bool PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator::operator!=(const this_type& rhs) const
    {
        return !operator==(rhs);
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator::operator++() -> this_type&
    {
        // skip the next bit in the page until one is found with entries
        for (; bitIndex < ElementsPerPage && !page->slots.Test(bitIndex); bitIndex++)
        {}

        if (bitIndex == ElementsPerPage) // Done with this page, move to next page
        {
            bitIndex = 0;
            page = page->nextPage;

            if (!SkipEmptyPages())
            {
                // If SkipEmptyPages() returns false, it means it reached the last page without finding anything.
                // At this point the iterator is in its end state, so just return;
                return *this;
            }
        }

        AdvanceIterator();

        return *this;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator::operator++(int) -> this_type
    {
        this_type temp = *this;
        this->operator++();
        return temp;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    bool PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator::SkipEmptyPages()
    {
        // skip all initial empty pages.
        while (page && page->IsEmpty())
        {
            page = page->nextPage;
        }

        // If the page is null, it's at the end. This sets m_item to nullptr so that it == PagedDynamicArray::End().
        if (page == nullptr)
        {
            item = nullptr;
            return false;
        }

        // skip the empty bitfields in the page
        for(; !page->slots.Test(bitIndex); bitIndex++)
        {
	        
        }

        return true;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    void PagedDynamicArray<T, ElementsPerPage, Allocator>::iterator::AdvanceIterator()
    {
        item = page->GetItem(bitIndex);
        bitIndex++;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    PagedDynamicArray<T, ElementsPerPage, Allocator>::const_iterator::const_iterator(Page* firstPage)
	    : base_type(firstPage)
    {
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::const_iterator::operator*() const -> reference
    {
        return *base_type::item;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::const_iterator::operator->() const -> pointer
    {
        return base_type::item;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::const_iterator::operator++() -> this_type&
    {
        base_type::operator++();
        return *this;
    }

    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    auto PagedDynamicArray<T, ElementsPerPage, Allocator>::const_iterator::operator++(int) -> this_type
    {
        this_type temp = *this;
        this->operator++();
        return temp;
    }

    template<typename ValueType>
    class PagedDynamicArrayHandle
    {
    public:

        PagedDynamicArrayHandle() = default;

        ~PagedDynamicArrayHandle();

        /// Move Constructor
        PagedDynamicArrayHandle(PagedDynamicArrayHandle&& other);

        /// Move Assignment
        PagedDynamicArrayHandle& operator=(PagedDynamicArrayHandle&& other);

        bool IsValid() const { return data != nullptr; }

        bool IsNull() const { return data == nullptr; }

        ValueType* GetData() const { return data; }

        ValueType& operator*() const;
        ValueType* operator->() const;

        void Free();

    private:

        void Invalidate();

        template<typename PageType>
        PagedDynamicArrayHandle(ValueType* data, PageType* page);

        PagedDynamicArrayHandle(const PagedDynamicArrayHandle&) = delete;

        template<typename T2, SIZE_T ElementsPerPage, class Allocator>
        friend class PagedDynamicArray;

        using Destructor = void(*)(void*);

        Destructor destructor = nullptr;
        void* page = nullptr;
        ValueType* data = nullptr;
    };



    template <typename T, SIZE_T ElementsPerPage, class Allocator>
    void PagedDynamicArray<T, ElementsPerPage, Allocator>::Remove(Handle& handle)
    {
        if (handle.IsValid())
        {
            Page* page = static_cast<Page*>(handle.page);
            if (page->pageIndex < firstAvailablePage->pageIndex)
            {
                firstAvailablePage = page;
            }

            handle.data->~T();
            page->Free(handle.data);
            handle.Invalidate();
            itemCount--;
        }
    }

    template <typename ValueType>
    PagedDynamicArrayHandle<ValueType>::~PagedDynamicArrayHandle()
    {
        Free();
    }

    template <typename ValueType>
    PagedDynamicArrayHandle<ValueType>::PagedDynamicArrayHandle(PagedDynamicArrayHandle&& other)
    {
        *this = MoveTemp(other);
    }

    template <typename ValueType>
    PagedDynamicArrayHandle<ValueType>& PagedDynamicArrayHandle<ValueType>::operator=(PagedDynamicArrayHandle&& other)
    {
        if (this != static_cast<void*>(&other))
        {
            Free();
            data = other.data;
            destructor = other.destructor;
            page = other.page;
            other.Invalidate();
        }
        return *this;
    }

    template <typename ValueType>
    ValueType& PagedDynamicArrayHandle<ValueType>::operator*() const
    {
        return *data;
    }

    template <typename ValueType>
    ValueType* PagedDynamicArrayHandle<ValueType>::operator->() const
    {
        return data;
    }

    template <typename ValueType>
    void PagedDynamicArrayHandle<ValueType>::Free()
    {
        if (IsValid())
        {
            destructor(this);
        }
    }

    template <typename ValueType>
    void PagedDynamicArrayHandle<ValueType>::Invalidate()
    {
        page = nullptr;
        data = nullptr;
        destructor = nullptr;
    }

    template <typename ValueType>
    template <typename PageType>
    PagedDynamicArrayHandle<ValueType>::PagedDynamicArrayHandle(ValueType* data, PageType* page)
	    : data(data), page(page)
    {
        destructor = [](void* handlePointer)
            {
                PagedDynamicArrayHandle* handle = static_cast<PagedDynamicArrayHandle*>(handlePointer);
                static_cast<PageType*>(handle->page)->container->Remove(*handle);
            };
    }
} // namespace CE
