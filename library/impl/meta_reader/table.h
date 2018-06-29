
namespace xlang::meta::reader
{
    struct database;

    struct table_base
    {
        explicit table_base(database const& database) noexcept : m_database(database)
        {
        }

        table_base(table_base const&) = delete;
        table_base& operator=(table_base const&) = delete;

        database const& get_database() const noexcept
        {
            return m_database;
        }

        auto size() const noexcept
        {
            return m_row_count;
        }

        template <typename T>
        T get_value(uint32_t const row, uint32_t const column) const
        {
            WINRT_ASSERT(m_columns[column].size);

            if (row > size())
            {
                throw_invalid(u"Invalid row index");
            }

            T result{};
            memcpy(&result, m_data + row * m_row_size + m_columns[column].offset, m_columns[column].size);
            return result;
        }

    private:

        friend database;

        struct column
        {
            uint8_t offset;
            uint8_t size;
        };

        database const& m_database;
        uint8_t const* m_data{};
        uint32_t m_row_count{};
        uint8_t m_row_size{};
        std::array<column, 6> m_columns{};

        void set_row_count(uint32_t const row_count) noexcept
        {
            WINRT_ASSERT(!m_row_count);
            m_row_count = row_count;
        }

        void set_columns(uint8_t const a, uint8_t const b = 0, uint8_t const c = 0, uint8_t const d = 0, uint8_t const e = 0, uint8_t const f = 0) noexcept
        {
            WINRT_ASSERT(a);
            WINRT_ASSERT(a <= 8);
            WINRT_ASSERT(b <= 8);
            WINRT_ASSERT(c <= 8);
            WINRT_ASSERT(d <= 8);
            WINRT_ASSERT(e <= 8);
            WINRT_ASSERT(f <= 8);

            WINRT_ASSERT(!m_row_size);
            m_row_size = a + b + c + d + e + f;
            WINRT_ASSERT(m_row_size < UINT8_MAX);

            m_columns[0] = { 0, a };
            if (b) { m_columns[1] = { static_cast<uint8_t>(a), b }; }
            if (c) { m_columns[2] = { static_cast<uint8_t>(a + b), c }; }
            if (d) { m_columns[3] = { static_cast<uint8_t>(a + b + c), d }; }
            if (e) { m_columns[4] = { static_cast<uint8_t>(a + b + c + d), e }; }
            if (f) { m_columns[5] = { static_cast<uint8_t>(a + b + c + d + e), f }; }
        }

        void set_data(byte_view& view) noexcept
        {
            WINRT_ASSERT(!m_data);

            if (m_row_count)
            {
                WINRT_ASSERT(m_row_size);
                m_data = view.begin();
                view = view.seek(m_row_count * m_row_size);
            }
        }

        uint8_t index_size() const noexcept
        {
            return m_row_count < (1 << 16) ? 2 : 4;
        }
    };

    template <typename T>
    struct index_base
    {
        index_base() noexcept = default;

        index_base(table_base const* const table, T const type, uint32_t const row) noexcept :
            m_table{ table },
            m_value{ ((row + 1) << static_cast<uint32_t>(T::coded_index_bits)) | static_cast<uint32_t>(type) }
        {
        }

        index_base(table_base const* const table, uint32_t const value) noexcept :
            m_table{ table },
            m_value{ value }
        {
        }

        explicit operator bool() const noexcept
        {
            return m_value != 0;
        }

        T type() const noexcept
        {
            return static_cast<T>(m_value & ((1 << static_cast<uint32_t>(T::coded_index_bits)) - 1));
        }

        uint32_t index() const noexcept
        {
            return (m_value >> static_cast<uint32_t>(T::coded_index_bits)) - 1;
        }

        bool operator==(index_base const& other) const noexcept
        {
            return m_value == other.m_value;
        }

        bool operator!=(index_base const& other) const noexcept
        {
            return !(*this == other);
        }

        bool operator<(index_base const& other) const noexcept
        {
            return m_value < other.m_value;
        }

        database const& get_database() const noexcept
        {
            return m_table->get_database();
        }

    protected:

        table_base const* const m_table{};
        uint32_t const m_value{};
    };

    template <typename T>
    struct typed_index : index_base<T>
    {
        using index_base<T>::index_base;
    };

    template <> struct typed_index<MemberRefParent> : index_base<MemberRefParent>
    {
        using index_base<MemberRefParent>::index_base;

        auto TypeRef() const;
    };

    template <typename T>
    struct coded_index : typed_index<T>
    {
        coded_index() noexcept = default;

        coded_index(table_base const* const table, T const type, uint32_t const row) noexcept :
            typed_index<T>{ table, type, row }
        {
        }

        coded_index(table_base const* const table, uint32_t const value) noexcept :
            typed_index<T>{ table, value }
        {
        }
    };

    struct row_base
    {
        row_base(table_base const* const table, uint32_t const index) noexcept : m_table(table), m_index(index)
        {
        }

        uint32_t index() const noexcept
        {
            return m_index;
        }

        template <typename T>
        auto coded_index() const noexcept
        {
            return reader::coded_index{ m_table, T::TypeDef, index() };
        }

        template <typename T>
        T get_value(uint32_t const column) const
        {
            return m_table->get_value<T>(m_index, column);
        }

        bool operator==(row_base const& other) const noexcept
        {
            WINRT_ASSERT(m_table == other.m_table);
            return index() == other.index();
        }

        bool operator!=(row_base const& other) const noexcept
        {
            return !(*this == other);
        }

    protected:

        row_base() noexcept = default;

        std::string_view get_string(uint32_t const column) const;
        byte_view get_blob(uint32_t const column) const;

        template <typename T>
        auto get_coded_index(uint32_t const column) const
        {
            return reader::coded_index<T>{ m_table, m_table->get_value<uint32_t>(m_index, column) };
        }

        database const& get_database() const noexcept
        {
            return m_table->get_database();
        }

        table_base const* get_table() const noexcept
        {
            return m_table;
        }

    private:

        table_base const* const m_table{};
        uint32_t const m_index{};
    };

    template <typename Row>
    struct row_iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Row;
        using difference_type = int32_t;
        using pointer = value_type * ;
        using reference = value_type & ;

        table_base const* table{};
        uint32_t index{};

        row_iterator& operator++() noexcept
        {
            ++index;
            return *this;
        }

        row_iterator operator++(int) noexcept
        {
            row_iterator temp{ *this };
            operator++();
            return temp;
        }

        row_iterator& operator--() noexcept
        {
            --index;
            return *this;
        }

        row_iterator operator--(int) noexcept
        {
            row_iterator temp{ *this };
            operator--();
            return temp;
        }

        row_iterator& operator+=(difference_type offset) noexcept
        {
            index += offset;
            return *this;
        }

        row_iterator operator+(difference_type offset) const noexcept
        {
            row_iterator temp{ *this };
            return temp += offset;
        }

        row_iterator& operator-=(difference_type offset) noexcept
        {
            return *this += -offset;
        }

        row_iterator operator-(difference_type offset) const noexcept
        {
            return *this + -offset;
        }

        difference_type operator-(row_iterator const& other) const noexcept
        {
            WINRT_ASSERT(table == other.table);
            return index - other.index;
        }

        value_type operator[](difference_type offset) const noexcept
        {
            return { table, index + offset };
        }

        bool operator==(row_iterator const& other) const noexcept
        {
            WINRT_ASSERT(table == other.table);
            return index == other.index;
        }

        bool operator!=(row_iterator const& other) const noexcept
        {
            return !(*this == other);
        }

        bool operator<(row_iterator const& other) const noexcept
        {
            WINRT_ASSERT(table == other.table);
            return index < other.index;
        }

        bool operator>(row_iterator const& other) const noexcept
        {
            return other < *this;
        }

        bool operator<=(row_iterator const& other) const noexcept
        {
            return !(other < *this);
        }

        bool operator>=(row_iterator const& other) const noexcept
        {
            return !(*this < other);
        }

        value_type operator*() const noexcept
        {
            return { table, index };
        }
    };

    template <typename Row>
    inline row_iterator<Row> operator+(typename row_iterator<Row>::difference_type n, row_iterator<Row> const& iter) noexcept
    {
        return iter + n;
    }

    template <typename T>
    struct table : table_base
    {
        explicit table(database const* const database) noexcept : table_base{ *database }
        {
        }

        row_iterator<T> begin() const noexcept
        {
            return { this, 0 };
        }

        row_iterator<T> end() const noexcept
        {
            return { this, size() };
        }

        T operator[](uint32_t const row) const noexcept
        {
            return { this, row };
        }
    };
}
