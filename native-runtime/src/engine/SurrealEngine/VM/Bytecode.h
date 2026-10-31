#pragma once

#include "Packages/Core/UClass.h"
#include "Packages/Core/Properties/UProperty.h"
#include "Package/Package.h"
#include "Expression.h"
#include "Utils/MemoryArena.h"
#include "HP2.h"

class BytecodeStream;

class Bytecode
{
public:
	Bytecode(const Array<uint8_t>& bytecode, Package* package);

	int FindStatementIndex(uint16_t offset) const
	{
		auto it = OffsetToExpression.find(offset);
		return it != OffsetToExpression.end() ? it->second->StatementIndex : -1;
	}

	int FindOffset(int statementIndex) const
	{
		for (const auto& it : OffsetToExpression)
		{
			if (it.second->StatementIndex == statementIndex)
				return it.first;
		}
		return -1;
	}

	int FindLabelIndex(const NameString& label)
	{
		if (Statements.empty())
			return -1;
		LabelTableExpression* labels = dynamic_cast<LabelTableExpression*>(Statements.back());
		if (labels)
		{
			for (LabelEntry& entry : labels->Labels)
			{
				if (entry.Name == label)
					return FindStatementIndex(entry.Offset);
			}
		}
		return -1;
	}

	Array<Expression*> Statements;

private:
	Expression* ReadToken(BytecodeStream* stream, int depth);

	template<typename T>
	T* Create(uint16_t offset)
	{
		T* obj = arena.Create<T>();
		OffsetToExpression[offset] = obj;
		return obj;
	}

	std::map<uint16_t, Expression*> OffsetToExpression;
	static MemoryArena arena;
};

class BytecodeStream
{
public:
	BytecodeStream(const void* d, size_t s, Package* package) : data((uint8_t*)d), size((uint32_t)s), package(package) { }

	void ReadBytes(void* d, uint32_t s)
	{
		if (pos + s > size)
			Exception::Throw("BytecodeStream::ReadBytes: Unexpected end of file in " + package->GetPackageName().ToString());
		memcpy(d, data + pos, s);
		pos += s;
	}

	int8_t ReadInt8() { int8_t t; ReadBytes(&t, 1); return t; }
	int16_t ReadInt16() { int16_t t; ReadBytes(&t, 2); return t; }
	int32_t ReadInt32() { int32_t t; ReadBytes(&t, 4); return t; }
	int64_t ReadInt64() { int64_t t; ReadBytes(&t, 8); return t; }
	float ReadFloat() { float t; ReadBytes(&t, 4); return t; }

	int32_t ReadIndex() { return ReadInt32(); }

	uint8_t ReadUInt8() { return ReadInt8(); }
	uint16_t ReadUInt16() { return ReadInt16(); }
	uint32_t ReadUInt32() { return ReadInt32(); }
	uint64_t ReadUInt64() { return ReadInt64(); }

	std::string ReadAsciiZ()
	{
		std::string s;
		while (true)
		{
			char c = ReadInt8();
			if (c == 0) break;
			s.push_back(c);
		}
		return s;
	}

	std::wstring ReadUnicodeZ()
	{
		std::wstring s;
		while (true)
		{
			wchar_t c = ReadUInt16();
			if (c == 0) break;
			s.push_back(c);
		}
		return s;
	}

	// HP2's bytecode: DebugInfo records are skipped and its token numbers map to the stock ones
	void SetHP2Tokens(bool enable) { hp2Tokens = enable; }

	bool SkipDebugInfo()
	{
		uint32_t start = pos;
		while (hp2Tokens && pos < size)
		{
			size_t recordSize = HP2::DebugInfoSize(data + pos, size - pos);
			if (recordSize == 0)
				break;
			pos += (uint32_t)recordSize;
		}
		return pos != start;
	}

	ExprToken ReadToken()
	{
		SkipDebugInfo();
		if (pos >= size)
			Exception::Throw("BytecodeStream::ReadToken: Unexpected end of file in " + package->GetPackageName().ToString());
		uint8_t token = data[pos++];
		return (ExprToken)(hp2Tokens ? HP2::ToStockToken(token) : token);
	}

	ExprToken PeekToken()
	{
		uint32_t start = pos;
		SkipDebugInfo();
		uint32_t tokenpos = pos;
		pos = start;
		if (tokenpos >= size)
			Exception::Throw("BytecodeStream::PeekToken: Unexpected end of file in " + package->GetPackageName().ToString());
		return (ExprToken)(hp2Tokens ? HP2::ToStockToken(data[tokenpos]) : data[tokenpos]);
	}

	NameString ReadName()
	{
		return package->GetName(ReadIndex());
	}

	template<typename T>
	T* ReadObject()
	{
		return UObject::Cast<T>(package->GetUObject(ReadIndex()));
	}

	uint16_t GetOffset() const { return pos; }
	bool IsEnd() const { return pos == size; }

	int GetVersion() const
	{
		return package->GetVersion();
	}

private:
	uint8_t* data = nullptr;
	uint32_t size = 0;
	uint32_t pos = 0;
	Package* package = nullptr;
	bool hp2Tokens = false;
};
