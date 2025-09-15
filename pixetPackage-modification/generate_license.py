#!/usr/bin/env python

import base64
import sys


def hashDjb2(inputStr):
	vHash = 5381
	for ch in inputStr:
		vHash = (vHash << 5) + vHash + ord(ch)
	return vHash & 0xFFFFFFFFFFFFFFFF

def convert8BtoByteArray(val):
	final = bytearray()
	for i in range(0, 8):
		tmp = (val & (0xFF << i * 8)) >> (i * 8)
		final.append(tmp)
	return final
	
def createLicenseFile2(licName, outputFile):
	hsh = hashDjb2(licName)
	hshBytes = convert8BtoByteArray(hsh)
	
	base32 = base64.b32encode(hshBytes).decode('utf-8')
	result = licName + "|" + base32[0:6] + "-" + base32[6:12] + "-" + base32[12] + "XFNVR"
	
	f = open(outputFile, 'w')
	f.write(result)
	f.close()	
	

createLicenseFile2(str(sys.argv[1]), "lic.info")

