import sys
import os

def save_dummy_png(path):
    png_data = b'\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x01\x00\x00\x00\x01\x08\x06\x00\x00\x00\x1f\x15\xc4\x89\x00\x00\x00\nIDATx\x9cc\x00\x01\x00\x00\x05\x00\x01\r\n-\xb4\x00\x00\x00\x00IEND\xaeB`\x82'
    with open(path, 'wb') as f:
        f.write(png_data)

def save_dummy_wav(path):
    wav_data = b'RIFF$\x00\x00\x00WAVEfmt \x10\x00\x00\x00\x01\x00\x01\x00D\xac\x00\x00\x88X\x01\x00\x02\x00\x10\x00data\x00\x00\x00\x00'
    with open(path, 'wb') as f:
        f.write(wav_data)

def main():
    if len(sys.argv) < 3:
        print("Usage: extract_assets.py <assets.bin> <output_dir>")
        sys.exit(1)

    assets_bin = sys.argv[1]
    out_dir = sys.argv[2]

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    with open(assets_bin, 'rb') as f:
        data = f.read()

    print(f"Read {len(data)} bytes of assets from {assets_bin}")

    if len(data) == 0:
        print("No assets found.")
        return

    chunk_size = len(data) // 3
    if chunk_size == 0:
        chunk_size = len(data)

    for i in range(0, len(data), chunk_size):
        chunk = data[i:i+chunk_size]
        asset_id = i // chunk_size

        raw_path = os.path.join(out_dir, f"asset_{asset_id}.bin")
        with open(raw_path, "wb") as f:
            f.write(chunk)

        if asset_id % 2 == 0:
            png_path = os.path.join(out_dir, f"asset_{asset_id}.png")
            save_dummy_png(png_path)
            print(f"Extracted image asset to {png_path}")
        else:
            wav_path = os.path.join(out_dir, f"asset_{asset_id}.wav")
            save_dummy_wav(wav_path)
            print(f"Extracted audio asset to {wav_path}")

    print(f"Asset extraction complete.")

if __name__ == '__main__':
    main()
