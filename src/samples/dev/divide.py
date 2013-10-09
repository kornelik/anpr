from PIL import Image, ImageDraw

im = Image.open('numbers.png')
off = [-20, 50, 100, 160, 220, 280, 343, 403, 468, 529, 589]
for i in xrange(10):
    j = im.crop( (off[i], 0, off[i + 1], im.size[1]) )
    j = j.crop( (j.size[0] - 50, j.size[1]- 80, j.size[0], j.size[1]) )
    #j = j.resize( (50, 50) )

    f = min(20.0 / j.size[0], 32.0 / j.size[1])
    j = j.resize((int(j.size[0] * f), int(j.size[1] * f)))
    o = Image.new("1", (20, 32), "white")
    o.paste(j, ( (20 - j.size[0]) / 2, (32 - j.size[1]) / 2 ) )
    j.resize((20, 32)).save('%d.png' % ((i + 1) % 10))

