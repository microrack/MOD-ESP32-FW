#!/usr/bin/env python3
"""
ESP32 release build script:
- Combines boot_app0.bin, bootloader.bin, partitions.bin and firmware.bin into a single binary
- Creates an archive with the binary, nvs.csv and address file
"""

import os
import sys
import csv
import shutil
from pathlib import Path
from zipfile import ZipFile

# Standard addresses for ESP32
BOOT_APP0_ADDR = 0xe000
BOOTLOADER_ADDR = 0x1000
PARTITIONS_ADDR = 0x8000

def parse_partitions_csv(csv_path):
    """Parses default.csv and returns a dictionary with partition addresses"""
    partitions = {}
    with open(csv_path, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            if row and not row[0].startswith('#'):
                if len(row) >= 5:
                    name = row[0].strip()
                    offset = row[3].strip()
                    if offset.startswith('0x') or offset.startswith('0X'):
                        partitions[name] = int(offset, 16)
                    else:
                        partitions[name] = int(offset)
    return partitions

def read_binary_file(file_path):
    """Reads a binary file"""
    with open(file_path, 'rb') as f:
        return f.read()

def write_binary_at_offset(output_file, data, offset):
    """Writes data to file at specified offset"""
    output_file.seek(offset)
    output_file.write(data)

def create_combined_binary(prebuilt_dir, firmware_bin_path, output_path):
    """Creates a combined binary from all components"""
    boot_app0_path = os.path.join(prebuilt_dir, 'boot_app0.bin')
    bootloader_path = os.path.join(prebuilt_dir, 'bootloader.bin')
    partitions_path = os.path.join(prebuilt_dir, 'partitions.bin')
    
    # Check if files exist
    if not os.path.exists(boot_app0_path):
        print(f"Warning: {boot_app0_path} not found, skipping")
        boot_app0_path = None
    if not os.path.exists(bootloader_path):
        raise FileNotFoundError(f"Bootloader not found: {bootloader_path}")
    if not os.path.exists(partitions_path):
        raise FileNotFoundError(f"Partitions not found: {partitions_path}")
    if not os.path.exists(firmware_bin_path):
        raise FileNotFoundError(f"Firmware not found: {firmware_bin_path}")
    
    # Read files
    bootloader_data = read_binary_file(bootloader_path)
    partitions_data = read_binary_file(partitions_path)
    firmware_data = read_binary_file(firmware_bin_path)
    
    boot_app0_data = None
    if boot_app0_path:
        boot_app0_data = read_binary_file(boot_app0_path)
    
    # Calculate maximum size
    sizes = [
        BOOTLOADER_ADDR + len(bootloader_data),
        PARTITIONS_ADDR + len(partitions_data),
        len(firmware_data) + 0x10000  # firmware starts at 0x10000
    ]
    if boot_app0_data:
        sizes.append(BOOT_APP0_ADDR + len(boot_app0_data))
    max_size = max(sizes)
    
    # Create output file
    with open(output_path, 'wb') as f:
        # Fill with 0xFF up to maximum size
        f.write(b'\xff' * max_size)
        f.seek(0)
        
        # Write boot_app0.bin (if exists)
        if boot_app0_data:
            write_binary_at_offset(f, boot_app0_data, BOOT_APP0_ADDR)
        
        # Write bootloader.bin
        write_binary_at_offset(f, bootloader_data, BOOTLOADER_ADDR)
        
        # Write partitions.bin
        write_binary_at_offset(f, partitions_data, PARTITIONS_ADDR)
        
        # Write firmware.bin (app0 starts at 0x10000)
        APP0_ADDR = 0x10000
        write_binary_at_offset(f, firmware_data, APP0_ADDR)
    
    print(f"Created combined binary: {output_path}")
    return {
        'boot_app0': BOOT_APP0_ADDR if boot_app0_data else None,
        'bootloader': BOOTLOADER_ADDR,
        'partitions': PARTITIONS_ADDR,
        'app0': APP0_ADDR
    }

def create_addresses_file(partitions, binary_addrs, output_path):
    """Creates a text file with addresses for flashing"""
    with open(output_path, 'w') as f:
        f.write("# ESP32 firmware flash addresses\n")
        f.write("# Format: filename, flash address (hex), flash address (dec)\n\n")
        
        f.write("# ============================================\n")
        f.write("# FIRMWARE FLASHING INSTRUCTIONS\n")
        f.write("# ============================================\n\n")
        
        f.write("combined_firmware.bin -> flash from address 0x00000000 (flash memory start)\n")
        f.write("# This file contains all firmware components:\n")
        if binary_addrs['boot_app0'] is not None:
            f.write(f"#   - boot_app0.bin at address 0x{binary_addrs['boot_app0']:08X}\n")
        f.write(f"#   - bootloader.bin at address 0x{binary_addrs['bootloader']:08X}\n")
        f.write(f"#   - partitions.bin at address 0x{binary_addrs['partitions']:08X}\n")
        f.write(f"#   - firmware.bin (app0) at address 0x{binary_addrs['app0']:08X}\n\n")
        
        f.write("# NVS partition:\n")
        nvs_addr = partitions.get('nvs', 0x9000)
        f.write(f"nvs.csv -> write to nvs partition (address: 0x{nvs_addr:08X}, {nvs_addr})\n")
        f.write("# Note: nvs.csv must be compiled to nvs.bin first using nvs_partition_gen.py\n\n")
        
        f.write("# ============================================\n")
        f.write("# DETAILED PARTITION INFORMATION\n")
        f.write("# ============================================\n\n")
        
        f.write("# Component addresses in combined_firmware.bin:\n")
        if binary_addrs['boot_app0'] is not None:
            f.write(f"boot_app0.bin, 0x{binary_addrs['boot_app0']:08X}, {binary_addrs['boot_app0']}\n")
        f.write(f"bootloader.bin, 0x{binary_addrs['bootloader']:08X}, {binary_addrs['bootloader']}\n")
        f.write(f"partitions.bin, 0x{binary_addrs['partitions']:08X}, {binary_addrs['partitions']}\n")
        f.write(f"firmware.bin (app0), 0x{binary_addrs['app0']:08X}, {binary_addrs['app0']}\n\n")
        
        f.write("# Partitions from default.csv:\n")
        for name, addr in sorted(partitions.items()):
            f.write(f"{name}, 0x{addr:08X}, {addr}\n")

def main():
    # Define paths
    project_dir = Path(__file__).parent.parent
    prebuilt_dir = Path.home() / '.platformio' / 'platforms' / 'urack-esp32' / 'prebuilt'
    default_csv = prebuilt_dir / 'default.csv'
    
    # Path to compiled firmware.bin
    firmware_bin = project_dir / '.pio' / 'build' / 'modesp32v1' / 'firmware.bin'
    
    # Output files
    output_dir = project_dir / '.pio' / 'build' / 'modesp32v1'
    combined_bin = output_dir / 'combined_firmware.bin'
    addresses_txt = output_dir / 'flash_addresses.txt'
    archive_path = output_dir / 'firmware_release.zip'
    
    # Check if required files exist
    if not prebuilt_dir.exists():
        raise FileNotFoundError(f"Prebuilt directory not found: {prebuilt_dir}")
    if not default_csv.exists():
        raise FileNotFoundError(f"default.csv not found: {default_csv}")
    if not firmware_bin.exists():
        raise FileNotFoundError(f"Firmware binary not found: {firmware_bin}")
    
    # Parse partition addresses
    print(f"Reading partitions from: {default_csv}")
    partitions = parse_partitions_csv(default_csv)
    print(f"Found partitions: {list(partitions.keys())}")
    
    # Create combined binary
    print(f"Creating combined binary...")
    binary_addrs = create_combined_binary(prebuilt_dir, firmware_bin, combined_bin)
    
    # Create addresses file
    print(f"Creating addresses file...")
    create_addresses_file(partitions, binary_addrs, addresses_txt)
    
    # Create archive
    print(f"Creating archive...")
    nvs_csv = project_dir / 'nvs.csv'
    if not nvs_csv.exists():
        raise FileNotFoundError(f"nvs.csv not found: {nvs_csv}")
    
    with ZipFile(archive_path, 'w') as zipf:
        zipf.write(combined_bin, 'combined_firmware.bin')
        zipf.write(nvs_csv, 'nvs.csv')
        zipf.write(addresses_txt, 'flash_addresses.txt')
    
    print(f"\n✓ Release archive created: {archive_path}")
    print(f"  - combined_firmware.bin")
    print(f"  - nvs.csv")
    print(f"  - flash_addresses.txt")

if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

