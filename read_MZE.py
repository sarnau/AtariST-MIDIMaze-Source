#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import struct
import sys
import datetime

def dump_memory_block(data,size=0,start_data=0):
	if size == 0:
		size = len(data)
	LINE_LENGTH = 16
	for l in range(0,size,LINE_LENGTH):
		if True:
			print('%04x: ' % (start_data+l),end='')
		for p in range(0,LINE_LENGTH):
			offs = start_data + l + p
			if offs < len(data):
				print('%02x ' % data[offs],end='')
			else:
				print('   ',end='')
		if True:
			print(' ',end='')
			for p in range(0,LINE_LENGTH):
				offs = start_data + l + p
				if offs < len(data):
					c = data[start_data + l + p]
					if c < 0x20 or c >= 0x7F:
						c = ord('.')
					print('%c' % c,end='')
				else:
					print(' ',end='')
		print('')


def do_random():
	global randomBase
	#
	# Compute x = (7^5 * x) mod (2^31 - 1)
	# wihout overflowing 31 bits:
	#      (2^31 - 1) = 127773 * (7^5) + 2836
	# From "Random number generators: good ones are hard to find",
	# Park and Miller, Communications of the ACM, vol. 31, no. 10,
	# October 1988, p. 1195.
	#

	# Can't be initialized with 0, so use another value.
	x = randomBase
	if x == 0:
		x = 1
	hi = int(x / 127773)
	lo = int(x % 127773)
	x = 16807 * lo - 2836 * hi
	if x < 0:
		x += 0x7fffffff
	randomBase = x
	return x

def decode_dos_date_time(dos_date_time):
	# Decode DOS Date
	dos_date = dos_date_time >> 16
	day = dos_date & 0b11111
	month = (dos_date >> 5) & 0b1111
	year = ((dos_date >> 9) & 0b1111111) + 1980

	# Decode DOS Time
	dos_time = dos_date_time & 0xFFFF
	second = (dos_time & 0b11111) * 2
	minute = (dos_time >> 5) & 0b111111
	hour = (dos_time >> 11) & 0b11111
	try:
		return datetime.datetime(year, month, day, hour, minute, second)
	except:
		return

def loadMaze(filename):
	global randomBase
	mazeData = bytearray(open(filename,'rb').read())
	
	magic,version,randomBase = struct.unpack('>LHL', mazeData[:10])
	if magic != 0xe44d5a45:
		print("Not an MZE file")
		sys.exit(1)
	if version != 1 and version != 2:
		print("Unknown version of a MZE file %d" % version)
		sys.exit(1)
	
	decryptedMazeData = bytearray()
	for offs in range(10,len(mazeData),2):
		val = struct.unpack('>H', mazeData[offs:offs+2])[0] ^ (do_random() & 0xFFFF)
		decryptedMazeData.append(val >> 8)
		decryptedMazeData.append(val & 0xFF)
	#dump_memory_block(decryptedMazeData)
	mazeSize = struct.unpack('>H', decryptedMazeData[:2])[0]
	print('Maze size = %dx%d' % (mazeSize,mazeSize))
	if mazeSize < 64:
		mazeSize = mazeSize + 1
	offset = 2
	while offset < len(decryptedMazeData):
		segmentType,segmentLength = struct.unpack('>2H', decryptedMazeData[offset:offset+4])
		offset += 4
		segmentData = decryptedMazeData[offset:offset+segmentLength]
		offset += segmentLength
	
		if segmentType == 0:
			print("Maze name = %s" % segmentData.decode('latin-1'))
		elif segmentType == 1:
			print("Maze author = %s" % segmentData.decode('latin-1'))
		elif segmentType == 2: # Maze date (64x64 byte nibbles = 2kb)
			print("Maze data:")
			for y in range(mazeSize):
				for x in range(mazeSize):
					dd = segmentData[x//2 + y * 32]
					if (x & 1) == 0:
						nibble = dd >> 4
					else:
						nibble = dd & 0x0F
					print('0XID456789abcde '[nibble],end='')
				print()
		elif segmentType == 3: # 
			#print("Maze data end marker")
			#dump_memory_block(segmentData) # unused data
			break
		elif segmentType == 4: # creation date and modification date (2x16 bit DOS date/time)
			creationDate,modificationDate = struct.unpack('>LL', segmentData)
			print("Maze creation date = %s" % decode_dos_date_time(creationDate))
			print("Maze modification date = %s" % decode_dos_date_time(modificationDate))
		elif segmentType == 5: # special object animation list
			print("Maze animation data:")
			# we never did an editor for this, but it allows e.g. removing walls after a certain time
			dump_memory_block(segmentData)
		else:
			print(segmentType)
			dump_memory_block(segmentData)

if False:
	for dirpath, dirnames, filenames in os.walk('./MIDIMAZE2/'):
		for filename in filenames:
			if filename.lower().endswith('mze'):
				loadMaze(os.path.join(dirpath, filename))
else:
	loadMaze('./MIDIMAZE2/MIDIMAZE.MZE')
