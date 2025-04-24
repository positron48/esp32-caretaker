#!/usr/bin/env python

import os
import sys
import codecs

def detect_encoding(raw_data):
    """Simple encoding detection without external dependencies."""
    # Check for UTF-8 BOM
    if raw_data.startswith(b'\xef\xbb\xbf'):
        return 'utf-8-sig'
    # Check for UTF-16 LE BOM
    elif raw_data.startswith(b'\xff\xfe'):
        return 'utf-16-le'
    # Check for UTF-16 BE BOM
    elif raw_data.startswith(b'\xfe\xff'):
        return 'utf-16-be'
    # Check for UTF-32 LE BOM
    elif raw_data.startswith(b'\xff\xfe\x00\x00'):
        return 'utf-32-le'
    # Check for UTF-32 BE BOM
    elif raw_data.startswith(b'\x00\x00\xfe\xff'):
        return 'utf-32-be'
    # Default to UTF-8
    else:
        return 'utf-8'

def parse_env_file(env_path):
    """Parse a .env file and return a dictionary of key-value pairs."""
    if not os.path.exists(env_path):
        print("""
==============================================================
ERROR: .env file not found!

To configure WiFi, create a .env file in the project root
with the following content:

WIFI_SSID=your_wifi_name
WIFI_PASSWORD=your_wifi_password

Replace 'your_wifi_name' and 'your_wifi_password' with your 
actual WiFi credentials.

Alternative method: Uncomment and modify the 'build_flags' line
in platformio.ini to directly specify your WiFi credentials.
==============================================================
""", file=sys.stderr)
        return {}
    
    env_vars = {}
    
    # First read file as binary to detect encoding
    with open(env_path, 'rb') as f:
        raw_data = f.read()
        
    # Detect file encoding without external dependencies
    encoding = detect_encoding(raw_data)
    
    # Try to decode content
    try:
        content = raw_data.decode(encoding, errors='replace')
    except Exception:
        # Try different encodings on error
        for enc in ['utf-8', 'utf-16', 'utf-16-le', 'utf-16-be', 'latin1', 'cp1251']:
            try:
                content = raw_data.decode(enc, errors='replace')
                break
            except Exception:
                continue
    
    # Process lines
    for line in content.splitlines():
        line = line.strip()
        
        if not line or line.startswith('#'):
            continue
            
        if '=' in line:
            key, value = line.split('=', 1)
            clean_key = key.strip()
            # Clean key from BOM or other special characters
            if clean_key.startswith(u'\ufeff'):
                clean_key = clean_key[1:]
            # Clean other non-alphanumeric characters at the beginning
            while clean_key and not clean_key[0].isalnum():
                clean_key = clean_key[1:]
                
            if clean_key:
                env_vars[clean_key] = value.strip()
    
    return env_vars

def main():
    # .env file in current directory
    env_path = '.env'
    
    # Parse the .env file
    env_vars = parse_env_file(env_path)
    
    # Check if required variables exist
    if 'WIFI_SSID' not in env_vars or 'WIFI_PASSWORD' not in env_vars:
        if os.path.exists(env_path):
            print("""
==============================================================
ERROR: Required variables not found in .env file!

The .env file must contain the following variables:
WIFI_SSID=your_wifi_name
WIFI_PASSWORD=your_wifi_password

Check the file format and make sure it's correct.
==============================================================
""", file=sys.stderr)
        
        # Use dummy values to allow build to continue but show error
        print('-DWIFI_SSID=\\"WIFI_NOT_CONFIGURED\\" -DWIFI_PASSWORD=\\"PLEASE_CHECK_ENV_FILE\\"')
        # Return a non-zero exit code
        sys.exit(1)
    
    # Output the build flags
    print(f'-DWIFI_SSID=\\"{env_vars["WIFI_SSID"]}\\" -DWIFI_PASSWORD=\\"{env_vars["WIFI_PASSWORD"]}\\"')

if __name__ == '__main__':
    main() 