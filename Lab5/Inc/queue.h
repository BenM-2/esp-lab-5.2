/**
 * @file Queue.h
 * @author Ben Marples
 */

#ifndef Queue_IMPL
#define Queue_IMPL

//------------------------------------------------------------------------------
// includes
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------
// Structures

typedef struct Queue
{
    uint8_t *const data;
    size_t const dataSize;

    size_t writeIndex;
    size_t readIndex;
} Queue;

typedef enum
{
    Queue_Ok,
    Queue_Insufficient_Space,
    Queue_Insufficient_Data,
} Queue_err_t;

//------------------------------------------------------------------------------
// Inline Function Implementations

static inline size_t Queue_available_read(const Queue *const Queue)
{
    size_t writeIndex = Queue->writeIndex;
    size_t readIndex = Queue->readIndex;
    if (writeIndex < readIndex)
    {
        return writeIndex + (Queue->dataSize - readIndex);
    }
    return writeIndex - readIndex;
}

static inline size_t Queue_available_write(const Queue *const Queue)
{
    // Available write = Total - Written(readable)
    size_t available_write = (Queue->dataSize - 1) - Queue_available_read(Queue);
    return available_write;
}

static inline size_t Queue_write_internal(Queue *const Queue, const void *const data, size_t numberOfBytes, size_t writeIndex)
{
    if ((writeIndex + numberOfBytes) < Queue->dataSize)
    {
        memcpy((void *)&Queue->data[writeIndex], data, numberOfBytes);
        return writeIndex + numberOfBytes;
    }

    // before wrap
    size_t beforeWrap = Queue->dataSize - writeIndex;
    memcpy((void *)&Queue->data[writeIndex], data, beforeWrap);

    // After wrap
    size_t afterWrap = numberOfBytes - beforeWrap;
    memcpy(&Queue->data[0], (uint8_t *)data + beforeWrap, afterWrap);

    return afterWrap;
}

static inline Queue_err_t Queue_push(Queue *const Queue, const void *const data, size_t numberOfBytes)
{
    size_t available_bytes = Queue_available_write(Queue);
    if (available_bytes < numberOfBytes)
    {
        return Queue_Insufficient_Space;
    }

    size_t writeIndex = Queue->writeIndex;

    size_t newWriteIndex = Queue_write_internal(Queue, data, numberOfBytes, writeIndex);

    Queue->writeIndex = newWriteIndex;
    return Queue_Ok;
}

static inline size_t Queue_read_internal(Queue *const Queue, void *const outData, size_t numberOfBytes)
{
    size_t readIndex = Queue->readIndex;

    if (readIndex + numberOfBytes < Queue->dataSize)
    {
        memcpy(outData, (void *)&Queue->data[readIndex], numberOfBytes);
        return readIndex + numberOfBytes;
    }

    // before wrap
    size_t beforeWrap = Queue->dataSize - readIndex;
    memcpy(outData, (void *)&Queue->data[readIndex], beforeWrap);

    // After wrap
    size_t afterWrap = numberOfBytes - beforeWrap;
    memcpy((uint8_t *)outData + beforeWrap, &Queue->data[0], afterWrap);

    return afterWrap;
}

static inline Queue_err_t Queue_pop(Queue *const Queue, void *const outData, size_t numberOfBytes)
{
    size_t available_bytes = Queue_available_read(Queue);
    if (available_bytes < numberOfBytes)
    {
        return Queue_Insufficient_Data;
    }

    size_t newReadIndex = Queue_read_internal(Queue, outData, numberOfBytes);
    Queue->readIndex = newReadIndex;
    return Queue_Ok;
}

static inline Queue_err_t Queue_peek(Queue *const Queue, void *const outData, size_t numberOfBytes)
{
    size_t available_bytes = Queue_available_read(Queue);
    if (available_bytes < numberOfBytes)
    {
        return Queue_Insufficient_Data;
    }

    Queue_read_internal(Queue, outData, numberOfBytes);
    return Queue_Ok;
}

static inline Queue_err_t Queue_pop_all(Queue *const Queue, void *const outData)
{
    size_t available_bytes = Queue_available_read(Queue);
    size_t newReadIndex = Queue_read_internal(Queue, outData, available_bytes);
    return Queue_Ok;
}
#endif