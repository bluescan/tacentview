// tChunk.cpp
//
// The generic chunk based file format for all Tacent projects. This interface can be used for creating and reading
// arbitrary data files. At the bottom of this file are all the global chunk ids. There are 268 million available, so
// just add chunk IDs as necessary. The format supports alignment of data from 4 bytes to 512 bytes in powers of 2.
//
// A chunk consists of:
//		1) A 4 byte specifier. From most-significant bit to least:
//			1  bit:  Is-container boolean.
//			3  bits: Alignment shift.
//			28 bits: Chunk ID.
//		2) A 4 byte unsigned int (big-endian? or plat-dependent?). This is the chunk data size not including any
//			leading alignment padding.
//		3) Zero or more alignment padding bytes.
//		4) The data.
//		5) Any necessary ending alignment pad to achieve 4-byte alignment.
//
// Multiple chunks within a file are consecutive and chunks may contain sub chunks. Regardless of the data alignment
// size, all chunks start at 4-byte aligned addresses. Data alignment requirements may be stricter.
//
// Technically chunks may contain both data and sub-chunks but if you do it this way your chunk reading code will need
// to be smart enough to know where the data and sub-chunks are. By convention we do _not_ do this. A chunk with
// sub-chunks should not contain any other straight data, only other sub-chunks.
//
// A chunk ID of 0x00000000 is considered invalid so don't use it. It is also the ID returned by the uber-chunk
// (tChunkReader object).
//
// To maintain good file compatibility, chunks whose ID are unknown should be ignored and not treated as an error.
//
// If the contents of a chunk need to change you'd better use a new chunk ID. If you really like the name of the chunk
// you can keep it though. Simply rename it (e.g. from tChunkID_NICENAME to tChunkID_OBSOLETE_NICENAME) while keeping
// the ID the same. You can now use the chunk name (tChunkID_NICENAME) elsewhere with a new ID. The obsolete chunk, of
// course, must contain the same stuff as the original nice name chunk.
//
// Copyright (c) 2006, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Foundation/tMemory.h>
#include "System/tFile.h"
#include "System/tChunk.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;


void tChunkWriter::Open(const tString& fileName, tEndianness dstEndianness)
{
	tAssert(!WriteBuffer);

	#ifdef PLATFORM_WIN
	if (ChunkFile)
		throw tChunkError("File already opened.");
	#else
	tAssert(!ChunkFile);
	#endif

	// We need to determine, based on the write endianness and the current endianness, if swaps will be necessary.
	tEndianness srcEndianness = tGetEndianness();
	NeedsEndianSwap = (srcEndianness == dstEndianness) ? false : true;
	IsContainer = true;

	ChunkFile = tOpenFile(fileName, "wb");

	#ifdef PLATFORM_WIN
	if (!ChunkFile)
		throw tChunkError("Cannot open file [%s].", fileName.Pod());
	#else
	tAssert(ChunkFile);
	#endif
}


bool tChunkWriter::OpenSafe(const tString& fileName, tEndianness dstEndianness)
{
	tAssert(!WriteBuffer);

	#ifdef PLATFORM_WIN
	if (ChunkFile)
		return false;
	#else
	tAssert(!ChunkFile);
	#endif

	// We need to determine, based on the write endianness and the current endianness, if swaps will be necessary.
	tEndianness srcEndianness = tGetEndianness();
	NeedsEndianSwap = (srcEndianness == dstEndianness) ? false : true;
	IsContainer = true;

	ChunkFile = tOpenFile(fileName, "wb");

	#ifdef PLATFORM_WIN
	if (!ChunkFile)
		return false;
	#else
	tAssert(ChunkFile);
	#endif

	return true;
}


void tChunkWriter::Close()
{
	tAssert(!WriteBuffer);

	if (ChunkFile)
	{
		tCloseFile(ChunkFile);
		ChunkFile = 0;
	}

	#ifndef PLATFORM_WIN
	tAssert(!ChunkInfos.Head());
	#endif

	if (ChunkInfos.Head())
	{
		// Remove elements, then throw.
		while (ChunkInfo* chunkInfo = ChunkInfos.Remove())
			delete chunkInfo;

		#ifdef PLATFORM_WIN
		throw tChunkError("Some chunks not ended.");
		#endif
	}
}


void tChunkWriter::BeginChunk(uint32 id, int alignment)
{
	// Alignment variable is overridden for container chunks.
	if (id & 0x80000000)
		alignment = 4;

	#ifdef PLATFORM_WIN
	if (!IsContainer)
		throw tChunkError("You can only begin a chunk from a container.");

	if (!ChunkFile && !WriteBuffer)
		throw tChunkError("No chunk file opened and no memory buffer to write to.");

	if
	(
		(alignment > (1 << (int(Alignment::Largest)+2))) ||
		(alignment < (1 << (int(Alignment::Smallest)+2))) ||
		!tIsPower2(alignment)
	)
		throw tChunkError("Chunk alignment value not supported.");
	#else
	tAssert(IsContainer);
	tAssert(ChunkFile || WriteBuffer);
	tAssert
	(
		(alignment <= (1 << (int(Alignment::Largest) + 2))) &&
		(alignment >= (1 << (int(Alignment::Smallest) + 2))) &&
		tIsPower2(alignment)
	);
	#endif

	// Ensure first nibble of id is 0x0 (data) or 0x8 (container).
	tAssert(((id & 0xF0000000) == 0x00000000) || ((id & 0xF0000000) == 0x80000000));
	int alignShift = tLog2(alignment);
	tAssert(alignShift >= 2);
	uint32 idaa = id | ((alignShift-2) << 28);

	IsContainer = true;
	if ((id & 0xF0000000) == 0x00000000)
		IsContainer = false;

	int chunkStart = WriteBuffer ? WriteBufferPos : tSystem::tFileTell(ChunkFile);
	tAssert((chunkStart % 4) == 0);

	if (NeedsEndianSwap)
		tSwapEndian(idaa);

	if (ChunkFile)
	{
		tWriteFile(ChunkFile, &idaa, sizeof(uint32));	// Chunk ID and alignment shift.
		tWriteFile(ChunkFile, &idaa, sizeof(uint32));	// Dummy size.
	}
	else
	{
		tAssert((WriteBufferSize - WriteBufferPos) >= 8);
		tMemcpy(WriteBuffer+WriteBufferPos, &idaa, sizeof(uint32));	WriteBufferPos += 4;	// Chunk ID and alignment shift.
		tMemcpy(WriteBuffer+WriteBufferPos, &idaa, sizeof(uint32));	WriteBufferPos += 4;	// Dummy size
	}

	// We need to advance the position until we meet the alignment requirements for this data.
	int dataStart = WriteBuffer ? WriteBufferPos : tSystem::tFileTell(ChunkFile);
	int numBytesPad = (dataStart % alignment) ? (alignment - (dataStart % alignment)) : 0;
	dataStart += numBytesPad;
	if (ChunkFile)
	{
		uint32 zero = 0;
		while (numBytesPad > 3)
		{
			tWriteFile(ChunkFile, &zero, 4);
			numBytesPad -= 4;
		}

		while (numBytesPad)
		{
			tWriteFile(ChunkFile, &zero, 1);
			numBytesPad--;
		}
	}
	else
	{
		tAssert((WriteBufferSize - WriteBufferPos) >= numBytesPad);
		for (int i = 0; i < numBytesPad; i++)
			*(WriteBuffer + WriteBufferPos + i) = 0;

		WriteBufferPos += numBytesPad;
	}

	ChunkInfo* chunkInfo = new ChunkInfo(chunkStart, dataStart);
	ChunkInfos.Insert(chunkInfo);
}


void tChunkWriter::EndChunk()
{
	ChunkInfo* topChunk = ChunkInfos.Remove();

	#ifdef PLATFORM_WIN
	if (!ChunkFile && !WriteBuffer)
		throw tChunkError("No chunk file opened and no chunk buffer on memory.");

	if (!topChunk)
	{
		tCloseFile(ChunkFile);
		throw tChunkError("Chunk ended but not started.");
	}
	#else
	tAssert(ChunkFile || WriteBuffer);
	tAssert(topChunk);
	#endif

	// We need to write the data size at the beginning of the chunk.
	int currPos = ChunkFile ? tSystem::tFileTell(ChunkFile) : WriteBufferPos;
	tAssert(topChunk->StartData > topChunk->StartChunk);
	uint32 dataSize = currPos - topChunk->StartData;
	if (NeedsEndianSwap)
		tSwapEndian(dataSize);

	if (ChunkFile)
	{
		tFileSeek(ChunkFile, topChunk->StartChunk + 4, tSeekOrigin::Beginning);
		tWriteFile(ChunkFile, &dataSize, sizeof(uint32));
		tFileSeek(ChunkFile, currPos, tSeekOrigin::Beginning);
	}
	else
	{
		tMemcpy((WriteBuffer + topChunk->StartChunk + 4), &dataSize, sizeof(uint32));
	}

	delete topChunk;

	// We need to advance the position until we aligned to 4 bytes so that the next chunk starts at a mult of 4 bytes.
	int numBytesPad = (currPos % 4) ? (4 - (currPos % 4)) : 0;
	if (ChunkFile)
	{
		uint8 zero = 0;
		for (int p = 0; p < numBytesPad; p++)
			tWriteFile(ChunkFile, &zero, 1);
	}
	else
	{
		tAssert((WriteBufferSize - WriteBufferPos) >= numBytesPad);
		for (int p = 0; p < numBytesPad; p++)
			*(WriteBuffer + WriteBufferPos + p) = 0;

		WriteBufferPos += numBytesPad;
	}

	IsContainer = true;
}


int tChunkWriter::Write(const void* data, int sizeInBytes)
{
	#ifdef PLATFORM_WIN
	if (!ChunkFile && !WriteBuffer)
		throw tChunkError("No chunk file opened and no chunk buffer on memory.");

	if (IsContainer)
		throw tChunkError("Not allowed to write data into a container chunk.");
	#else
	tAssert(ChunkFile || WriteBuffer);
	tAssert(!IsContainer);
	#endif

	int numWritten = 0;
	if (ChunkFile)
	{
		numWritten = tWriteFile(ChunkFile, data, sizeInBytes);

		#ifdef PLATFORM_WIN
		if (numWritten != sizeInBytes)
		{
			tCloseFile(ChunkFile);
			throw tChunkError("Could not write to chunk file.");
		}
		#else
		tAssert(numWritten == sizeInBytes);
		#endif
	}
	else
	{
		tAssert((WriteBufferSize - WriteBufferPos) >= sizeInBytes);
		tMemcpy(WriteBuffer+WriteBufferPos, data, sizeInBytes);
		WriteBufferPos += sizeInBytes;
		numWritten = sizeInBytes;
	}

	return numWritten;
}


int tChunkWriter::Write(const tVector2& v)
{
	if (NeedsEndianSwap)
	{
		tVec2 s;
		tSet(s, v);
		for (int i = 0; i < 2; i++)
			tSwapEndian(s.E[i]);
		return Write( (void*)&s, sizeof(s) );
	}

	return Write( (void*)&v, sizeof(v) );
}


int tChunkWriter::Write(const tVector3& v)
{
	if (NeedsEndianSwap)
	{
		tVec3 s;
		tSet(s, v);
		for (int i = 0; i < 3; i++)
			tSwapEndian(s.E[i]);
		return Write( (void*)&s, sizeof(s) );
	}

	return Write( (void*)&v, sizeof(v) );
}


int tChunkWriter::Write(const tVector4& v)
{
	if (NeedsEndianSwap)
	{
		tVec4 s;
		tSet(s, v);
		for (int i = 0; i < 4; i++)
			tSwapEndian(s.E[i]);
		return Write( (void*)&s, sizeof(s) );
	}

	return Write( (void*)&v, sizeof(v) );
}


int tChunkWriter::Write(const tQuaternion& q)
{
	if (NeedsEndianSwap)
	{
		tQuat s;
		tSet(s, q);
		for (int i = 0; i < 4; i++)
			tSwapEndian(s.E[i]);
		return Write( (void*)&s, sizeof(s) );
	}

	return Write( (void*)&q, sizeof(q) );
}


int tChunkWriter::Write(const tMatrix2& m)
{
	if (NeedsEndianSwap)
	{
		tMat2 s;
		tSet(s, m);
		for (int i = 0; i < 4; i++)
			tSwapEndian(s.E[i]);
		return Write( (void*)&s, sizeof(s) );
	}

	return Write( (void*)&m, sizeof(m) );
}


int tChunkWriter::Write(const tMatrix4& m)
{
	if (NeedsEndianSwap)
	{
		tMat4 s;
		tSet(s, m);
		for (int i = 0; i < 16; i++)
			tSwapEndian(s.E[i]);
		return Write( (void*)&s, sizeof(s) );
	}

	return Write( (void*)&m, sizeof(m) );
}


int tChunkWriter::Write(const tEdge& e)
{
	if (NeedsEndianSwap)
	{
		tEdge s(e);
		for (int i = 0; i < 2; i++)
			tSwapEndian(s.Index[i]);
		return Write( (void*)&s, sizeof(s) );
	}

	return Write( (void*)&e, sizeof(e) );
}


int tChunkWriter::Write(const tTri& f)
{
	if (NeedsEndianSwap)
	{
		tTri s(f);
		for (int i = 0; i < 3; i++)
			tSwapEndian(s.Index[i]);
		return Write( (void*)&s, sizeof(s) );
	}

	return Write( (void*)&f, sizeof(f) );
}


int tChunkWriter::Write(const tQuad& f)
{
	if (NeedsEndianSwap)
	{
		tQuad s(f);
		for (int i = 0; i < 4; i++)
			tSwapEndian(s.Index[i]);
		return Write( (void*)&s, sizeof(s) );
	}

	return Write( (void*)&f, sizeof(f) );
}


int tChunkWriter::Write(const tSphere& s)
{
	if (NeedsEndianSwap)
	{
		tSphere ss(s);
		for (int i = 0; i < 3; i++)
			tSwapEndian(ss.Center.E[i]);
		tSwapEndian(ss.Radius);
		return Write( (void*)&ss, sizeof(ss) );
	}

	return Write( (void*)&s, sizeof(s) );
}


int tChunkWriter::Write(const tVector2* data, int numItems)
{
	if (!numItems || !data)
		return 0;

	if (NeedsEndianSwap)
	{
		tVec2* swappedItems = new tVec2[numItems];
		for (int i = 0; i < numItems; i++)
			for (int e = 0; e < 2; e++)
				swappedItems[i].E[e] = tGetSwapEndian(data[i].E[e]);

		int n = Write( (void*)swappedItems, sizeof(tVec2)*numItems );
		delete[] swappedItems;
		return n;
	}

	return Write( (void*)data, sizeof(tVec2)*numItems );
}


int tChunkWriter::Write(const tVector3* data, int numItems)
{
	if (!numItems || !data)
		return 0;

	if (NeedsEndianSwap)
	{
		tVec3* swappedItems = new tVec3[numItems];
		for (int i = 0; i < numItems; i++)
			for (int e = 0; e < 3; e++)
				swappedItems[i].E[e] = tGetSwapEndian(data[i].E[e]);

		int n = Write( (void*)swappedItems, sizeof(tVector3)*numItems );
		delete[] swappedItems;
		return n;
	}

	return Write( (void*)data, sizeof(tVec3)*numItems );
}


int tChunkWriter::Write(const tVector4* data, int numItems)
{
	if (!numItems || !data)
		return 0;

	if (NeedsEndianSwap)
	{
		tVec4* swappedItems = new tVec4[numItems];
		for (int i = 0; i < numItems; i++)
			for (int e = 0; e < 4; e++)
				swappedItems[i].E[e] = tGetSwapEndian(data[i].E[e]);

		int n = Write( (void*)swappedItems, sizeof(tVec4)*numItems );
		delete[] swappedItems;
		return n;
	}

	return Write( (void*)data, sizeof(tVec4)*numItems );
}


int tChunkWriter::Write(const tQuaternion* data, int numItems)
{
	if (!numItems || !data)
		return 0;

	if (NeedsEndianSwap)
	{
		tQuat* swappedItems = new tQuat[numItems];
		for (int i = 0; i < numItems; i++)
			for (int e = 0; e < 4; e++)
				swappedItems[i].E[e] = tGetSwapEndian(data[i].E[e]);

		int n = Write( (void*)swappedItems, sizeof(tQuat)*numItems );
		delete[] swappedItems;
		return n;
	}

	return Write( (void*)data, sizeof(tQuat)*numItems );
}


int tChunkWriter::Write(const tMatrix2* data, int numItems)
{
	if (!numItems || !data)
		return 0;

	if (NeedsEndianSwap)
	{
		tMat2* swappedItems = new tMat2[numItems];
		for (int i = 0; i < numItems; i++)
			for (int e = 0; e < 4; e++)
				swappedItems[i].E[e] = tGetSwapEndian(data[i].E[e]);

		int n = Write( (void*)swappedItems, sizeof(tMat2)*numItems );
		delete[] swappedItems;
		return n;
	}

	return Write( (void*)data, sizeof(tMat2)*numItems );
}


int tChunkWriter::Write(const tMatrix4* data, int numItems)
{
	if (!numItems || !data)
		return 0;

	if (NeedsEndianSwap)
	{
		tMat4* swappedItems = new tMat4[numItems];
		for (int i = 0; i < numItems; i++)
			for (int e = 0; e < 16; e++)
				swappedItems[i].E[e] = tGetSwapEndian(data[i].E[e]);

		int n = Write( (void*)swappedItems, sizeof(tMat4)*numItems );
		delete[] swappedItems;
		return n;
	}

	return Write( (void*)data, sizeof(tMat4)*numItems );
}


int tChunkWriter::Write(const tEdge* data, int numItems)
{
	if (!numItems || !data)
		return 0;

	if (NeedsEndianSwap)
	{
		tEdge* swappedItems = new tEdge[numItems];
		for (int i = 0; i < numItems; i++)
			for (int e = 0; e < 2; e++)
				swappedItems[i].Index[e] = tGetSwapEndian(data[i].Index[e]);

		int n = Write( (void*)swappedItems, sizeof(tEdge)*numItems );
		delete[] swappedItems;
		return n;
	}
		
	return Write( (void*)data, sizeof(tEdge)*numItems );
}


int tChunkWriter::Write(const tTri* data, int numItems)
{
	if (!numItems || !data)
		return 0;

	if (NeedsEndianSwap)
	{
		tTri* swappedItems = new tTri[numItems];
		for (int i = 0; i < numItems; i++)
			for (int e = 0; e < 3; e++)
				swappedItems[i].Index[e] = tGetSwapEndian(data[i].Index[e]);

		int n = Write( (void*)swappedItems, sizeof(tTri)*numItems );
		delete[] swappedItems;
		return n;
	}

	return Write( (void*)data, sizeof(tTri)*numItems );
}


int tChunkWriter::Write(const tQuad* data, int numItems)
{
	if (!numItems || !data)
		return 0;

	if (NeedsEndianSwap)
	{
		tQuad* swappedItems = new tQuad[numItems];
		for (int i = 0; i < numItems; i++)
			for (int e = 0; e < 4; e++)
				swappedItems[i].Index[e] = tGetSwapEndian(data[i].Index[e]);

		int n = Write( (void*)swappedItems, sizeof(tQuad)*numItems );
		delete[] swappedItems;
		return n;
	}

	return Write( (void*)data, sizeof(tQuad)*numItems );
}


int tChunkWriter::Write(const tSphere* data, int numItems)
{
	if (!numItems || !data)
		return 0;

	if (NeedsEndianSwap)
	{
		tSphere* swappedItems = new tSphere[numItems];
		for (int i = 0; i < numItems; i++)
		{
			for (int e = 0; e < 3; e++)
				swappedItems[i].Center.E[e] = tGetSwapEndian(data[i].Center.E[e]);
			swappedItems[i].Radius = tGetSwapEndian(data[i].Radius);
		}

		int n = Write( (void*)swappedItems, sizeof(tSphere)*numItems );
		delete[] swappedItems;
		return n;
	}
		
	return Write( (void*)data, sizeof(tSphere)*numItems );
}


int tChunkWriter::Write(const void* data, const tString& layoutStr)
{
	tString layout = layoutStr;
	if (layout.IsEmpty() || !data)
		return 0;

	layout.Remove(' ');
	layout.Remove('_');
	layout.Remove('-');
	layout.Replace("m4", "v4v4v4v4");
	layout.Replace("m2", "v2v2");
	layout.Replace("v4", "4444");
	layout.Replace("v3", "444");
	layout.Replace("v2", "44");
	layout.Replace("q", "4444");
	layout.Replace("f", "4");
	layout.Replace("d", "8");

	// Is the string wellformed?
	int strLen = 0;
	strLen += layout.CountChar('1');
	strLen += layout.CountChar('2');
	strLen += layout.CountChar('4');
	strLen += layout.CountChar('8');
	if (!strLen || (layout.Length() != strLen))
		return 0;

	// Now lets compute the number of data bytes.
	int numBytes = 0;
	for (int c = 0; c < strLen; c++)
		numBytes += layout[c] - '0';

	if (NeedsEndianSwap)
	{
		uint8* swapped = new uint8[numBytes];
		tMemcpy(swapped, data, numBytes);
		uint8* curr = swapped;

		for (int c = 0; c < strLen; c++)
		{
			switch (layout[c])
			{
				case '1':
					curr += 1;
					break;

				case '2':
				{
					uint16& d = *((uint16*)curr);
					tSwapEndian(d);
					curr += 2;
					break;
				}

				case '4':
				{
					uint32& d = *((uint32*)curr);
					tSwapEndian(d);
					curr += 4;
					break;
				}

				case '8':
				{
					uint64& d = *((uint64*)curr);
					tSwapEndian(d);
					curr += 4;
					break;
				}
			}
		}

		int n = Write((void*)swapped, numBytes);
		delete[] swapped;
		return n;
	}

	return Write((void*)data, numBytes);
}


void tChunkReader::Load(const tString& filename, uint8* buffer)
{
	UnLoad();
	tFileHandle fh = tOpenFile(filename.ConstText(), "rb");
	ReadBufferSize = tGetFileSize(fh);
	const int maxAlign = 1 << (int(tChunkWriter::Alignment::Largest) + 2);

	if (!buffer)
	{
		// Create a buffer big enough for the file. Make sure it is aligned.
		ReadBuffer = (uint8*)tMem::tMalloc(ReadBufferSize, maxAlign);
		IsBufferOwned = true;
	}
	else
	{
		ReadBuffer = buffer;
	}

	// Casting to uint32 is safe even for 64 bit pointers because maxAlign is much smaller than 2^32.
	tAssert((uint32(ReadBuffer) % maxAlign) == 0);
	int numRead = tReadFile(fh, ReadBuffer, ReadBufferSize);
	tAssert(numRead == ReadBufferSize);
	tCloseFile(fh);
}


void tChunkReader::Load(uint8* buffer, int bufferSizeBytes)
{
	UnLoad();
	const int maxAlign = 1 << (int(tChunkWriter::Alignment::Largest) + 2);

	tAssert((uint32(buffer) % maxAlign) == 0);
	IsBufferOwned = false;
	ReadBufferSize = bufferSizeBytes;
	ReadBuffer = buffer;
}


bool tChunkReader::LoadSafe(const tString& filename)
{
	UnLoad();
	tFileHandle fh = tOpenFile(filename.ConstText(), "rb");
	ReadBufferSize = tGetFileSize(fh);
	if (ReadBufferSize == 0)
	{
		tCloseFile(fh);
		return false;
	}
	const int maxAlign = 1 << (int(tChunkWriter::Alignment::Largest) + 2);

	// Create a buffer big enough for the file. Make sure it is aligned.
	ReadBuffer = (uint8*)tMem::tMalloc(ReadBufferSize, maxAlign);
	IsBufferOwned = true;

	tAssert((uint32(ReadBuffer) % maxAlign) == 0);
	int numRead = tReadFile(fh, ReadBuffer, ReadBufferSize);
	tCloseFile(fh);
	if (numRead != ReadBufferSize)
		return false;

	return true;
}


int tChunkReader::Save(const tString& filename)
{
	if (!IsValid())
		return 0;

	tFileHandle chunkFile = tOpenFile(filename, "wb");
	tAssert(chunkFile);

	int numWritten = tWriteFile(chunkFile, ReadBuffer, ReadBufferSize);
	tAssert(numWritten == ReadBufferSize);

	tCloseFile(chunkFile);
	return numWritten;
}


void tChunkReader::UnLoad()
{
	// Unload anything currently maintained.
	if (IsBufferOwned && ReadBuffer)
		tMem::tFree(ReadBuffer);

	ReadBuffer = nullptr;
	ReadBufferSize = 0;
	IsBufferOwned = false;
}
