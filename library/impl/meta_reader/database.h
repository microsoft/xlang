
namespace xlang::impl
{
    constexpr uint8_t bits_needed(uint32_t value) noexcept
    {
        --value;
        uint8_t bits{ 1 };

        while (value >>= 1)
        {
            ++bits;
        }

        return bits;
    }

    static_assert(bits_needed(2) == 1);
    static_assert(bits_needed(3) == 2);
    static_assert(bits_needed(4) == 2);
    static_assert(bits_needed(5) == 3);
    static_assert(bits_needed(22) == 5);
}

namespace xlang::meta::reader
{
    struct database : file_view
    {
        database(std::string_view const& path) : file_view{ path }
        {
            auto dos = as<impl::image_dos_header>();

            if (dos.e_magic != 0x5A4D) // IMAGE_DOS_SIGNATURE
            {
                throw_invalid(u"Invalid DOS signature");
            }

            auto pe = as<impl::image_nt_headers32>(dos.e_lfanew);

            if (pe.FileHeader.NumberOfSections == 0 || pe.FileHeader.NumberOfSections > 100)
            {
                throw_invalid(u"Invalid PE section count");
            }

            auto com = pe.OptionalHeader.DataDirectory[14]; // IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR
            auto sections = &as<impl::image_section_header>(dos.e_lfanew + sizeof(impl::image_nt_headers32));
            auto sections_end = sections + pe.FileHeader.NumberOfSections;

            auto section = section_from_rva(sections, sections_end, com.VirtualAddress);

            if (section == sections_end)
            {
                throw_invalid(u"PE section containing CLI header not found");
            }

            auto offset = offset_from_rva(*section, com.VirtualAddress);

            auto cli = as<impl::image_cor20_header>(offset);

            if (cli.cb != sizeof(impl::image_cor20_header))
            {
                throw_invalid(u"Invalid CLI header");
            }

            section = section_from_rva(sections, sections_end, cli.MetaData.VirtualAddress);

            if (section == sections_end)
            {
                throw_invalid(u"PE section containing CLI metadata not found");
            }

            offset = offset_from_rva(*section, cli.MetaData.VirtualAddress);

            if (as<uint32_t>(offset) != 0x424a5342)
            {
                throw_invalid(u"CLI metadata magic signature not found");
            }

            auto version_length = as<uint32_t>(offset + 12);
            auto stream_count = as<uint16_t>(offset + version_length + 18);
            auto view = seek(offset + version_length + 20);
            byte_view tables;

            for (uint16_t i{}; i < stream_count; ++i)
            {
                auto stream = view.as<stream_range>();
                auto name = view.as<std::array<char, 12>>(8);

                if (name.data() == "#Strings"sv)
                {
                    m_strings = sub(offset + stream.offset, stream.size);
                }
                else if (name.data() == "#Blob"sv)
                {
                    m_blobs = sub(offset + stream.offset, stream.size);
                }
                else if (name.data() == "#GUID"sv)
                {
                    m_guids = sub(offset + stream.offset, stream.size);
                }
                else if (name.data() == "#~"sv)
                {
                    tables = sub(offset + stream.offset, stream.size);
                }
                else if (name.data() != "#US"sv)
                {
                    throw_invalid(u"Unknown metadata stream");
                }

                view = view.seek(stream_offset(name.data()));
            }

            auto heap_sizes = tables.as<std::bitset<8>>();
            uint8_t const string_index_size = heap_sizes.test(0) ? 4 : 2;
            uint8_t const guid_index_size = heap_sizes.test(1) ? 4 : 2;
            uint8_t const blob_index_size = heap_sizes.test(2) ? 4 : 2;

            auto valid_bits = tables.as<std::bitset<64>>(8);
            view = tables.seek(24);

            for (uint32_t i{}; i < 64; ++i)
            {
                if (!valid_bits.test(i))
                {
                    continue;
                }

                auto row_count = view.as<uint32_t>();
                view = view.seek(4);

                switch (i)
                {
                case 0x00: Module.set_row_count(row_count); break;
                case 0x01: TypeRef.set_row_count(row_count); break;
                case 0x02: TypeDef.set_row_count(row_count); break;
                case 0x04: Field.set_row_count(row_count); break;
                case 0x06: MethodDef.set_row_count(row_count); break;
                case 0x08: Param.set_row_count(row_count); break;
                case 0x09: InterfaceImpl.set_row_count(row_count); break;
                case 0x0a: MemberRef.set_row_count(row_count); break;
                case 0x0b: Constant.set_row_count(row_count); break;
                case 0x0c: CustomAttribute.set_row_count(row_count); break;
                case 0x0d: FieldMarshal.set_row_count(row_count); break;
                case 0x0e: DeclSecurity.set_row_count(row_count); break;
                case 0x0f: ClassLayout.set_row_count(row_count); break;
                case 0x10: FieldLayout.set_row_count(row_count); break;
                case 0x11: StandAloneSig.set_row_count(row_count); break;
                case 0x12: EventMap.set_row_count(row_count); break;
                case 0x14: Event.set_row_count(row_count); break;
                case 0x15: PropertyMap.set_row_count(row_count); break;
                case 0x17: Property.set_row_count(row_count); break;
                case 0x18: MethodSemantics.set_row_count(row_count); break;
                case 0x19: MethodImpl.set_row_count(row_count); break;
                case 0x1a: ModuleRef.set_row_count(row_count); break;
                case 0x1b: TypeSpec.set_row_count(row_count); break;
                case 0x1c: ImplMap.set_row_count(row_count); break;
                case 0x1d: FieldRVA.set_row_count(row_count); break;
                case 0x20: Assembly.set_row_count(row_count); break;
                case 0x21: AssemblyProcessor.set_row_count(row_count); break;
                case 0x22: AssemblyOS.set_row_count(row_count); break;
                case 0x23: AssemblyRef.set_row_count(row_count); break;
                case 0x24: AssemblyRefProcessor.set_row_count(row_count); break;
                case 0x25: AssemblyRefOS.set_row_count(row_count); break;
                case 0x26: File.set_row_count(row_count); break;
                case 0x27: ExportedType.set_row_count(row_count); break;
                case 0x28: ManifestResource.set_row_count(row_count); break;
                case 0x29: NestedClass.set_row_count(row_count); break;
                case 0x2a: GenericParam.set_row_count(row_count); break;
                case 0x2b: MethodSpec.set_row_count(row_count); break;
                case 0x2c: GenericParamConstraint.set_row_count(row_count); break;
                default: throw_invalid(u"Unknown metadata table");
                };
            }

            auto const TypeDefOrRef = composite_index_size(TypeDef, TypeRef, TypeSpec);
            auto const HasConstant = composite_index_size(Field, Param, Property);
            auto const HasCustomAttribute = composite_index_size(MethodDef, Field, TypeRef, TypeDef, Param, InterfaceImpl, MemberRef, Module, Property, Event, StandAloneSig, ModuleRef, TypeSpec, Assembly, AssemblyRef, File, ExportedType, ManifestResource, GenericParam, GenericParamConstraint, MethodSpec);
            auto const HasFieldMarshal = composite_index_size(Field, Param);
            auto const HasDeclSecurity = composite_index_size(TypeDef, MethodDef, Assembly);
            auto const MemberRefParent = composite_index_size(TypeDef, TypeRef, ModuleRef, MethodDef, TypeSpec);
            auto const HasSemantics = composite_index_size(Event, Property);
            auto const MethodDefOrRef = composite_index_size(MethodDef, MemberRef);
            auto const MemberForwarded = composite_index_size(Field, MethodDef);
            auto const Implementation = composite_index_size(File, AssemblyRef, ExportedType);
            auto const CustomAttributeType = composite_index_size(MethodDef, MemberRef);
            auto const ResolutionScope = composite_index_size(Module, ModuleRef, AssemblyRef, TypeRef);
            auto const TypeOrMethodDef = composite_index_size(TypeDef, MethodDef);

            Assembly.set_columns(4, 8, 4, blob_index_size, string_index_size, string_index_size);
            AssemblyOS.set_columns(4, 4, 4);
            AssemblyProcessor.set_columns(4);
            AssemblyRef.set_columns(8, 4, blob_index_size, string_index_size, string_index_size, blob_index_size);
            AssemblyRefOS.set_columns(4, 4, 4, AssemblyRef.index_size());
            AssemblyRefProcessor.set_columns(4, AssemblyRef.index_size());
            ClassLayout.set_columns(2, 4, TypeDef.index_size());
            Constant.set_columns(2, HasConstant, blob_index_size);
            CustomAttribute.set_columns(HasCustomAttribute, CustomAttributeType, blob_index_size);
            DeclSecurity.set_columns(2, HasDeclSecurity, blob_index_size);
            EventMap.set_columns(TypeDef.index_size(), Event.index_size());
            Event.set_columns(2, string_index_size, TypeDefOrRef);
            ExportedType.set_columns(4, 4, string_index_size, string_index_size, Implementation);
            Field.set_columns(2, string_index_size, blob_index_size);
            FieldLayout.set_columns(4, Field.index_size());
            FieldMarshal.set_columns(HasFieldMarshal, blob_index_size);
            FieldRVA.set_columns(4, Field.index_size());
            File.set_columns(4, string_index_size, blob_index_size);
            GenericParam.set_columns(2, 2, TypeOrMethodDef, string_index_size);
            GenericParamConstraint.set_columns(GenericParam.index_size(), TypeDefOrRef);
            ImplMap.set_columns(2, MemberForwarded, string_index_size, ModuleRef.index_size());
            InterfaceImpl.set_columns(TypeDef.index_size(), TypeDefOrRef);
            ManifestResource.set_columns(4, 4, string_index_size, Implementation);
            MemberRef.set_columns(MemberRefParent, string_index_size, blob_index_size);
            MethodDef.set_columns(4, 2, 2, string_index_size, blob_index_size, Param.index_size());
            MethodImpl.set_columns(TypeDef.index_size(), MethodDefOrRef, MethodDefOrRef);
            MethodSemantics.set_columns(2, MethodDef.index_size(), HasSemantics);
            MethodSpec.set_columns(MethodDefOrRef, blob_index_size);
            Module.set_columns(2, string_index_size, guid_index_size, guid_index_size, guid_index_size);
            ModuleRef.set_columns(string_index_size);
            NestedClass.set_columns(TypeDef.index_size(), TypeDef.index_size());
            Param.set_columns(2, 2, string_index_size);
            Property.set_columns(2, string_index_size, blob_index_size);
            PropertyMap.set_columns(TypeDef.index_size(), Property.index_size());
            StandAloneSig.set_columns(blob_index_size);
            TypeDef.set_columns(4, string_index_size, string_index_size, TypeDefOrRef, Field.index_size(), MethodDef.index_size());
            TypeRef.set_columns(ResolutionScope, string_index_size, string_index_size);
            TypeSpec.set_columns(blob_index_size);

            Module.set_data(view);
            TypeRef.set_data(view);
            TypeDef.set_data(view);
            Field.set_data(view);
            MethodDef.set_data(view);
            Param.set_data(view);
            InterfaceImpl.set_data(view);
            MemberRef.set_data(view);
            Constant.set_data(view);
            CustomAttribute.set_data(view);
            FieldMarshal.set_data(view);
            DeclSecurity.set_data(view);
            ClassLayout.set_data(view);
            FieldLayout.set_data(view);
            StandAloneSig.set_data(view);
            EventMap.set_data(view);
            Event.set_data(view);
            PropertyMap.set_data(view);
            Property.set_data(view);
            MethodSemantics.set_data(view);
            MethodImpl.set_data(view);
            ModuleRef.set_data(view);
            TypeSpec.set_data(view);
            ImplMap.set_data(view);
            FieldRVA.set_data(view);
            Assembly.set_data(view);
            AssemblyProcessor.set_data(view);
            AssemblyOS.set_data(view);
            AssemblyRef.set_data(view);
            AssemblyRefProcessor.set_data(view);
            AssemblyRefOS.set_data(view);
            File.set_data(view);
            ExportedType.set_data(view);
            ManifestResource.set_data(view);
            NestedClass.set_data(view);
            GenericParam.set_data(view);
            MethodSpec.set_data(view);
            GenericParamConstraint.set_data(view);
        }

        table<TypeRef> TypeRef{ this };
        table<GenericParamConstraint> GenericParamConstraint{ this };
        table<TypeSpec> TypeSpec{ this };
        table<TypeDef> TypeDef{ this };
        table<CustomAttribute> CustomAttribute{ this };
        table<MethodDef> MethodDef{ this };
        table<MemberRef> MemberRef{ this };
        table<Module> Module{ this };
        table<Param> Param{ this };
        table<InterfaceImpl> InterfaceImpl{ this };
        table<Constant> Constant{ this };
        table<Field> Field{ this };
        table<FieldMarshal> FieldMarshal{ this };
        table<DeclSecurity> DeclSecurity{ this };
        table<ClassLayout> ClassLayout{ this };
        table<FieldLayout> FieldLayout{ this };
        table<StandAloneSig> StandAloneSig{ this };
        table<EventMap> EventMap{ this };
        table<Event> Event{ this };
        table<PropertyMap> PropertyMap{ this };
        table<Property> Property{ this };
        table<MethodSemantics> MethodSemantics{ this };
        table<MethodImpl> MethodImpl{ this };
        table<ModuleRef> ModuleRef{ this };
        table<ImplMap> ImplMap{ this };
        table<FieldRVA> FieldRVA{ this };
        table<Assembly> Assembly{ this };
        table<AssemblyProcessor> AssemblyProcessor{ this };
        table<AssemblyOS> AssemblyOS{ this };
        table<AssemblyRef> AssemblyRef{ this };
        table<AssemblyRefProcessor> AssemblyRefProcessor{ this };
        table<AssemblyRefOS> AssemblyRefOS{ this };
        table<File> File{ this };
        table<ExportedType> ExportedType{ this };
        table<ManifestResource> ManifestResource{ this };
        table<NestedClass> NestedClass{ this };
        table<GenericParam> GenericParam{ this };
        table<MethodSpec> MethodSpec{ this };

        std::string_view get_string(uint32_t const index) const noexcept
        {
            auto view = m_strings.seek(index);
            auto last = std::find(view.begin(), view.end(), 0);

            if (last == view.end())
            {
                throw_invalid(u"Missing string terminator");
            }

            return { reinterpret_cast<char const*>(view.begin()), static_cast<uint32_t>(last - view.begin()) };
        }

        byte_view get_blob(uint32_t const index) const noexcept
        {
            auto view = m_blobs.seek(index);
            auto initial_byte = view.as<uint8_t>();
            uint32_t blob_size_bytes{};

            switch (initial_byte >> 5)
            {
            case 0:
            case 1:
            case 2:
            case 3:
                blob_size_bytes = 1;
                initial_byte &= 0x7f;
                break;

            case 4:
            case 5:
                blob_size_bytes = 2;
                initial_byte &= 0x3f;
                break;

            case 6:
                blob_size_bytes = 4;
                initial_byte &= 0x1f;
                break;

            default:
                throw_invalid(u"Invalid blob encoding");
            }

            uint32_t blob_size{ initial_byte };

            for (auto&& byte : view.sub(1, blob_size_bytes - 1))
            {
                blob_size = (blob_size << 8) + byte;
            }

            return { view.sub(blob_size_bytes, blob_size) };
        }

    private:

        struct stream_range
        {
            uint32_t offset;
            uint32_t size;
        };

        static bool composite_index_size(uint32_t const row_count, uint8_t const bits)
        {
            return row_count < (1ull << (16 - bits));
        }

        template <typename...Tables>
        static uint8_t composite_index_size(Tables const&... tables)
        {
            return (composite_index_size(tables.size(), impl::bits_needed(sizeof...(tables))) && ...) ? 2 : 4;
        }

        static uint32_t stream_offset(std::string_view const& name) noexcept
        {
            uint32_t padding = 4 - name.size() % 4;

            if (padding == 0)
            {
                padding = 4;
            }

            return static_cast<uint32_t>(8 + name.size() + padding);
        }

        static impl::image_section_header const* section_from_rva(impl::image_section_header const* const first, impl::image_section_header const* const last, uint32_t const rva) noexcept
        {
            return std::find_if(first, last, [rva](auto&& section) noexcept
            {
                return rva >= section.VirtualAddress && rva < section.VirtualAddress + section.Misc.VirtualSize;
            });
        }

        static uint32_t offset_from_rva(impl::image_section_header const& section, uint32_t const rva) noexcept
        {
            return rva - section.VirtualAddress + section.PointerToRawData;
        }

        byte_view m_strings;
        byte_view m_blobs;
        byte_view m_guids;
    };

    inline byte_view row_base::get_blob(uint32_t const column) const
    {
        return get_database().get_blob(m_table->get_value<uint32_t>(m_index, column));
    }

    inline std::string_view row_base::get_string(uint32_t const column) const
    {
        return get_database().get_string(m_table->get_value<uint32_t>(m_index, column));
    }
}
