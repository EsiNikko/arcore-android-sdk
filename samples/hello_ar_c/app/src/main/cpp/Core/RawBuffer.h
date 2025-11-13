#pragma once
#include "Core/Core.h"

namespace Ray
{
	// Non-owning raw buffer
	struct RawBuffer
	{
		void* Data = nullptr;
		uint64_t Size = 0;

		RawBuffer() = default;
		explicit RawBuffer(uint64_t size)
		{
			Allocate(size);
		}
		RawBuffer(const void* data, uint64_t size) : Data(const_cast<void*>(data)), Size(size) {}

		static RawBuffer Copy(const RawBuffer& other)
		{
			RawBuffer buffer(other.Size);
			memcpy(buffer.Data, other.Data, other.Size);
			return buffer;
		}

		static RawBuffer Copy(const void* data, uint64_t size)
		{
			RAY_CORE_ASSERT(data != nullptr, "Data is null!");
			RAY_CORE_ASSERT(size > 0, "Size is 0!");
			RawBuffer buffer(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}

		void Allocate(uint64_t size)
		{
			Release();
			if (size == 0) return;
			Size = size;
			Data = new uint8_t[size];
		}
		
		void Release()
		{
			delete[] static_cast<uint8_t*>(Data);
			Data = nullptr;
			Size = 0;
		}

		void ZeroInitialize() const
		{
			if (Data)
				memset(Data, 0, Size);
		}

		explicit operator bool() const
		{
			return Data != nullptr;
		}

		uint8_t& operator[](int index)
		{
			return static_cast<uint8_t*>(Data)[index];
		}

		uint8_t operator[](int index) const
		{
			return static_cast<uint8_t*>(Data)[index];
		}

		template <typename T>
		T* As() const
		{
			return static_cast<T*>(Data);
		}

		template<typename T>
		T& Read(uint64_t offset = 0)
		{
			RAY_CORE_ASSERT(offset + sizeof(T) <= Size, "Buffer overflow!");
            return *static_cast<T*>((uint64_t )Data + offset);
		}

		template<typename T>
		const T& Read(uint64_t offset = 0) const
		{
			RAY_CORE_ASSERT(offset + sizeof(T) <= Size, "Buffer overflow!");
			return *static_cast<T*>((uint64_t )Data + offset);
		}

		[[nodiscard]] void* ReadNewCopy(uint64_t size, uint64_t offset) const
		{
			RAY_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
			void* buffer = new uint8_t[size];
			memcpy(buffer, (void*)((uint64_t)Data + offset), size);
			return buffer;
		}
				
		void Write(const void* data, uint64_t size, uint64_t offset = 0) const
		{
			RAY_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
			memcpy((void*)((uint64_t)Data + offset), data, size);
		}
	};

	struct ScopedRawBuffer
	{
		RAY_DELETE_COPY(ScopedRawBuffer);
		RAY_DELETE_MOVE(ScopedRawBuffer);
		explicit ScopedRawBuffer(uint64_t size) : m_Buffer(size) {}
		explicit ScopedRawBuffer(RawBuffer buffer) { m_Buffer.Release(); m_Buffer = buffer; }
		~ScopedRawBuffer() { m_Buffer.Release(); }

		[[nodiscard]] void* Data() const { return m_Buffer.Data; }
		[[nodiscard]] uint64_t Size() const { return m_Buffer.Size; }
		template <typename T>
		T* As() { return m_Buffer.As<T>(); }
		explicit operator bool() const { return (bool)m_Buffer; }
	private:
		RawBuffer m_Buffer;
	};
}
