#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import struct
from PIL import Image, ImageDraw

MIDIMAZE_COLOR_PALETTE = [
				0x000, 0x566, 0x455, 0x227,
				0x444, 0x122, 0x344, 0x233,
				0x770, 0x740, 0x403, 0x707,
				0x057, 0x060, 0x700, 0x777 ]

# The Atari ST range for color is from 0-7, PIL requires 0-255
COLORCONV = 255.0/7.0

MM_COLORS = []
for color in MIDIMAZE_COLOR_PALETTE:
	MM_COLORS.append((int(((color >> 8) & 0xf) * COLORCONV),int(((color >> 4) & 0xf) * COLORCONV),int(((color >> 0) & 0xf) * COLORCONV)))

def convert_title(data,planeCount,name):
	imageWordCount,lineDataCount = struct.unpack('>2H', data[:4])
	offsetData = 4
	offsetImageData = offsetData + lineDataCount
	planeBuffer = [] # all bitplanes are compressed in vertical rows
	wc = 0
	for _ in range(lineDataCount >> 1):
		numberOfLines = data[offsetData]
		offsetData += 1
		for _ in range(numberOfLines):
			imageData = (data[offsetImageData] << 8) | data[offsetImageData+1]
			wc += 1
			offsetImageData += 2
			planeBuffer.append(imageData)
		numberOfSkippedLines = data[offsetData]
		offsetData += 1
		for _ in range(numberOfSkippedLines):
			planeBuffer.append(0)
	assert(offsetData == lineDataCount + 4)
	assert(wc == imageWordCount)
	assert(len(planeBuffer) == 16000)

	# re-sort into an atari screenbuffer order
	screenmem = []
	for i in range(16000):
		o = (i % 80) * 200 + int(i / 80)
		screenmem.append(planeBuffer[o])

	if planeCount == 1: # black/white
		WIDTH = 640
		HEIGHT = 400
		img = Image.new('RGB', (WIDTH, HEIGHT))
		draw = ImageDraw.Draw(img)
		for y in range(HEIGHT):
			for xw in range(40):
				pixels = screenmem[y * 40 + xw]
				for x in reversed(range(16)):
					color = 0
					if (pixels >> x) & 1:
						color = 15
					xc = (15 - x) + xw * 16
					draw.point([(xc, y)], MM_COLORS[color])
	elif planeCount == 4: # 16 colors
		WIDTH = 320
		HEIGHT = 200
		img = Image.new('RGB', (WIDTH, HEIGHT))
		draw = ImageDraw.Draw(img)
		for y in range(HEIGHT):
			scro = y * 80
			for xw in range(0,80,4):
				plane0 = screenmem[scro + xw + 0]
				plane1 = screenmem[scro + xw + 1]
				plane2 = screenmem[scro + xw + 2]
				plane3 = screenmem[scro + xw + 3]
				for x in reversed(range(16)):
					color = (((plane3 >> x) & 1) << 3) | (((plane2 >> x) & 1) << 2) | (((plane1 >> x) & 1) << 1) | (((plane0 >> x) & 1) << 0)
					draw.point([(15-x + xw * 16//4, y)], MM_COLORS[color])
	img.save(name)


def print_sine_table(sine_table):
	sines = struct.unpack('>65H', sine_table)
	print(sines)

def print_body_shapes(data,FACE_COUNT):

	BODY_SHAPE_COUNT = 24 # There are 24 different sizes of smily shapes in the image data

	shapeOffset = 0
	for shapeIndex in range(BODY_SHAPE_COUNT):
		for faceIndex in range(FACE_COUNT):
			# scaling factor for the share (32..1) with the shapeIndex 0..23
			shapeScale = BODY_SHAPE_COUNT-shapeIndex
			if shapeScale > 16:
				shapeScale = ((shapeScale-16)<<1)+16
			
			# the height of the shape in lines
			# 24 shapes: 53, 50, 47, 43, 40, 37, 33, 30, 27, 25, 23, 22, 20, 18, 17, 15, 13, 12, 10, 8, 7, 5, 3, 2
			draw_shape_bodyHeight = (shapeScale*40//12+1) // 2
			
			# the width of the shape in 16-bit words
			shapeWidthInWords = (shapeScale-1)//8+1
		
			print('#%2d : %2d %dx%d' % (shapeIndex, shapeScale, draw_shape_bodyHeight, shapeWidthInWords))
			
			shape = data[shapeOffset:shapeOffset + draw_shape_bodyHeight * shapeWidthInWords * 2]
			offs = shapeOffset
			for _ in range(draw_shape_bodyHeight):
				for b in range(shapeWidthInWords * 2):
					print("{:08b}".format(data[offs+b]),end='')
				print()
				offs += shapeWidthInWords * 2
			
			shapeOffset += draw_shape_bodyHeight * shapeWidthInWords * 2

data = bytearray(open('./MIDIMazeDisc/MIDIMAZE.D8A','rb').read())
color_screen = data[0x00000:0x05076]
convert_title(color_screen, 4, 'TITLE_COL.PNG')
bw_screen = data[0x05076:0x05076+0x5A62]
convert_title(bw_screen, 1, 'TITLE_BW.PNG')
sine_table = data[0x0AAD8:0x0AAD8+0x0082]
#print_sine_table(sine_table)
ball_images = data[0x0AB5A:0x0AB5A+0x0C64]
#print_body_shapes(ball_images,1)
face_images = data[0x0B7BE:0x0B7BE+0xF7D0]
#print_body_shapes(face_images,20)
