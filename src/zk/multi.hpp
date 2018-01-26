#pragma once

#include <zk/config.hpp>

#include <initializer_list>
#include <iosfwd>
#include <vector>

#include "forwards.hpp"
#include "op.hpp"
#include "results.hpp"

namespace zk
{

/** \addtogroup Client
 *  \{
**/

class multi_op final
{
public:
    using iterator       = std::vector<op>::iterator;
    using const_iterator = std::vector<op>::const_iterator;
    using size_type      = std::vector<op>::size_type;

public:
    multi_op() noexcept
    { }

    multi_op(std::vector<op> ops) noexcept;

    multi_op(std::initializer_list<op> ops) :
            multi_op(std::vector<op>(ops))
    { }

    ~multi_op() noexcept;

    size_type size() const { return _ops.size(); }

    op&       operator[](size_type idx)       { return _ops[idx]; }
    const op& operator[](size_type idx) const { return _ops[idx]; }

    iterator begin()              { return _ops.begin(); }
    const_iterator begin() const  { return _ops.begin(); }
    const_iterator cbegin() const { return _ops.begin(); }

    iterator end()              { return _ops.end(); }
    const_iterator end() const  { return _ops.end(); }
    const_iterator cend() const { return _ops.end(); }

    void reserve(size_type capacity) { _ops.reserve(capacity); }

    template <typename... TArgs>
    void emplace_back(TArgs&&... args)
    {
        _ops.emplace_back(std::forward<TArgs>(args)...);
    }

    void push_back(op&& x)
    {
        emplace_back(std::move(x));
    }

    void push_back(const op& x)
    {
        emplace_back(x);
    }

private:
    std::vector<op> _ops;
};

std::ostream& operator<<(std::ostream&, const multi_op&);

std::string to_string(const multi_op&);

class multi_result final
{
public:
    /** A part of a result. The behavior depends on the \c op_type of \c op provided to the original transaction. **/
    class part final
    {
    public:
        explicit part(op_type, std::nullptr_t) noexcept;
        explicit part(create_result) noexcept;
        explicit part(set_result) noexcept;

        part(const part&);
        part(part&&) noexcept;

        part& operator=(const part&) = delete;
        part& operator=(part&&) = delete;

        ~part() noexcept;

        /** The \c op_type of the \c op that caused this result. **/
        op_type type() const { return _type; }

        const create_result& as_create() const;

        const set_result& as_set() const;

    private:
        using any_result = std::variant<std::monostate, create_result, set_result>;

        template <typename T>
        const T& as(ptr<const char> operation) const;

    private:
        op_type    _type;
        any_result _storage;
    };

    using iterator       = std::vector<part>::iterator;
    using const_iterator = std::vector<part>::const_iterator;
    using size_type      = std::vector<part>::size_type;

public:
    multi_result() noexcept
    { }

    multi_result(std::vector<part> parts) noexcept;

    ~multi_result() noexcept;

    size_type size() const { return _parts.size(); }

    part&       operator[](size_type idx)       { return _parts[idx]; }
    const part& operator[](size_type idx) const { return _parts[idx]; }

    iterator begin()              { return _parts.begin(); }
    const_iterator begin() const  { return _parts.begin(); }
    const_iterator cbegin() const { return _parts.begin(); }

    iterator end()              { return _parts.end(); }
    const_iterator end() const  { return _parts.end(); }
    const_iterator cend() const { return _parts.end(); }

    void reserve(size_type capacity) { _parts.reserve(capacity); }

    template <typename... TArgs>
    void emplace_back(TArgs&&... args)
    {
        _parts.emplace_back(std::forward<TArgs>(args)...);
    }

    void push_back(part&& x)
    {
        emplace_back(std::move(x));
    }

    void push_back(const part& x)
    {
        emplace_back(x);
    }

private:
    std::vector<part> _parts;
};

std::ostream& operator<<(std::ostream&, const multi_result::part&);
std::ostream& operator<<(std::ostream&, const multi_result&);

std::string to_string(const multi_result::part&);
std::string to_string(const multi_result&);

/** \} **/

}
