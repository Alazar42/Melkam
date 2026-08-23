import os
import math
from PIL import Image, ImageDraw, ImageFilter

os.makedirs("assets/sprites", exist_ok=True)

def generate_player_smiley():
    # 64x64 Cute Smiley Box Player
    size = 64
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    margin = 4
    box_rect = [margin, margin, size - 1 - margin, size - 1 - margin]
    radius = 12

    # Main Body: Vibrant Cyan-Teal Rounded Box
    draw.rounded_rectangle(box_rect, radius=radius, fill=(50, 180, 240, 255), outline=(20, 100, 180, 255), width=2)

    # Top highlight gloss
    draw.rounded_rectangle([margin + 2, margin + 2, size - 1 - margin - 2, margin + 14], radius=6, fill=(130, 220, 255, 180))

    # Rosy Cheeks
    draw.ellipse([margin + 6, size - margin - 20, margin + 18, size - margin - 10], fill=(255, 120, 160, 180))
    draw.ellipse([size - margin - 18, size - margin - 20, size - margin - 6, size - margin - 10], fill=(255, 120, 160, 180))

    # Eyes: Left and Right
    # Left eye
    draw.ellipse([20, 22, 28, 34], fill=(15, 30, 50, 255))
    draw.ellipse([22, 24, 25, 27], fill=(255, 255, 255, 255)) # highlight
    draw.ellipse([25, 29, 27, 31], fill=(255, 255, 255, 200)) # micro highlight

    # Right eye
    draw.ellipse([36, 22, 44, 34], fill=(15, 30, 50, 255))
    draw.ellipse([38, 24, 41, 27], fill=(255, 255, 255, 255)) # highlight
    draw.ellipse([41, 29, 43, 31], fill=(255, 255, 255, 200)) # micro highlight

    # Big Happy Mouth (arc)
    draw.arc([24, 26, 40, 42], start=20, end=160, fill=(15, 30, 50, 255), width=3)

    img.save("assets/sprites/player_smiley.png", "PNG")
    print("Generated assets/sprites/player_smiley.png")

def generate_coin():
    # 48x48 Clean Game Coin
    size = 48
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    center = size / 2.0
    r = 20.0

    # Outer Glow Ring
    draw.ellipse([center - r - 2, center - r - 2, center + r + 2, center + r + 2], outline=(255, 200, 50, 80), width=2)

    # Gold Base
    draw.ellipse([center - r, center - r, center + r, center + r], fill=(240, 160, 20, 255), outline=(255, 220, 80, 255), width=2)

    # Inner Recessed Face
    inner_r = r - 4.0
    draw.ellipse([center - inner_r, center - inner_r, center + inner_r, center + inner_r], fill=(255, 195, 40, 255), outline=(210, 130, 15, 255), width=1)

    # Center Star
    star_points = []
    outer_sr = 9.0
    inner_sr = 4.0
    for i in range(10):
        rad = (i * 36 - 90) * math.pi / 180.0
        cur_r = outer_sr if (i % 2 == 0) else inner_sr
        star_points.append((center + cur_r * math.cos(rad), center + cur_r * math.sin(rad)))

    draw.polygon(star_points, fill=(255, 245, 150, 255), outline=(190, 110, 10, 255))

    img.save("assets/sprites/coin.png", "PNG")
    print("Generated assets/sprites/coin.png")

def generate_grass_platform():
    # 128x64 Floating Grass Platform Tile (9-sliceable / stretchable)
    w, h = 128, 64
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    margin_x = 2
    margin_y = 2
    radius = 10

    # Dirt Body
    dirt_rect = [margin_x, margin_y + 10, w - 1 - margin_x, h - 1 - margin_y]
    draw.rounded_rectangle(dirt_rect, radius=radius, fill=(95, 60, 42, 255), outline=(65, 38, 25, 255), width=2)

    # Dirt Rock Details
    draw.ellipse([20, 28, 36, 40], fill=(120, 80, 58, 255))
    draw.ellipse([64, 34, 82, 46], fill=(120, 80, 58, 255))
    draw.ellipse([100, 26, 112, 36], fill=(120, 80, 58, 255))

    # Lush Grass Top Cap (Rounded rectangle + blade tufts)
    grass_rect = [margin_x, margin_y, w - 1 - margin_x, margin_y + 20]
    draw.rounded_rectangle(grass_rect, radius=radius, fill=(76, 175, 55, 255), outline=(48, 120, 32, 255), width=2)

    # Grass Highlight Strip
    draw.rounded_rectangle([margin_x + 4, margin_y + 2, w - 1 - margin_x - 4, margin_y + 8], radius=4, fill=(120, 215, 80, 255))

    # Hanging Grass Tufts
    for x in range(12, w - 12, 14):
        tuft_h = 6 if ((x // 14) % 2 == 0) else 10
        draw.polygon([(x - 4, margin_y + 18), (x + 4, margin_y + 18), (x, margin_y + 18 + tuft_h)], fill=(76, 175, 55, 255))

    img.save("assets/sprites/grass_platform.png", "PNG")
    print("Generated assets/sprites/grass_platform.png")

if __name__ == "__main__":
    generate_player_smiley()
    generate_coin()
    generate_grass_platform()
