from PIL import ImageDraw, Image, ImageFont

fonts = ["lucon.ttf", "courbd.ttf", "arialbd.ttf", "BRITANIC.TTF", "BRLNSR.ttf", "BRLNSDB.ttf", "ARLRDBD.ttf", "COPRGTB.ttf", "COPRGTL.ttf", "tahoma.ttf", "tahomabd.ttf", "TCM_____.ttf", "4115417.ttf"]
#fonts = ["lucon.ttf", "courbd.ttf", "arialbd.ttf", "BRITANIC.TTF", "COPRGTL.ttf", "tahoma.ttf", "tahomabd.ttf", "TCM_____.ttf"]
it = 0 
for f in fonts:
    font = ImageFont.truetype(f, 32)
    for c in '0123456789ABCEHIKMOPTX':
        im = Image.new('RGB', (40, 50), 'white')
        draw = ImageDraw.Draw(im)
        draw.text((5, 5), c, (0, 0, 0), font=font)
        im.save('%c_%d.png' % (c, it))
        print '%c\t%c_%d.png' % (c, c, it)
    it += 1

for f in fonts:
    font = ImageFont.truetype(f, 32)
    for c in '0123456789ABCEHIKMOPTX':
        im = Image.new('RGB', (40, 50), 'white')
        draw = ImageDraw.Draw(im)
        draw.text((5, 5), c, (0, 0, 0), font=font)

        im = im.convert('RGBA').rotate(8, expand=1)
        im2 = Image.new('RGBA', im.size, (255,) * 4)
        o = Image.composite(im, im2, im)
        o.convert('RGB').save('%c_%d.png' % (c, it))

        print '%c\t%c_%d.png' % (c, c, it)
    it += 1

for f in fonts:
    font = ImageFont.truetype(f, 32)
    for c in '0123456789ABCEHIKMOPTX':
        im = Image.new('RGB', (40, 50), 'white')
        draw = ImageDraw.Draw(im)
        draw.text((5, 5), c, (0, 0, 0), font=font)

        im = im.convert('RGBA').rotate(-8, expand=1)
        im2 = Image.new('RGBA', im.size, (255,) * 4)
        o = Image.composite(im, im2, im)
        o.convert('RGB').save('%c_%d.png' % (c, it))

        print '%c\t%c_%d.png' % (c, c, it)
    it += 1

for c in '0123456789ABCEHIKMOPTX':
    print '%c\t%c_f1.png' % (c, c)
    print '%c\t%c_f2.png' % (c, c)
