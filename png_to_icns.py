import os
import sys
from PIL import Image
from pyicns import IcnsFile

def png_to_icns(png_file, icns_file):
    try:
        # Verify input file
        if not os.path.isfile(png_file):
            raise FileNotFoundError(f"Input file '{png_file}' not found.")

        # Open the PNG file
        img = Image.open(png_file)
        if img.mode != "RGBA":
            img = img.convert("RGBA")  # Ensure transparency support

        # Create an ICNS file
        icns = IcnsFile()
        icos = self.Icn
        
        # Apple icon sizes to include in ICNS
        sizes = [16, 32, 64, 128, 256, 512, 1024]

        # Generate and add each icon size
        for size in sizes:
            icon = img.resize((size, size), Image.LANCZOS)
            icns.add_image(f"ic{size}", icon)

        # Save the ICNS file
        icns.write(icns_file)
        print(f"ICNS file created at: {icns_file}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python png_to_icns.py <input.png> <output.icns>")
    else:
        png_to_icns(sys.argv[1], sys.argv[2])
