/**
 * @file
 *
 * Compression interface.
 */
#ifndef MS_COMPRESS_H
#define MS_COMPRESS_H

#include <moonsugar/api.h>
#include <moonsugar/memory.h>

typedef enum {
  MS_COMPRESS_ALGORITHM_DEFLATE,
  MS_COMPRESS_ALGORITHM_ZLIB,
} ms_compress_algorithm;

typedef enum {
  MS_COMPRESS_LEVEL_FASTEST,
  MS_COMPRESS_LEVEL_BALANCED,
  MS_COMPRESS_LEVEL_BEST
} ms_compress_level;

typedef struct {
  /**
   * Memory allocator.
   */
  ms_allocator const *allocator;

  /**
   * Compression algorithm.
   */
  ms_compress_algorithm algorithm;

  /**
   * Compression level.
   */
  ms_compress_level level;

  /**
   * Input size, in bytes.
   */
  size_t input_sz;

  /**
   * Input data.
   */
  void const * input;

  /**
   * Output buffer size.
   */
  size_t output_sz;

  /**
   * Output buffer.
   */
  void * output;

  /**
   * Pointer to the memory that will receive the amount
   * of compressed bytes.
   */
   size_t *out_compressed_size;
} ms_compress_description;

typedef struct {
  /**
   * Memory allocator.
   */
  ms_allocator const *allocator;

  /**
   * Compression algorithm.
   */
  ms_compress_algorithm algorithm;

  /**
   * Compressed data size, in bytes.
   */
  size_t input_sz;

  /**
   * Compressed data.
   */
  void const * input;

  /**
   * Output buffer size, in bytes.
   */
  size_t output_sz;

  /**
   * Output buffer.
   */
  void * output;

  /**
   * Pointer to the memory that will receive the amount
   * of decompressed bytes.
   */
  size_t *out_decompressed_size;
} ms_decompress_description;

/**
 * Compress data in one go.
 *
 * @param description The compression operation description.
 *
 * @return
 *  MS_RESULT_SUCCESS on success.
 *  MS_RESULT_LENGTH if the output buffer is too small.
 *  MS_RESULT_UNKNOWN if compression fails for another reason.
 */
ms_result MSAPI ms_compress(ms_compress_description const * const description);

/**
 * Decompress data in one go.
 *
 * @param description The decompression operation description.
 *
 * @return
 *  MS_RESULT_SUCCESS on success.
 *  MS_RESULT_LENGTH if the output buffer is too small.
 *  MS_RESULT_UNKNOWN if decompression fails for another reason.
 */
ms_result MSAPI ms_decompress(ms_decompress_description const * const description);

#endif // MS_COMPRESS_H
