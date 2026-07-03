#include "CoreRHI.h"

namespace CE::RHI
{
	FreeListAllocator::~FreeListAllocator()
	{
		Shutdown();
	}

	void FreeListAllocator::Init(const Descriptor& desc)
	{
		this->descriptor = desc;

		CE_ASSERT(desc.capacityInBytes > 0, "FreeListAllocator Capacity must be greater than zero.");
		
		headNode = AcquireNode();
		headNode->address = desc.baseAddress;
		headNode->size = desc.capacityInBytes;
	}

	void FreeListAllocator::Shutdown()
	{
		for (FreeRange* pooledNode : pooledNodes)
		{
			delete pooledNode;
		}

		pooledNodes.Clear();
	}

	VirtualAddress FreeListAllocator::Allocate(SIZE_T byteCount, SIZE_T byteAlignment)
	{
		return AllocateFirstFit(byteCount, byteAlignment);
	}

	void FreeListAllocator::DeAllocate(VirtualAddress alignedAddress)
	{
		AllocationRecord record{};
		if (!RemoveAllocation(alignedAddress, record))
		{
			return;
		}

		// Acquire a node and prepare it for re-insertion
		FreeRange* newNode = AcquireNode();
		newNode->address = record.address.ptr;
		newNode->size = record.size;

		// Insert into the FreeList (Must maintain sorted order by address)
		FreeRange* prev = nullptr;
		FreeRange* curr = headNode;

		// Find position
		while (curr != nullptr && curr->address < newNode->address)
		{
			prev = curr;
			curr = curr->nextFree;
		}

		// Insert newNode between prev and curr
		newNode->prevFree = prev;
		newNode->nextFree = curr;

		if (prev != nullptr)
			prev->nextFree = newNode;
		else
			headNode = newNode; // Inserting at the very start

		if (curr != nullptr)
			curr->prevFree = newNode;

		// - Coalesce -

		// Merge Forward: Does the new node touch the next node?
		if (newNode->nextFree != nullptr && (newNode->address + newNode->size == newNode->nextFree->address))
		{
			FreeRange* nextNode = newNode->nextFree;

			newNode->size += nextNode->size;
			newNode->nextFree = nextNode->nextFree;

			if (newNode->nextFree != nullptr)
				newNode->nextFree->prevFree = newNode;

			ReleaseNode(nextNode);
		}

		// Merge Backward: Does the previous node touch the new node?
		if (newNode->prevFree != nullptr && (newNode->prevFree->address + newNode->prevFree->size == newNode->address))
		{
			FreeRange* prevNode = newNode->prevFree;

			prevNode->size += newNode->size;
			prevNode->nextFree = newNode->nextFree;

			if (prevNode->nextFree != nullptr)
				prevNode->nextFree->prevFree = prevNode;

			ReleaseNode(newNode);
		}
	}

	VirtualAddress FreeListAllocator::AllocateFirstFit(SIZE_T byteCount, SIZE_T byteAlignment)
	{
		FreeRange* node = headNode;
		if (byteAlignment == 0)
			byteAlignment = 1;

		CE_ASSERT((byteAlignment & (byteAlignment - 1)) == 0, "Byte alignment must be a power of 2!");

		while (node != nullptr)
		{
			SIZE_T address = node->address;
			SIZE_T alignedAddress = Memory::AlignUp(address, byteAlignment);
			SIZE_T padding = alignedAddress - address;
			SIZE_T totalSize = padding + byteCount;

			if (totalSize <= node->size)
			{
				if (node->size > totalSize) // Split the node
				{
					SIZE_T remainingSize = node->size - totalSize;

					FreeRange* splitNode = AcquireNode();
					splitNode->address = address + totalSize;
					splitNode->size = remainingSize;

					if (node->prevFree)
					{
						node->prevFree->nextFree = splitNode;
					}
					if (node->nextFree)
					{
						node->nextFree->prevFree = splitNode;
					}

					splitNode->prevFree = node->prevFree;
					splitNode->nextFree = node->nextFree;

					if (node == headNode)
					{
						headNode = splitNode;
					}

					ReleaseNode(node);
				}
				else // No split required
				{
					if (node->prevFree != nullptr)
					{
						node->prevFree->nextFree = node->nextFree;
					}

					if (node->nextFree != nullptr)
					{
						node->nextFree->prevFree = node->prevFree;
					}

					if (node == headNode)
					{
						headNode = node->nextFree;
					}

					ReleaseNode(node);
				}

				TrackAllocation({
					.address = address,
					.alignedAddress = alignedAddress,
					.size = totalSize
				});

				return VirtualAddress{ alignedAddress };
			}

			node = node->nextFree;
		}

		return {};
	}

	FreeListAllocator::FreeRange* FreeListAllocator::AcquireNode()
	{
		if (pooledNodes.IsEmpty())
		{
			return new FreeRange{ .address = 0, .size = 0, .prevFree = nullptr, .nextFree = nullptr };
		}

		FreeRange* node = pooledNodes.GetLast();
		pooledNodes.RemoveAt(pooledNodes.GetSize() - 1);
		return node;
	}

	void FreeListAllocator::ReleaseNode(FreeRange* node)
	{
		node->address = 0;
		node->size = 0;
		node->prevFree = node->nextFree = nullptr;
		pooledNodes.Add(node);
	}

	void FreeListAllocator::TrackAllocation(const AllocationRecord& record)
	{
		CE_ASSERT(numAllocations < kMaxAllocations, "Allocation record pool exhausted!");

		SIZE_T low = 0;
		SIZE_T high = numAllocations;

		while (low < high)
		{
			SIZE_T mid = low + (high - low) / 2;
			if (allocationRecords[mid].address.ptr < record.address.ptr)
			{
				low = mid + 1;
			}
			else
			{
				high = mid;
			}
		}

		SIZE_T insertIdx = low;

		if (insertIdx < numAllocations)
		{
			SIZE_T bytesToMove = (numAllocations - insertIdx) * sizeof(AllocationRecord);
			std::memmove(&allocationRecords[insertIdx + 1], &allocationRecords[insertIdx], bytesToMove);
		}

		allocationRecords[insertIdx] = record;
		numAllocations++;
	}

	bool FreeListAllocator::RemoveAllocation(VirtualAddress alignedAddress, AllocationRecord& outRecord)
	{
		// 1. Binary search to find index
		SIZE_T low = 0;
		SIZE_T high = numAllocations;
		SIZE_T foundIdx = (SIZE_T)-1;

		while (low < high)
		{
			SIZE_T mid = low + (high - low) / 2;
			if (allocationRecords[mid].alignedAddress.ptr < alignedAddress.ptr)
			{
				low = mid + 1;
			}
			else if (allocationRecords[mid].alignedAddress.ptr > alignedAddress.ptr)
			{
				high = mid;
			}
			else
			{
				foundIdx = mid;
				break;
			}
		}

		if (foundIdx == (SIZE_T)-1)
			return false;

		outRecord = allocationRecords[foundIdx];

		// Shift left to fill the gap
		if (foundIdx < numAllocations - 1)
		{
			SIZE_T bytesToMove = (numAllocations - foundIdx - 1) * sizeof(AllocationRecord);
			std::memmove(&allocationRecords[foundIdx],
				&allocationRecords[foundIdx + 1],
				bytesToMove);
		}

		numAllocations--;
		return true;
	}

} // namespace CE::RHI
