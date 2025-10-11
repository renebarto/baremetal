//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : DoubleLinkedList.h
//
// Namespace   : baremetal
//
// Class       : DoubleLinkedList
//
// Description : Template for double linked pointer list class
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files(the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and /or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//------------------------------------------------------------------------------

#pragma once

#include "baremetal/Assert.h"

namespace baremetal {

// Magic number for list (PLMC)
#define PTR_LIST_MAGIC 0x504C4D43

/// <summary>
/// Doubly linked list template of pointers
/// </summary>
/// <typeparam name="Pointer"></typeparam>
template <class Pointer> class DoubleLinkedList
{
public:
    /// <summary>
    /// Element in double linked pointer list
    /// </summary>
    struct Element
    {
        /// @brief Magic number to check if element is valid
        unsigned m_magic;
        /// @brief Actual pointer
        Pointer m_ptr;

        /// @brief Pointer to previous element
        Element* m_prev;
        /// @brief Pointer to next element
        Element* m_next;

        explicit Element(Pointer ptr);

        bool CheckMagic() const;
    };

private:
    /// @brief Pointer to first element in list
    DoubleLinkedList<Pointer>::Element* m_head;

public:
    DoubleLinkedList();
    ~DoubleLinkedList();

    DoubleLinkedList<Pointer>::Element* GetFirst();                                                 // Returns nullptr if list is empty
    DoubleLinkedList<Pointer>::Element* GetNext(const DoubleLinkedList<Pointer>::Element* element); // Returns nullptr if nothing follows

    Pointer GetPointer(const DoubleLinkedList<Pointer>::Element* element); // get pointer for element

    void InsertBefore(DoubleLinkedList<Pointer>::Element* before, Pointer pointer); // after must be != nullptr
    void InsertAfter(DoubleLinkedList<Pointer>::Element* after, Pointer pointer);   // before == nullptr to set first element

    void Remove(DoubleLinkedList<Pointer>::Element* element); // remove this element

    DoubleLinkedList<Pointer>::Element* Find(Pointer pointer); // find element using pointer
};

/// <summary>
/// Construct element for pointer
/// </summary>
/// <typeparam name="Pointer"></typeparam>
/// <param name="ptr">ptr Pointer to store in element</param>
template <class Pointer>
DoubleLinkedList<Pointer>::Element::Element(Pointer ptr)
    : m_magic{PTR_LIST_MAGIC}
    , m_ptr{ptr}
    , m_prev{}
    , m_next{}
{
}

/// <summary>
/// Verify magic number
/// </summary>
/// <typeparam name="Pointer"></typeparam>
/// <returns>True if the magic number is correct, false otherwise</returns>
template <class Pointer> bool DoubleLinkedList<Pointer>::Element::CheckMagic() const
{
    return (m_magic == PTR_LIST_MAGIC);
}

/// <summary>
/// Construct a default double linked list
/// </summary>
/// <typeparam name="Pointer"></typeparam>
template <class Pointer>
DoubleLinkedList<Pointer>::DoubleLinkedList()
    : m_head{}
{
}

/// <summary>
/// Destruct a double linked list
/// </summary>
/// <typeparam name="Pointer"></typeparam>
template <class Pointer> DoubleLinkedList<Pointer>::~DoubleLinkedList()
{
    assert(m_head == nullptr);
}

/// <summary>
/// Get the first element in the list
/// </summary>
/// <typeparam name="Pointer"></typeparam>
/// <returns>Pointer to first element in the list, or nullptr if none exists</returns>
template <class Pointer> typename DoubleLinkedList<Pointer>::Element* DoubleLinkedList<Pointer>::GetFirst()
{
    return m_head;
}

/// <summary>
/// Get the next element in the list
/// </summary>
/// <typeparam name="Pointer"></typeparam>
/// <param name="element">Current element</param>
/// <returns>Pointer to next element, or nullptr if none exists</returns>
template <class Pointer> typename DoubleLinkedList<Pointer>::Element* DoubleLinkedList<Pointer>::GetNext(const DoubleLinkedList<Pointer>::Element* element)
{
    assert(element != nullptr);
    assert(element->CheckMagic());

    return element->m_next;
}

/// <summary>
/// Extract pointer from element
/// </summary>
/// <typeparam name="Pointer"></typeparam>
/// <param name="element">Current element</param>
/// <returns>Pointer stored inside element</returns>
template <class Pointer> Pointer DoubleLinkedList<Pointer>::GetPointer(const typename DoubleLinkedList<Pointer>::Element* element)
{
    assert(element != nullptr);
    assert(element->CheckMagic());

    return element->m_ptr;
}

/// <summary>
/// Insert a pointer before a given element
/// </summary>
/// <typeparam name="Pointer"></typeparam>
/// <param name="before">Pointer to element before which to store e new element for the pointer</param>
/// <param name="pointer">Pointer to store in new element</param>
template <class Pointer> void DoubleLinkedList<Pointer>::InsertBefore(typename DoubleLinkedList<Pointer>::Element* before, Pointer pointer)
{
    assert(m_head != nullptr);
    assert(before != nullptr);
    assert(before->CheckMagic());

    Element* element = new Element(pointer);
    assert(element != nullptr);

    if (before == m_head)
    {
        element->m_prev = nullptr;
        element->m_next = before;

        m_head->m_prev = element;

        m_head = element;
    }
    else
    {
        element->m_prev = before->m_prev;
        element->m_next = before;

        if (before->m_prev != nullptr)
        {
            assert(before->m_prev->CheckMagic());
            before->m_prev->m_next = element;
        }

        before->m_prev = element;
    }
}

/// <summary>
/// Insert a pointer after a given element
/// </summary>
/// <typeparam name="Pointer"></typeparam>
/// <param name="after">Pointer to element after which to store e new element for the pointer</param>
/// <param name="pointer">Pointer to store in new element</param>
template <class Pointer> void DoubleLinkedList<Pointer>::InsertAfter(typename DoubleLinkedList<Pointer>::Element* after, Pointer pointer)
{
    Element* element = new Element(pointer);
    assert(element != nullptr);

    if (after == nullptr)
    {
        assert(m_head == nullptr);

        element->m_prev = nullptr;
        element->m_next = nullptr;

        m_head = element;
    }
    else
    {
        assert(m_head != nullptr);
        assert(after->CheckMagic());

        element->m_prev = after;
        element->m_next = after->m_next;

        if (after->m_next != nullptr)
        {
            assert(after->m_next->CheckMagic());
            after->m_next->m_prev = element;
        }

        after->m_next = element;
    }
}

/// <summary>
/// Remove an element
/// </summary>
/// <typeparam name="Pointer"></typeparam>
/// <param name="element">Pointer to element to remove</param>
template <class Pointer> void DoubleLinkedList<Pointer>::Remove(typename DoubleLinkedList<Pointer>::Element* element)
{
    assert(element != nullptr);
    assert(element->CheckMagic());

    if (element == m_head)
    {
        m_head = element->m_next;

        if (element->m_next != nullptr)
        {
            assert(element->m_next->CheckMagic());
            element->m_next->m_prev = nullptr;
        }
    }
    else
    {
        assert(element->m_prev != nullptr);
        assert(element->m_prev->CheckMagic());
        element->m_prev->m_next = element->m_next;

        if (element->m_next != nullptr)
        {
            assert(element->m_next->CheckMagic());
            element->m_next->m_prev = element->m_prev;
        }
    }

#ifndef NDEBUG
    element->m_magic = 0;
#endif
    delete element;
}

/// <summary>
/// Find the element containing a pointer
/// </summary>
/// <typeparam name="Pointer"></typeparam>
/// <param name="pointer">Pointer to search for</param>
/// <returns>Pointer stored inside element</returns>
template <class Pointer> typename DoubleLinkedList<Pointer>::Element* DoubleLinkedList<Pointer>::Find(Pointer pointer)
{
    for (Element* element = m_head; element != nullptr; element = element->m_next)
    {
        assert(element->CheckMagic());

        if (element->m_ptr == pointer)
        {
            return element;
        }
    }

    return nullptr;
}

} // namespace baremetal
