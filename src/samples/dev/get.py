from PIL import Image, ImageDraw

im = Image.open('letters.png')

c = [('A', 0, 19),
     ('B', 0, 19 + 65 * 2),
     ('E', 0, 5 + 65 * 4),
     ('K', 1, 19),
     ('M', 1, 22 + 65 * 2),
     ('H', 1, 30 + 65 * 3),
     ('O', 1, 32 + 65 * 4),
     ('P', 1, 35 + 65 * 6),
     ('C', 1, 34 + 65 * 7),
     ('T', 2, 13),
     ('X', 2, 12 + 65 * 3),
     ('I', 3, 32 + 65 * 3),
 ]

Y = [20, 116, 115 + 95, 115 + 95 * 2]

for i in c:
    y = Y[i[1]]
    j = im.crop( (i[2], y, i[2] + 60, y + 60) )
    if i[0] == 'M':
        j = j.crop( (5, 3, 55, 57) )
    else:
        j = j.crop( (7, 3, 53, 57) )

    f = min(20.0 / j.size[0], 32.0 / j.size[1])
    print (int(j.size[0] * f), int(j.size[1] * f))
    j = j.resize((int(j.size[0] * f), int(j.size[1] * f)))

    o = Image.new("1", (20, 32), "white")
    o.paste(j, ( (20 - j.size[0]) / 2, (32 - j.size[1]) / 2 ) )
    o.resize((20, 32)).save('%s2.png' % i[0])

