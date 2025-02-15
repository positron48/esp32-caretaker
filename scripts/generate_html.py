#!/usr/bin/env python3

import sys
import os

def generate_header(html_file, output_file):
    with open(html_file, 'r') as f:
        content = f.read()
    
    with open(output_file, 'w') as f:
        f.write('#pragma once\n\n')
        f.write('const char INDEX_HTML[] PROGMEM = R"rawliteral(\n')
        f.write(content)
        f.write(')rawliteral";\n')

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: generate_html.py <input_html> <output_header>")
        sys.exit(1)
    
    generate_header(sys.argv[1], sys.argv[2]) 