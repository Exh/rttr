/************************************************************************************
*                                                                                   *
*   Copyright (c) 2014, 2015 - 2016 Axel Menzel <info@rttr.org>                     *
*                                                                                   *
*   This file is part of RTTR (Run Time Type Reflection)                            *
*   License: MIT License                                                            *
*                                                                                   *
*   Permission is hereby granted, free of charge, to any person obtaining           *
*   a copy of this software and associated documentation files (the "Software"),    *
*   to deal in the Software without restriction, including without limitation       *
*   the rights to use, copy, modify, merge, publish, distribute, sublicense,        *
*   and/or sell copies of the Software, and to permit persons to whom the           *
*   Software is furnished to do so, subject to the following conditions:            *
*                                                                                   *
*   The above copyright notice and this permission notice shall be included in      *
*   all copies or substantial portions of the Software.                             *
*                                                                                   *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
*   SOFTWARE.                                                                       *
*                                                                                   *
*************************************************************************************/

#ifndef RTTR_TYPE_DATABASE_P_H_
#define RTTR_TYPE_DATABASE_P_H_

#include "rttr/type.h"
#include "rttr/detail/metadata/metadata.h"
#include "rttr/property.h"
#include "rttr/method.h"
#include "rttr/constructor.h"
#include "rttr/destructor.h"
#include "rttr/enumeration.h"
#include "rttr/array_range.h"
#include "rttr/string_view.h"
#include "rttr/filter_item.h"

#include "rttr/detail/misc/flat_map.h"
#include "rttr/detail/misc/flat_multimap.h"
#include "rttr/detail/type/type_data.h"

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

#define RTTR_MAX_TYPE_COUNT 32767
#define RTTR_MAX_INHERIT_TYPES_COUNT 50

#define RTTR_DEFAULT_TYPE_COUNT 4096

namespace rttr
{
namespace detail
{

struct type_comparator_base;

/*!
 * This class holds all type information.
 * It is not part of the rttr API
 */
class RTTR_LOCAL type_database
{
    public:
        type_database(const type_database&) = delete;
        type_database& operator=(const type_database&) = delete;

        static type_database& instance();

        /////////////////////////////////////////////////////////////////////////////////////

        void register_enumeration(const type& t, std::unique_ptr<enumeration_wrapper_base> enum_data);
        void register_metadata( const type& t, std::vector<metadata> data);
        void register_converter(const type& t, std::unique_ptr<type_converter_base> converter);
        void register_comparator(const type& t, const type_comparator_base* comparator);

        /////////////////////////////////////////////////////////////////////////////////////

        const type_converter_base* get_converter(const type& source_type, const type& target_type) const;

        /////////////////////////////////////////////////////////////////////////////////////

        const type_comparator_base* get_comparator(const type& t) const;

        /////////////////////////////////////////////////////////////////////////////////////

        variant get_metadata(const type& t, const variant& key) const;

        /////////////////////////////////////////////////////////////////////////////////////

        enumeration get_enumeration(const type& t) const;

        /////////////////////////////////////////////////////////////////////////////////////

    private:
        type_database();
        ~type_database();

        std::vector<metadata>* get_metadata_list(const type& t) const;
        variant get_metadata(const variant& key, const std::vector<metadata>& data) const;

        using hash_type = std::size_t;
        RTTR_INLINE static hash_type generate_hash(const std::string& text) { return generate_hash(text.c_str()); }
        RTTR_INLINE static hash_type generate_hash(const char* text)
        {
            const std::size_t  magic_prime = static_cast<std::size_t>(0x01000193);
            std::size_t               hash = static_cast<std::size_t>(0xcbf29ce4);

            for (; *text; ++text)
              hash = (hash ^ *text) * magic_prime;

            return hash;
        }

        using rttr_cast_func        = void*(*)(void*);
        using get_derived_info_func = derived_info(*)(void*);

    public:

        template<typename T, typename Data_Type = conditional_t<std::is_pointer<T>::value, T, std::unique_ptr<T>>>
        struct data_container
        {
            data_container(type::type_id id) : m_id(id) {}
            data_container(type::type_id id, Data_Type data) : m_id(id), m_data(std::move(data)) {}
            data_container(data_container<T, Data_Type>&& other) : m_id(other.m_id), m_data(std::move(other.m_data)) {}
            data_container<T, Data_Type>& operator = (data_container<T, Data_Type>&& other)
            {
                m_id = other.m_id;
                m_data = std::move(other.m_data);
                return *this;
            }

            struct order_by_id
            {
                RTTR_INLINE bool operator () ( const data_container<T>& _left, const data_container<T>& _right )  const
                {
                    return _left.m_id < _right.m_id;
                }
                RTTR_INLINE bool operator () ( const type::type_id& _left, const data_container<T>& _right ) const
                {
                    return _left < _right.m_id;
                }
                RTTR_INLINE bool operator () ( const data_container<T>& _left, const type::type_id& _right ) const
                {
                    return _left.m_id < _right;
                }
            };

            type::type_id   m_id;
            Data_Type       m_data;
        };

        friend class type;

        template<typename T>
        static RTTR_INLINE T* get_item_by_type(const type& t, const std::vector<data_container<T>>& vec);
        template<typename T>
        static RTTR_INLINE void register_item_type(const type& t, std::unique_ptr<T> new_item, std::vector<data_container<T>>& vec);

        std::vector<data_container<type_converter_base>>            m_type_converter_list;  //!< This list stores all type conversion objects
        std::vector<data_container<const type_comparator_base*>>    m_type_comparator_list;
        std::vector<data_container<enumeration_wrapper_base>>       m_enumeration_list;
        std::vector<data_container<std::vector<metadata>>>          m_metadata_type_list;
};

} // end namespace detail
} // end namespace rttr

#endif // RTTR_TYPE_DATABASE_P_H_
