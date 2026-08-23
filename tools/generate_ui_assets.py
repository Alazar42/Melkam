import os
import math
from PIL import Image, ImageDraw, ImageFilter

os.makedirs("assets/UI", exist_ok=True)

def generate_panel_9slice():
    # 128x128 Panel with 24px corners, transparent exterior, dark glass interior, cyan glowing border
    size = 128
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    margin = 4
    rect = [margin, margin, size - 1 - margin, size - 1 - margin]
    radius = 12

    # Draw dark translucent glass interior
    draw.rounded_rectangle(rect, radius=radius, fill=(18, 24, 38, 230), outline=(40, 140, 235, 255), width=2)

    # Tech Corner Accents (at each corner)
    c_len = 16
    c_col = (80, 200, 255, 255)
    # Top-Left
    draw.line([(margin, margin + c_len), (margin, margin), (margin + c_len, margin)], fill=c_col, width=3)
    # Top-Right
    draw.line([(size - 1 - margin - c_len, margin), (size - 1 - margin, margin), (size - 1 - margin, margin + c_len)], fill=c_col, width=3)
    # Bottom-Left
    draw.line([(margin, size - 1 - margin - c_len), (margin, size - 1 - margin), (margin + c_len, size - 1 - margin)], fill=c_col, width=3)
    # Bottom-Right
    draw.line([(size - 1 - margin - c_len, size - 1 - margin), (size - 1 - margin, size - 1 - margin), (size - 1 - margin, size - 1 - margin - c_len)], fill=c_col, width=3)

    img.save("assets/UI/panel_frame.png", "PNG")
    print("Generated assets/UI/panel_frame.png")

def generate_button_9slice():
    # 96x48 Button with 12px corners, sleek metallic gradient, cyan border
    w, h = 96, 48
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    margin = 2
    rect = [margin, margin, w - 1 - margin, h - 1 - margin]
    radius = 8

    # Button Base
    draw.rounded_rectangle(rect, radius=radius, fill=(35, 48, 75, 255), outline=(70, 150, 240, 255), width=2)
    # Top highlight line
    draw.line([(margin + radius, margin + 3), (w - 1 - margin - radius, margin + 3)], fill=(100, 180, 255, 180), width=1)

    img.save("assets/UI/button_frame.png", "PNG")
    print("Generated assets/UI/button_frame.png")

def generate_coin_icon():
    # 64x64 Glowing Gold Coin on pure transparent background
    size = 64
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    center = size / 2.0
    r = 26.0

    # Outer Gold Glow
    for i in range(4):
        draw.ellipse([center - r - (3-i), center - r - (3-i), center + r + (3-i), center + r + (3-i)],
                     outline=(255, 190, 40, 40 + i * 40), width=1)

    # Main Coin Rim
    draw.ellipse([center - r, center - r, center + r, center + r],
                 fill=(220, 140, 20, 255), outline=(255, 215, 60, 255), width=3)

    # Inner Coin Face
    inner_r = r - 4.0
    draw.ellipse([center - inner_r, center - inner_r, center + inner_r, center + inner_r],
                 fill=(245, 175, 30, 255), outline=(200, 120, 15, 255), width=2)

    # Embossed 5-Point Star
    star_points = []
    outer_sr = 12.0
    inner_sr = 5.5
    for i in range(10):
        rad = (i * 36 - 90) * math.pi / 180.0
        cur_r = outer_sr if (i % 2 == 0) else inner_sr
        star_points.append((center + cur_r * math.cos(rad), center + cur_r * math.sin(rad)))

    draw.polygon(star_points, fill=(255, 235, 120, 255), outline=(180, 100, 10, 255))

    img.save("assets/UI/coin_icon.png", "PNG")
    print("Generated assets/UI/coin_icon.png")

if __name__ == "__main__":
    generate_panel_9slice()
    generate_button_9slice()
    generate_coin_icon()
