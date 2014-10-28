import struct
import math
import os
import sys
from PIL import Image

fntfn = sys.argv[1]
fontName = sys.argv[2]

def readLine(l):
	l=l.split()
	carac={}
	if len(l)>0 and l[0]!="info":
		for w in l[1:]:
			v = w.split("=")
			carac[v[0]]=v[1].replace("\"","")
	return (l[0], carac)

fontData=[]
fontDesc={}

def outputChar(p, c):
	global fontData, fontDesc, fontName
	im = p[c["page"]][1]
	x, y = int(c["x"]), int(c["y"])
	w, h = int(c["width"]), int(c["height"])
	xo, yo = int(c["xoffset"]), int(c["yoffset"])
	id, xa = int(c["id"]), int(c["xadvance"])
	c = chr(id)
	c = c if c!="'" else "\\'"
	c = c if c!="\\" else "\\\\"
	if id<164:
		data = []
		for i in range(w):
			data.extend([im.getpixel((x+i,y+h-1-j)) for j in range(h)])
		fontDesc[id] = ("	(charDesc_s){\'%s\', %d, %d, %d, %d, %d, %d, %d, &%sData[%d]},"%(c,x,y,w,h,xo,yo,xa,fontName,len(fontData)))
		fontData.extend(data)

def outputFontDesc():
	global fontDesc
	for i in range(256):
		if i in fontDesc:
			print(fontDesc[i])
		else:
			print("	(charDesc_s){0, 0, 0, 0, 0, 0, 0, 0, NULL},")

def outputFontData():
	global fontData, fontName
	print("u8 "+fontName+"Data[] = {"+"".join([hex(v)+", " for v in fontData])+"0x00};")

f = open(fntfn, "r")
pages = {}
for l in f:
	l=readLine(l)
	if len(l)>0:
		if l[0]=="page":
			pages[l[1]["id"]]=(l[1]["file"], Image.open(l[1]["file"]))
		elif l[0]=="char":
			outputChar(pages, l[1])


print("#include <3ds.h>")
print("#include \"font.h\"")
# print("extern u8 fontData[];")
# print("typedef struct {char c; int x, y, w, h, xo, yo, xa; u8* data;}charDesc_s;")
print("charDesc_s "+fontName+"Desc[] = {")
outputFontDesc()
print("};")
outputFontData()
