Import("env")
import os
import gzip
from SCons.Builder import Builder

def generate_html_header(target, source, env):
    """Convert HTML to header file with gzip compression"""
    html_file = str(source[0])
    header_file = str(target[0])
    
    print(f"Converting and compressing {html_file} to {header_file}")
    
    # Read the HTML content
    with open(html_file, 'r') as f:
        content = f.read()
    
    # Compress with gzip
    compressed_content = gzip.compress(content.encode('utf-8'))
    
    # Write the header file with compressed content as byte array
    with open(header_file, 'w') as f:
        f.write('#pragma once\n\n')
        f.write(f'const size_t INDEX_HTML_SIZE = {len(compressed_content)};\n\n')
        f.write('const uint8_t INDEX_HTML[] PROGMEM = {\n    ')
        
        # Convert binary data to hex representation
        byte_count = 0
        for byte in compressed_content:
            f.write(f"0x{byte:02x}, ")
            byte_count += 1
            if byte_count % 12 == 0:  # Format with 12 bytes per line
                f.write('\n    ')
        
        f.write('\n};\n')
    
    return None

# Get paths from platformio.ini
html_source = env.GetProjectOption("custom_html_source")
html_header = env.GetProjectOption("custom_html_header")

# Replace ${PROJECT_DIR} with actual path
project_dir = env.get("PROJECT_DIR", "")
html_source = html_source.replace("${PROJECT_DIR}", project_dir)
html_header = html_header.replace("${PROJECT_DIR}", project_dir)

# Ensure directories exist
os.makedirs(os.path.dirname(html_header), exist_ok=True)

# Create a builder for HTML to header conversion
html_builder = Builder(
    action=generate_html_header,
    suffix='.h',
    src_suffix='.html'
)

# Add the builder to the environment
env.Append(BUILDERS={'HTMLHeader': html_builder})

# Create build target
html_target = env.HTMLHeader(html_header, html_source)

# Make the compilation depend on the header file
env.Depends('$BUILD_DIR/${PROGNAME}', html_target) 