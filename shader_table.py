import pefile

pe = pefile.PE("speed.exe")

shader_table_offset = 0x54D9C4

for section in pe.sections:
    if section.Name.startswith(b'.data'):
        data_section = section
        break

data_section_va = data_section.VirtualAddress
data_section_raw = data_section.PointerToRawData

shader_table_rva = shader_table_offset - data_section_raw + data_section_va
shader_table_memory_address = pe.OPTIONAL_HEADER.ImageBase + shader_table_rva

print(f".data Virtual Address: 0x{data_section_va:X}")
print(f".data Raw Offset: 0x{data_section_raw:X}")
print(f"Shader Table RVA: 0x{shader_table_rva:X}")
print(f"Shader Table Memory Address: 0x{shader_table_memory_address:X}")
