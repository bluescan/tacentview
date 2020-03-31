// tChunk.h
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

#pragma once
#include <Foundation/tPlatform.h>
#include <Math/tLinearAlgebra.h>
#include <Math/tGeometry.h>
#include <Math/tVector2.h>
#include <Math/tVector3.h>
#include <Math/tVector4.h>
#include <Math/tMatrix2.h>
#include <Math/tMatrix4.h>
#include <Math/tQuaternion.h>
#include <Math/tColour.h>
#include "System/tFile.h"
#ifdef PLATFORM_WINDOWS
#pragma warning (disable: 4311 4369 4302)
#endif


// Use this to write a tChunk file. Chunk data may be guaranteed to be aligned by various amounts from 4 bytes to
// 512 bytes in powers of 2 only.
class tChunkWriter
{
public:
	// Creates the file if it doesn't exist, overwrites it if it does. See the Open() function comment. The endianness
	// is the desired endianness of the written data.
	tChunkWriter(const tString& filename, tEndianness endianness = tEndianness::Little)									: NeedsEndianSwap(false), IsContainer(true), ChunkInfos(), ChunkFile(0), WriteBuffer(nullptr), WriteBufferSize(0), WriteBufferPos(0) { Open(filename, endianness); }

	// Same as above but decides the endianness based on the supplied platform.
	tChunkWriter(const tString& filename, tPlatform platform)															: NeedsEndianSwap(false), IsContainer(true), ChunkInfos(), ChunkFile(0), WriteBuffer(nullptr), WriteBufferSize(0), WriteBufferPos(0) { Open(filename, tGetEndianness(platform)); }

	// Use this if you want this class to write to memory instead of a file. To compute the size of the buffer you'll
	// need, use this to be conservative:
	// numBytesNeeded = (numChunks * 8) + SumOverAllChunks(chunkDataSize + chunkAlignmentSize).
	// Also note that if you want the written data aligned you'll need to supply an aligned dst pointer. Choose a value
	// that is the maximum of your alignment requirements for all chunks you will be writing. Supplying a buffer that
	// is 512 byte aligned is guaranteed to work in all cases.
	tChunkWriter(uint8* dst, int dstBufSize, tEndianness endianness = tEndianness::Little)								: NeedsEndianSwap(false), IsContainer(true), ChunkInfos(), ChunkFile(0), WriteBuffer(dst), WriteBufferSize(dstBufSize), WriteBufferPos(0) { tEndianness srcEndianness = tGetEndianness(); NeedsEndianSwap = (srcEndianness == endianness) ? false : true; }

	// If you want to open the file at a later time. You must open it before calling any other function.
	tChunkWriter()																										: NeedsEndianSwap(false), IsContainer(true), ChunkInfos(), ChunkFile(0), WriteBuffer(nullptr), WriteBufferSize(0), WriteBufferPos(0) { }
	~tChunkWriter()																										{ if (ChunkFile) tSystem::tCloseFile(ChunkFile); while (ChunkInfo* chunkInfo = ChunkInfos.Remove()) delete chunkInfo; }

	// Creates the file if it doesn't exist, overwrites it if it does. This function won't overwrite hidden files.
	// Fixing this problem would slow it down for people who don't use hidden files, so I have opted to document the
	// behaviour instead.
	void Open(const tString& filename, tEndianness = tEndianness::Little);

	// Same as above but bases the write endianness on the platform.
	void Open(const tString& filename, tPlatform platform)																{ Open(filename, tGetEndianness(platform)); }

	// Same as above except returns false at first sign of failure.
	bool OpenSafe(const tString& filename, tPlatform platform)															{ return OpenSafe(filename, tGetEndianness(platform)); }
	bool OpenSafe(const tString& filename, tEndianness = tEndianness::Little);

	// In case you want to open something else. You should have finished writing all the chunks by this time. You can
	// optionally let the destructor call this fn for you.
	void Close();

	// Use this enum to enforce data alignment of written chunks. Alignment of data within chunk data is the user's
	// responsibility. The enum here guarantees the starting address of the data in a data chunk to be aligned to the
	// specified amount. Numbers are in bytes. You can _not_ add additional alignments to this enum. The file format
	// uses a set 3 bits (8 enum entries) to specify the alignment.
	enum class Alignment
	{
		Smallest,
		B4																												= Smallest,
		B8,
		B16,
		B32,
		B64,
		B128,
		B256,
		B512,
		Largest																											= B512
	};

	// Alignment is ignored for container chunks. Containerness is determined by the MS bit of the chunkID.
	void BeginChunk(uint32 chunkID, Alignment a = Alignment::B4)														{ BeginChunk(chunkID, 1 << (int(a)+2)); }

	// Same as above except alignmentInBytes must be 4, 8, 16, 32, 64, 128, 256, or 512.
	void BeginChunk(uint32 chunkID, int alignmentInBytes);
	void EndChunk();

	// All write functions return the number of bytes written. Here's the generic form of Write for various data types.
	// The functions after are specializations for cases that would not work otherwise. The list of types known to
	// work are: char, int8, uint8, int16, uint16, int32, uint32, int64, uint64, float, double, tPixel, tString,
	// tVec2, tVec3, tVec4, tQuat, tMat2, tMat4, tEdge, tTri, tQuad, and tSphere. The reason there are so many
	// specializations is that endian-swapping happens for many of the types, and sometimes to certain types members
	// and not others. The generic case only deals with data that does not need any potential swaps. OK. Forget it.
	// Turns out that the generic form would only save about 3 functions and increase the complexity of the
	// specialized versions. I'm staying with the explicit versions! OK OK. Forget the forget. It's super handy to be
	// able to write enums without first needing to convert them to int first -- so we're going to make the generic
	// version one that handles endian swaps and specialize the others. It turns out that regular old overloads always
	// get chosen before template specializations, so I don't actually need to add any specializations :) The generic
	// form is at the end of the overloads below.
	int Write(bool b)																									{ uint8 v = b ? 1 : 0; return Write( (void*)&v, sizeof(uint8) ); }
	int Write(char c)																									{ return Write( (void*)&c, sizeof(char) ); }
	int Write(int8 c)																									{ return Write( (void*)&c, sizeof(int8) ); }
	int Write(uint8 c)																									{ return Write( (void*)&c, sizeof(uint8) ); }
	int Write(const tColouri& c)																						{ return Write( (void*)&c, sizeof(tColouri) ); }
	int Write(const char* s)																							{ return Write( (void*)s, int(tStd::tStrlen(s)) + 1 ); }
	int Write(const tString& s)								/* Writes the null character as well. */					{ return Write( (const char*)s, s.Length() + 1 ); }
	int Write(const tMath::tVector2&);
	int Write(const tMath::tVector3&);
	int Write(const tMath::tVector4&);
	int Write(const tMath::tQuaternion&);
	int Write(const tMath::tMatrix2&);
	int Write(const tMath::tMatrix4&);
	int Write(const tMath::tEdge&);
	int Write(const tMath::tTri&);
	int Write(const tMath::tQuad&);
	int Write(const tMath::tSphere&);
	template<typename T> int Write(T obj)																				{ if (NeedsEndianSwap) tSwapEndian(obj); return Write( (void*)&obj, sizeof(T) ); }

	// Here are the multiple-object-at-once versions. numItems must be >= 1.
	int Write(const bool* data, int numItems)																			{ for (int i = 0; i < numItems; i++) { Write(*data); data++; } return sizeof(uint8)*numItems; }
	int Write(const char* data, int numItems)																			{ return Write( (void*)data, sizeof(char)*numItems ); }
	int Write(const int8* data, int numItems)																			{ return Write( (void*)data, sizeof(int8)*numItems ); }
	int Write(const uint8* data, int numItems)																			{ return Write( (void*)data, sizeof(uint8)*numItems ); }
	int Write(const tColouri* data, int numItems)																		{ return Write( (void*)data, sizeof(tColouri)*numItems ); }
	int Write(const tString* data, int numItems)																		{ int bytes = 0; for (int i = 0; i < numItems; i++) { bytes += Write(*data); data++; } return bytes; }
	int Write(const tMath::tVector2* data, int numItems);
	int Write(const tMath::tVector3* data, int numItems);
	int Write(const tMath::tVector4* data, int numItems);
	int Write(const tMath::tQuaternion* data, int numItems);
	int Write(const tMath::tMatrix2* data, int numItems);
	int Write(const tMath::tMatrix4* data, int numItems);
	int Write(const tMath::tEdge* data, int numItems);
	int Write(const tMath::tTri* data, int numItems);
	int Write(const tMath::tQuad* data, int numItems);
	int Write(const tMath::tSphere* data, int numItems);
	template<typename T> int Write(const T* data, int numItems)															{ if (!numItems || !data) return 0; if (NeedsEndianSwap) { T* swapped = new T[numItems]; for (int i = 0; i < numItems; i++) swapped[i] = tGetSwapEndian(data[i]); int n = Write( (void*)swapped, sizeof(T)*numItems ); delete[] swapped; return n; } return Write( (void*)data, sizeof(T)*numItems ); }

	// The layout string lets the Write call know the format of the data so it can perform the appropriate endian swaps
	// if necessary. The characters '1', '2', '4', and '8' are used to represent the number of bytes for each item. In
	// addition there are shorthands for vectors and matrices. Spaces, underscores, and hyphens are ignored, allowing
	// you to group numbers for readability.
	//
	// "v2" becomes "44".
	// "v3" becomes "444".
	// "v4" becomes "4444".
	// "m2" becomes "v2v2" or "4444".
	// "m4" becomes "v4v4v4v4" or "4444444444444444".
	// "q"  becomes "4444".
	// "f"  becomes "4".
	// "d"  becomes "8".
	int Write(const void* data, const tString& layout);

	// Shortened versions of the commands so you don't have to type as much.
	void Begin(uint32 chunkID, Alignment align = Alignment::B4)															{ BeginChunk(chunkID, align); }
	void Begin(uint32 chunkID, int alignmentInBytes)																	{ BeginChunk(chunkID, alignmentInBytes); }

	void End()																											{ EndChunk(); }
	bool GetNeedsEndianSwap() const																						{ return NeedsEndianSwap; }
	int GetNumBytesWritten() const																						{ return WriteBufferPos; }

private:
	// This must remain private, otherwise you wouldn't get a compiler error if the proper Write function didn't exist.
	// Returns the number of bytes written.
	int Write(const void* data, int sizeInBytes);

	struct ChunkInfo : public tLink<ChunkInfo>
	{
		ChunkInfo()																										: StartChunk(0), StartData(0) { }
		ChunkInfo(uint c, uint d)																						: StartChunk(c), StartData(d) { }
		uint StartChunk;				// Start of the chunk.
		uint StartData;					// Start of the data.
	};

	bool NeedsEndianSwap;				// True if the data needs an endianness swap before being written.
	bool IsContainer;					// True if the current chunk being created is a container. False for data-only.
	tList<ChunkInfo> ChunkInfos;
	tFileHandle ChunkFile;

	uint8* WriteBuffer;					// Only non-null if user supplied the buffer to write to.
	int WriteBufferSize;
	int WriteBufferPos;
};


// The tChunk class is used for reading chunk files. Use a tChunkReader to begin parsing tChunks.
class tChunk
{
public:
	tChunk()																											: Chunk(nullptr), LastChunk(nullptr), ItemData(nullptr) { }
	tChunk(uint8* chunk);
	tChunk(uint8* chunk, uint8* lastChunk);
	tChunk(uint8* chunk, tChunk lastChunk);

	// Returns nullptr if this is not a valid chunk.
	uint8* GetData() const;

	// Returns the size of the data in bytes (not including pro or epi-log padding). Returns zero if the chunk is not
	// valid.
	int GetDataSize() const																								{ return IsValid() ? *((int*)(Chunk+4)) : 0; }

	// Same as above except returns data size including all padding.
	int GetDataSizeRaw() const;

	// Returns the ID of the chunk or 0 if it's not a valid chunk. The returned ID has the alignment bits zeroed out.
	// This allows comparison with the same value supplied to Begin.
	uint32 GetID() const																								{ return IsValid() ? (*((uint32*)Chunk)) & 0x8FFFFFFF : 0; }
	uint32 GetIDRaw() const																								{ return IsValid() ? *((uint32*)Chunk) : 0; }
	bool IsValid() const																								{ return Chunk == LastChunk ? false : true; }
	bool IsContainer() const																							{ if (!IsValid()) return false; return ((*((uint32*)Chunk)) & 0x80000000) ? true : false; }
	bool IsDataOnly() const																								{ if (!IsValid()) return false; return ((*((uint32*)Chunk)) & 0x80000000) ? false : true; }

	// Assumes first byte of data is another chunk.
	tChunk GetFirstChunk() const																						{ tAssert(IsContainer()); return tChunk(GetData(), Chunk + GetDataSizeRaw() + 8); }

	// Assumes there is another chunk after the current one. Returns an invalid one if we reach the end in which case
	// Data == InvalidData == the last addr. Also returns invalid if the lastchunk was never specified.
	tChunk GetNextChunk() const																							{ tAssert(IsValid()); return LastChunk ? tChunk(Chunk + GetDataSizeRaw() + 8, LastChunk) : tChunk(); }

	// Chunk should equal LastChunk for this one. Getting data will return an invalid tChunk. Use of this fn is
	// optional as IsValid can be called after each GetNext.
	tChunk GetLastChunk() const																							{ return tChunk(LastChunk, LastChunk); }

	// These don't care about the LastChunk stuff.
	bool operator!=(const tChunk& c) const																				{ return (c.Chunk != Chunk); }
	bool operator==(const tChunk& c) const																				{ return (c.Chunk == Chunk); }

	// Shortened versions of the commands so you don't have to type as much.
	uint8* Data() const																									{ return GetData(); }
	int Size() const																									{ return GetDataSize(); }
	uint32 ID() const																									{ return GetID(); }
	bool Valid() const																									{ return IsValid(); }

	tChunk First() const																								{ return GetFirstChunk(); }
	tChunk Next() const																									{ return GetNextChunk(); }
	tChunk Last() const																									{ return GetLastChunk(); }

	// These item functions are completely optional. They are a more convenient way of parsing multiple items out of a
	// single chunk data. Instead of using GetData and casting and incrementing the pointer for each item, you can just
	// use these accessors. They are considered const as they do not modify the chunk data. However they do need to be
	// called on the correct order (they are not idempotent) since a mutable internal current item pointer is
	// maintained. ItemReset can reset it to the beginning if necessary. In most cases this isn't necessary as that's
	// where you start. All the GetItem functions return the number of bytes read from the chunk stream. In some cases
	// this does not match the size of the item being read. For example, bools are stored as 4 bytes by tChunkWriter.
	void ItemReset() const																								{ tAssert(IsDataOnly()); ItemData = GetData(); }

	// Here's the generic form of GetItem. It will work on most basic data types. The functions after are
	// specializations for cases that would not work otherwise. The list of types known to work are: char, int8, uint8,
	// int16, uint16, int32, uint32, int64, uint64, float, double, tVec2, tVec3, tVec4, tQuat, tMat2, tMat4, tEdge,
	// tTri, tQuad, tSphere, tPixel, and any type of enum or enum class. As intended these reading function do not do
	// any endian conversions. The chunk data is already in the correct format for the destination platform. These
	// functions return the number of bytes read. OK, turns out overloads are matched before specializations, so we
	// can keep it simple and just do that.
	int GetItem(bool& item) const																						{ tAssert(IsDataOnly()); uint8 b = *((uint8*)ItemData); item = b ? true : false; ItemData += sizeof(uint8); return sizeof(uint8); }
	int GetItem(tString& item) const																					{ tAssert(IsDataOnly()); item.Set((char*)ItemData); int n = item.Length()+1; ItemData += n; return n;}
	template<typename T> int GetItem(T& item) const																		{ tAssert(IsDataOnly()); item = *((T*)ItemData); ItemData += sizeof(T); return sizeof(T); }

	// Here are versions that get multiple items at once. numItems must be >= 1.
	int GetItems(bool* dest, int numItems) const																		{ tAssert(IsDataOnly()); for (int i = 0; i < numItems; i++) { uint8 b = *((uint8*)ItemData); *dest = b ? true : false; ItemData += sizeof(uint8); dest++; } return numItems*sizeof(uint8); }
	int GetItems(tString* dest, int numItems) const																		{ tAssert(IsDataOnly()); int bytes = 0; for (int i = 0; i < numItems; i++) { bytes += GetItem(*dest); dest++; } return bytes; }
	template<typename T> int GetItems(T* dest, int numItems) const														{ tAssert(IsDataOnly()); for (int i = 0; i < numItems; i++) { *dest = *((T*)ItemData); ItemData += sizeof(T); dest++; } return numItems*sizeof(T); }

	// Gets the current item as a uint8 pointer, and increments the item pointer by size bytes.
	uint8* GetItemCurrent(int size) const																				{ tAssert(IsDataOnly()); uint8* item = ItemData; ItemData += size; return item; }
	uint8* GetItemCurrent() const																						{ tAssert(IsDataOnly()); return ItemData; }
	void ItemAdvance(int numBytes) const																				{ tAssert(IsDataOnly()); ItemData += numBytes; }

protected:
	uint8* Chunk;						// The memory for this not managed by this class.
	uint8* LastChunk;					// Needed so that GetNextChunk knows when it should return an invalid chunk.
	mutable uint8* ItemData;
};


// Use this to read and parse an already existing chunk file. A tChunk file is simply one or more chunks after each
// other. Essentially the file itself is a container chunk.
class tChunkReader
{
public:
	// If you want to load in the file at a later time. You must load it before calling any other function.
	tChunkReader()																										: IsBufferOwned(false), ReadBufferSize(0), ReadBuffer(nullptr) { }

	// If buffer is nullptr, loads the entire file into memory. Acquires filesize bytes. If you want to manage the
	// memory yourself give it a valid buffer that is big enough. Call SizeNeeded first to find out. Note that a
	// supplied buffer must be aligned to Alignment::Largest.
	tChunkReader(const tString& filename, uint8* buffer = nullptr)														: IsBufferOwned(false), ReadBufferSize(0), ReadBuffer(nullptr) { Load(filename, buffer); }

	// This constructor assumes you already have a buffer with the file loaded. Note that a supplied buffer must be
	// aligned to Alignment_Largest.
	tChunkReader(uint8* buffer, int bufferSizeBytes)																	: IsBufferOwned(false), ReadBufferSize(0), ReadBuffer(nullptr) { Load(buffer, bufferSizeBytes); }
	~tChunkReader()																										{ UnLoad(); }

	// Reads in a file. Note that tChunkRead will need to unload any buffers it may currently be maintaining. Same
	// alignment requirements as above.
	void Load(const tString& filename, uint8* buffer = nullptr);

	// This call assumes the buffer is already allocated and has the data in it. See the corresponding constructor for
	// info. Same alignment requirements as above.
	void Load(uint8* buffer, int bufferSizeBytes);

	// Same as load but returns false at first sign of trouble.
	bool LoadSafe(const tString& filename);

	// Sometimes it is useful to load in a file and then modify the data but not the chunk structure. If this is the
	// case, you can resave the chunk file with the possibly modified data using the function below. Returns number
	// of written bytes.
	int Save(const tString& filename);

	// Unloads from memory any buffers being maintained by tChunkReader.
	void UnLoad();

	bool IsValid() const																								{ return (ReadBuffer && ReadBufferSize) ? true : false; }
	bool Valid() const																									{ return IsValid(); }
	tChunk GetFirstChunk() const																						{ tAssert(IsValid()); return tChunk(ReadBuffer, ReadBuffer + ReadBufferSize); }
	tChunk First() const																								{ return GetFirstChunk(); }
	tChunk Chunk() const																								{ return GetFirstChunk(); }
	static int GetBufferSizeNeeded(const tString& filename)																{ return tSystem::tGetFileSize(filename); }
	static int GetBufferAlignmentNeeded()																				{ return 1 << (int(tChunkWriter::Alignment::Largest) + 2); }

private:
	bool IsBufferOwned;
	int ReadBufferSize;
	uint8* ReadBuffer;
};


// Globally chunk IDs.
//
// The chunk ID may be viewed as bits in the following order:
//
// b31 b30 b29 b28 b27 b26 b25 b24 b23 ... b0
// --- ----------- --------------- ----------
// con    align       major id      minor id
//
// The first bit (b31) is the container indicator (1 = container, 0 = data). The next three bits (b30, b29, b28) are
// reserved and are used for storing alignment information. Up to 512 byte alignment is supported. The 3 bits [0, 7]
// represent the shift amount - 2. That is, alignment = (3-bit-value + 2)^2.
//
// 0 -> 1<<(0+2) = 4 byte alignment.
// 1 -> 1<<(1+2) = 8 byte alignment.
// 2 -> 1<<(2+2) = 16 byte alignemnt.
// ...
// 7 -> 1<<(7+2) = 512 byte alignment.
//
// The reason we add the 2 is because the chunk ID and size must be 4-byte aligned so we are guaranteed 4B alignment
// from the get go. Specifying 1 or 2 byte alignment is therefore pointless. This allows us to increase our max
// alignment from 128 Bytes to 512 Bytes.
//
// The next nybble, bits 27 to 24, is the major id and must be 0 for all core (basic) chunks. Each major system (scene,
// physics, sound, rendering, ai, camera, gameplay, ui, etc) gets its own major chunk id. The major ID nybble plus the
// remaining 24 bits (minor ID) form the chunk ID. The currently assigned major IDs are:
//
// 0 : Core Foundation chunk IDs.
// 1 : Scene. Used by exporter and pipeline tools for things like meshes, models, materials, lights, etc.
// 2 : Rendering. These are used by the runtime rendering engine and the tools that generate data for it.
// 3 : Gameplay. Things like trigger volumes, regions, POIs, COIs, way points, etc.
// 4 : Physics. Rigid bodies, collision meshes, etc.
// 5 : AI. Paths etc.
// 6 : UI. Frontend and interface chunks like screens, buttons, boxes, and other widgets.
// 7 : Sound. Emitters, samples, etc.
// 8 : Camera. Camera paths, triggers, etc used by the runtime.
// 9 : Unassigned.
// A-E: For product (app or game) specific chunks. It's fine if more than one product uses the same ID here.
//	A : Tactile.
// F: Generic. Don't know where it belongs. Use this major ID.
//
// In terms of chunk IDs the first nybble should be set to 0x8 for container chunks (chunks that only contain more
// chunks for their data). For chunks that contain data only, the first nybble should be 0x0. Only data chunks may have
// alignment. Everything else is 4-byte aligned.
//
// Note the use of 'old' style (not enum class) enums for chunk IDs. Since IDs more closely represent const integers,
// the type-safety of 'enum class' makes things more awkward. Casting to int every time would be onerous.
//
namespace tChunkID
{
	#if defined(Previous) || defined(Future)
		#error Previous and Future must not be previously defined if you include this header.
	#endif
	#define Previous(n)																									// Use this if the chunk was previously defined but you want to indicate valid usage within a sub-chunk.																									
	#define Future(n)																									// Use this if the chunk isn't implemented yet, but you want to show where it WILL go.


	// Core chunk IDs. Useful all over the place.
	enum Core
	{
		Core_String																				= 0x00001000,			// Data is a null terminated character string that is the name of the object.
		Core_Name																				= Core_String,
		Core_ID																					= 0x00002000,			// Data is a unique unsigned 32bit integer.
		Core_Bool																				= 0x00003000,			// Data is a byte with value 0 or 1 that represents false or true.
		Core_Int																				= 0x00004000,			// Data is a signed 32bit integer.
		Core_Real																				= 0x00005000,			// Data is a 4byte float.
		Core_Vector3																			= 0x00006000,			// Three 4-byte floating point values.
		Core_Position																			= Core_Vector3,
		Core_Vector4																			= 0x00007000,			// Four 4-byte floating point values.
		Core_Quaternion																			= 0x00008000,			// Four 4-byte floating point values.
		Core_Orientation																		= Core_Quaternion,
		Core_Matrix4																			= 0x00009000,			// Sixteen 4-byte floating point values.
		Core_Transformation																		= Core_Matrix4,
		Core_Version																			= 0x0000A000,			// Major, Minor, and possibly Revision, Build, and Release numbers.
	};


	// Scene chunk IDs used in exported tScene files.
	enum Scene
	{
		Future(Core_Version)
		Scene_Scene																				= 0x81001000,			// A scene contains objects (models, lights, cameras etc) and materials (which are used by some of the objects).
			Future(Core_Name)
			Scene_ObjectList																	= 0x81002000,			// An object can be a model, camera, light, skeleton, etc.
				Scene_Object																	= 0x81002100,
					Previous(Core_ID)
					Previous(Core_Name)
					Scene_AttributeList															= 0x8101A000,			// Models may have other, as yet unknown, attributes.
						Scene_Attribute															= 0x8101B000,
							Scene_AttributeName													= 0x0101B100,			// The attribute name.
							Scene_AttributeType													= 0x0101B200,			// See class aAttrib for supported types.
							Scene_AttributeValue												= 0x0101B300,			// Value is to be interpreted based on what type the attribute is.

				Scene_PolyModel																	= 0x81003000,
					Previous(Scene_Object)
					Previous(Core_Bool)																					// Is model a member of an LOD group.
					Scene_Mesh																	= 0x81004000,			// The mesh is a triangle mesh.
						Scene_MeshProperties													= 0x0100A000,			// Contains, in order, NumFaces, NumEdges, NumVerts, NumNormals, NumUVs, and NumColours
						Scene_FaceTable_VertPositionIndices										= 0x0100B000,			// Each entry contains 3 indices into the vertex table.
						Scene_FaceTable_VertWeightSetIndices									= 0x0100C000,			// Each entry contains 3 indices into the vert table of weight sets.
						Scene_FaceTable_VertNormalIndices										= 0x0100D000,			// Vertex normals. Each entry contains 3 indices into the normal table.
						Scene_FaceTable_FaceNormals												= 0x0100E000,			// Face normals. Each entry is the actual face normal.
						Scene_FaceTable_TexCoordIndices											= 0x0100F000,			// Texture coords. Each entry contains 3 indices into the uv table. The UV table may not be present on some models.
						Scene_FaceTable_NormalMapTexCoordIndices								= 0x01010000,			// Normal (bump) Map Texture coords. Each entry contains 3 indices into the normal map uv table. The normal map UV table may not be present on some models.
						Scene_FaceTable_ColourIndices											= 0x01011000,			// CBV. Each entry contains 3 indices into the colour table. The colour table may not be present on some models.
						Scene_FaceTable_MaterialIDs												= 0x01012000,			// Each entry is the material ID. The material table may not be present on some models. This decision is, of course, debatable, but I think a model could still look good and be, for example, only be CBV'd.
						Scene_FaceTable_TangentIndices											= 0x01012100,			// Each entry is a vector4 with (x,y,z) equal to the tangent aligned with the U texture direction and w the direction factor (1.0 or -1.0).
						Scene_EdgeTable_VertPositionIndices										= 0x01013000,			// Contains 2 indices into the vertex table.
						Scene_VertTable_Positions												= 0x01014000,			// Contains xyz's in local model space.
						Scene_VertTable_WeightSets												= 0x01015000,			// A table of tWeightSets. Each tWeightSet is an array of MaxJointInfluences tWeights. A tWeight is a triple (SkelID, JointID, Weight).
						Scene_VertTable_Normals													= 0x01016000,			// Contains normalized vertex normals in model space.
						Scene_VertTable_TexCoords												= 0x01017000,			// Contains texture UV coordinates. This table may not be present on all models.
						Scene_VertTable_NormalMapTexCoords										= 0x01018000,			// Contains texture normal (bump) map UV coordinates. This table may not be present on all models.
						Scene_VertTable_Colours													= 0x01019000,			// Contains RGB triples. This table may not be present on all models.
						Scene_VertTable_Tangents												= 0x01019100,			// Contains XYZWs where the tangent is (x, y, z) and the w is the direction factor (-1.0 or 1.0). This table may not be present on all models.

				Scene_PatchModel																= 0x8101C000,			// Not all models will be geometry. This guy will have a lot of the same chunks as the geom model.
					Previous(Scene_Object)
					Future(Core_Transformation)

				Scene_Skeleton																	= 0x8101D000,
					Previous(Scene_Object)
					Scene_SkeletonInfo															= 0x0101E000,			// NumJoints, NumPoses, FramePeriod(sec)
					Scene_JointList																= 0x8101F000,
						Scene_Joint																= 0x81020000,
							Future(Core_ID)
							Future(Core_Name)
							Scene_JointTranslation												= 0x01021000,			// Base Pose Translation. 4 floats (x, y, z, 0). Homogeneous coords.
							Scene_JointOrientation												= 0x01022000,			// Base Pose Orientation. 4 floats (w, y, x, y). A quaternion.
							Scene_JointScale													= 0x01023000,			// Base Pose Scale. 4 floats (x, y, z, 1).
							Scene_JointParentID													= 0x01024000,			// Parent ID
							Scene_JointNumChildren												= 0x01025000,			// Number of children joints.
							Scene_JointChildIDTable												= 0x01026000,			// Children IDs. Chunk won't be present if there are no children.

					Scene_PoseList																= 0x81027000,			// A list of poses for animation.
						Scene_Pose																= 0x81028000,			// A skeletal pose... one frame of animation.
							Scene_PoseInfo														= 0x01029000,			// FrameNum (-1 means base/bind) pose, ElapsedTime(sec).
							Scene_QuaternionTable												= 0x0102A000,			// A table of quaternions that orient the joints.
							Scene_ScaleTable													= 0x0102B000,			// A table of quaternions that orient the joints.

				Scene_Camera																	= 0x8102B100,
					Previous(Scene_Object)
					Scene_CameraParameters														= 0x0102B200,			// Aspect ratio, Horizontal FOV, Vertical FOV, Near Clip, Far Clip, 12 Bytes future growth.
					Scene_Projection															= 0x0102B300,

				Scene_Light																		= 0x8102B400,
					Previous(Scene_Object)
					Scene_LightType																= 0x0102B480,
					Scene_LightParameters														= 0x0102B500,			// Near Atten start, Near Atten end, far atten start, far atten end, 32 bytes for future.

				Scene_Path																		= 0x8102B540,
					Previous(Scene_Object)
					Scene_PathParameters														= 0x0102B550,			// IsClosed (4 bytes), num CVs (4 bytes).
					Scene_PathControlVertTable													= 0x0102B560,

			// The instance list contains a list of, well, instances. An instance simply consists of a transformation
			// matrix and a pointer to the object being instanced. The "pointer" is in the form of an object ID. Since
			// IDs are only guaranteed to be unique within a particular object type, the object type is specified in
			// addition to the ID. A set of per-instance attributes may also be present in addition to any per-object
			// instance lists that may exist.
			Scene_InstanceList																	= 0x8102B600,
				Scene_Instance																	= 0x8102B700,
					Previous(Scene_Object)
					Scene_InstanceParameters													= 0x0102B710,			// 4Bytes: The type of instance this is. 0=Invalid, 1=PolyModel, 2=PatchModel, 3=Skeleton, 4=Camera, 5=Light. 4Bytes: The ID of the object this instance represents. 4Bytes: Billboarding mode. 0 -> no-billboarding. non-zero for the other types like view-plane alighned, view-point, y-axis only, etc. See BillboardTracking and BillboardMode.
					Scene_Transformation														= 0x01008000,			// Model to world space transformation matrix.

			Scene_SelectionList																	= 0x8102B800,
				Scene_Selection																	= 0x8102B900,
					Previous(Scene_Object)
					Scene_SelectionInstanceIDs													= 0x0102BA00,

			Scene_GroupList																		= 0x8102C000,			// Object groups. These groups are in a separate list because the are not visible objects in the world.
				Scene_LodGroup																	= 0x8102D000,
					Previous(Scene_Object)
					Scene_LodParams																= 0x8102E000,			// A list of lod infos.
						Scene_LodParam															= 0x0102E100,			// ModelID, Screen ratio threshold.
					Future(NamedGroup)

			Scene_MaterialList																	= 0x8102F000,
				Scene_Material																	= 0x81030000,			// ID plus other stuff.
					Previous(Scene_Object)
					Scene_MaterialProperties													= 0x01031000,			// Stuff like semitransparency, texture data format, shinnyness, etc.
					Scene_MaterialTextureDiffuse												= 0x01032000,			// Specifies the diffuse texture. It's properties and texture file location (probably a tga or bmp). The file will be specified relative to the .txp file.
					Scene_MaterialTextureNormalMap												= 0x01033000,			// Normap map.
					Scene_MaterialTextureA														= 0x01034000,			// Alternate texture A.
					Scene_MaterialTextureB														= 0x01035000,			// Alternate texture B.
					Scene_MaterialTextureC														= 0x01036000,			// Alternate texture C.
					Scene_MaterialTextureD														= 0x01036100,			// Alternate texture D.
					Scene_MaterialTextureE														= 0x01036200,			// Alternate texture E.
					Scene_MaterialShaderFile													= 0x01038000,			// AFX effects file.
					Scene_MaterialNormalMapFile													= 0x01039000,			// Normal map file.
	};

	// Tacent image module.
	enum Image
	{
		Image_Picture																			= 0x81040000,
			Image_PictureProperties																= 0x01040010,			// Width, height.
			Image_PicturePixels																	= 0x01040020,

		Image_Layer																				= 0x81040030,
			Image_LayerProperties																= 0x01040040,			// Pixel format, width, height.
			Image_LayerData																		= 0x01040050,

		Image_Texture																			= 0x81040100,
			Image_TextureProperties																= 0x01040110,			// Opaque.
			Image_TextureLayers																	= 0x81040120,
				Previous(Image_Layer)

		Image_Cubemap																			= 0x81040200,
			Image_CubemapProperties																= 0x01040208,			// All opaque.
			Image_CubemapSides																	= 0x81040210,
				Image_CubemapSide																= 0x01040220,
					Image_CubemapSideProperties													= 0x01040230,			// tSide.
					Previous(Image_Texture)
	};

	// Rendering chunk IDs.
	enum Render
	{
		Render_ModelListProperties																= 0x02006000,			// Number of models in the model list.
		Render_ModelList																		= 0x82007000,
			Render_Model																		= 0x82008000,
				Render_ModelID																	= 0x02009000,
				Render_ModelProperties															= 0x0200A000,			// Number of LOD meshes present.
				Render_LodMeshList																= 0x8200A200,			// The list of LOD meshes. Even models with only one mesh will have a list with 1 mesh inside it.
					Render_Mesh																	= 0x8200B000,
						Render_MeshProperties													= 0x0200C000,			// Distance Threshold. How many materials are in the mesh. Min radius. Max (bounding) radius. Bounding Box (Min xyz followed by max xyz).
						Render_MeshMaterialList													= 0x8200D000,
							Render_MeshMaterial													= 0x8200E000,
								Render_MeshMaterialProperties									= 0x0200F000,			// FaceIndexFormat, Num faces, Num vertices.
								Render_MeshMaterialID											= 0x02010000,			// The ID of a material in the global material list. This ID can also be considered a unique identifier for the mesh material within the current model.
								Render_MeshMaterialTristripIndexTable							= 0x02011000,			// Each element is an index into the vertex table. The indices represent a triangle strip.
								Render_MeshMaterialVertexTable									= 0x02012000,			// Each element contains a position, a unit normal, UV texture coords, and a colour.
								Render_MeshMaterialVertexPositionTable							= 0x02013000,
								Render_MeshMaterialVertexNormalTable							= 0x02014000,
								Render_MeshMaterialVertexUVTable								= 0x02015000,
								Render_MeshMaterialVertexColourTable							= 0x02016000,

		Render_InstanceListProperties															= 0x02017000,			// Number of instances in the instance list.
		Render_InstanceList																		= 0x82018000,
			Render_Instance																		= 0x82018100,
				Render_InstanceID																= 0x02018200,			// A unique instance ID.
				Render_InstanceProperties														= 0x02018300,			// 32bit hash of the instance name, InstanceType, InstanceRole, ModelID being instanced, Instance behaviour mode (rotate mode and alignment (VP or VP))
				Future(Core_Transformation)

		Render_MaterialList																		= 0x82021000,

			// Material ID, Material Type, Ambient Colour, Diffuse Colour, Specular Colour, Emissive Colour, Opacity,
			// Shininess (0.0-128.0), Diffuse Textures Completely Opaque (boolean). After all these 4 byte quantities
			// comes the diffuse texture id table: 16 32bit Texture IDs (0 is for diffuse, 1-7 for swap diffuse textures,
			// rest for multitexturing of some kind or another). Texture IDs of 0xFFFFFFFF are invalid.
			Render_Material																		= 0x02021100,

		Render_TextureLayer																		= 0x8200A000,
			Render_TextureLayerProperties														= 0x0200B000,			// Width, height, numBytes.
			Render_TextureLayerData																= 0x0200C000,

		Render_TextureListProperties															= 0x02022000,			// Total number of textures across all texture files, Number of textures in this file.
		Render_TextureList																		= 0x82022100,
			Render_Texture																		= 0x82022200,
				Render_TextureProperties														= 0x02022300,			// TextureID (consecutive starting at 0), pixel format (see Texture.h), boolean mipmapped, num mipmap layers (including first) this will be 1 if no mipmapping.
				Render_TextureLayerList															= 0x82022400,
					Future(Render_TextureLayer)

		Render_TextureMapFontListProperties														= 0x02023000,			// Number of fonts.
		Render_TextureMapFontList																= 0x82023100,
			Render_TextureMapFont																= 0x82023200,
				Render_TextureMapFontProperties													= 0x02023300,			// Name hash, start character, num characters, pixel format (see Texture.h), num mipmap layers (1 for no mipmapping).
				Render_TextureMapFontGlyphInfos													= 0x02023400,			// Each glyphinfo consists of 2 UVs. There will as many of these as there are characters.
				Render_TextureMapFontLayerList													= 0x82023500,
					Future(Render_TextureLayer)

		Render_WorldName																		= 0x02024000,
		Render_WorldParameters																	= 0x02024100,
	};


	// Physics chunk IDs.
	enum Physics
	{
		Physics_ModelListProperties																= 0x04019000,			// Number of models in the model list.
		Physics_ModelList																		= 0x84019100,
			Physics_Model																		= 0x84019200,
				Physics_ModelID																	= 0x04019300,
				Physics_ModelProperties															= 0x04019400,			// Number of LOD meshes present.
				Physics_CollisionMesh															= 0x04019500,			// The list of LOD meshes. Even models with only one mesh will have a list with 1 mesh inside it.

		// @todo The collision chunks should probably follow the same methodology as other instances. For now there is
		// simply a single chunk for both the instance part (the transformation) and the mesh itself.
		Physics_CollisionGeomPartitioned														= 0x84020000,			// @todo Hey, what's inside this chunk.
			Physics_CollisionGeomPartitionedID													= 0x04020100,
			Physics_CollisionGeomPartitionedTransformation										= 0x04020200,
			Physics_CollisionGeomPartitionedData												= 0x04020300,
			Physics_CollisionGeomPartitionedSize												= 0x04020400,
			Physics_CollisionGeomPartitionedProperties											= 0x04020500,
			Physics_CollisionGeomPartitionedVertPosTable										= 0x04020600,
			Physics_CollisionGeomPartitionedTriTable											= 0x04020700,
			Physics_CollisionGeomPartitionedTriMatTable											= 0x04020800,
	};


	// Sound chunk IDs.
	enum Sound
	{
		Sound_SampleListProperties																= 0x07060000,			// Number of samples.
		Sound_SampleList																		= 0x87061000,
			Sound_Sample																		= 0x87062000,
				Sound_SampleProperties															= 0x07063000,			// SampleNumber (0 is default), size in bytes, frequency in Hz, looping, format as an ALenum value.
				Sound_SampleData																= 0x07064000,

		Sound_EmitterList																		= 0x87065000,
			Sound_Emitter																		= 0x07066000,			// XYZ Position, sample ID, inner radius, outer radius, 8 bytes or zeros for future use.
	};

	#undef Previous																									
	#undef Future
}																														// Ends tChunkID namespace.


// These may be thrown by the chunk file parsing code.
struct tChunkError : public tError
{
	tChunkError(const char* format, ...)																				: tError("[tChunk] ") { va_list marker; va_start(marker, format); Message += tvsPrintf(Message, format, marker); }
	tChunkError(const tString& m)																						: tError("[tChunk] ") { Message += m; }
	tChunkError()																										: tError("[tChunk]") { }
};


// Implementation below this line.


inline tChunk::tChunk(uint8* chunk) :
	Chunk(chunk),
	LastChunk(0)
{
	uint32 idaa = *((uint32*)Chunk);
	int shift = ((idaa & 0x70000000) >> 28) + 2;
	int align = 1 << shift;

	// We are safe to use uint32 cast here even for 64 bit pointers because the alignment requirements are much
	// smaller. That is, align is much less than 2^32.
	int bytesAfterAlign = uint32(uint64(chunk+8)) % align;
	int pad = bytesAfterAlign ? (align - bytesAfterAlign) : 0;
	ItemData = (chunk+8) + pad;
}


inline tChunk::tChunk(uint8* chunk, uint8* lastChunk) :
	Chunk(chunk),
	LastChunk(lastChunk)
{
	uint32 idaa = *((uint32*)Chunk);
	int shift = ((idaa & 0x70000000) >> 28) + 2;
	int align = 1 << shift;

	int bytesAfterAlign = uint32(uint64(chunk+8)) % align;
	int pad = bytesAfterAlign ? (align - bytesAfterAlign) : 0;
	ItemData = (chunk+8) + pad;
}


inline tChunk::tChunk(uint8* chunk, tChunk lastChunk) :
	Chunk(chunk),
	LastChunk(lastChunk.Chunk)
{
	uint32 idaa = *((uint32*)Chunk);
	int shift = ((idaa & 0x70000000) >> 28) + 2;
	int align = 1 << shift;

	int bytesAfterAlign = uint32(uint64(chunk+8)) % align;
	int pad = bytesAfterAlign ? (align - bytesAfterAlign) : 0;
	ItemData = (chunk+8) + pad;
}


inline uint8* tChunk::GetData() const
{
	if (!IsValid())
		return nullptr;

	uint32 idaa = *((uint32*)Chunk);
	int shift = ((idaa & 0x70000000) >> 28) + 2;
	int align = 1 << shift;

	int bytesAfterAlign = uint32(uint64(Chunk+8)) % align;
	int pad = bytesAfterAlign ? (align - bytesAfterAlign) : 0;
	return Chunk + 8 + pad;
}


inline int tChunk::GetDataSizeRaw() const
{
	if (!IsValid())
		return 0;

	uint8* data = Chunk+8;
	uint32 idaa = *((uint32*)Chunk);
	int shift = ((idaa & 0x70000000) >> 28) + 2;
	int align = 1 << shift;

	int bytesAfterAlign = uint32(uint64(data)) % align;
	int prologPad = bytesAfterAlign ? (align - bytesAfterAlign) : 0;
	int dataSize = GetDataSize();
	data += prologPad + dataSize;

	int epilogPad = (uint32(uint64(data)) % 4) ? (4 - (uint32(uint64(data)) % 4)) : 0;
	return prologPad + dataSize + epilogPad;
}
