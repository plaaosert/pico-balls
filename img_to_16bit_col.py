from PIL import Image

out_string = "const uint8_t arducam_logo[25608] = { 0X10,0X10,0X00,0X50,0X00,0XA0,0X01,0X1B,\n"

im = Image.open('tiles.png')
im = im.convert("RGB")
pix = im.load()

num_added = 0
for x in range(160):
    for y in range(80):
        red, green, blue = im.getpixel((x,y))
        if red == 0 and green == 0 and blue == 0:
            rgb565 = 0
        else:
            rgb565 = (((red & 0b11111000) << 8) + ((green & 0b11111100) << 3) + (blue >> 3))

        out_string += "0x{:02x}, 0x{:02x}, ".format((rgb565 & 0xff00) >> 8, rgb565 & 0x00ff)
        num_added += 1
        
        if (((x * 160) + y) + 1) % 8 == 0:
            out_string += "\n"

print(out_string)
