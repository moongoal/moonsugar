#include <string.h>
#include <zlib.h>
#include <moonsugar/compress.h>

#define THROW(err) do { result = err; goto on_error; } while(0)

static voidpf zlib_alloc(voidpf opaque, uInt items, uInt size) {
  return ms_malloc(opaque, (size_t)items * size, MS_DEFAULT_ALIGNMENT);
}

static void zlib_free(voidpf opaque, voidpf address) {
  ms_free(opaque, address);
}

static int get_window_bits(ms_compress_algorithm const algo) {
  switch(algo) {
    case MS_COMPRESS_ALGORITHM_DEFLATE: {
      return -15;
    } break;

    case MS_COMPRESS_ALGORITHM_ZLIB: {
      return 15;
    } break;
  }
}

static ms_result init_compression_zstream(z_stream * const s, ms_compress_description const * const description) {
  int compression_level;
  int window_bits = get_window_bits(description->algorithm);

  memset(s, 0, sizeof(z_stream));

  s->next_in = (void*)description->input;
  s->avail_in = description->input_sz;

  s->next_out = description->output;
  s->avail_out = description->output_sz;

  s->zalloc = zlib_alloc;
  s->zfree = zlib_free;
  s->opaque = (voidpf)description->allocator;

  switch(description->level) {
    case MS_COMPRESS_LEVEL_FASTEST: {
      compression_level = 1;
    } break;

    case MS_COMPRESS_LEVEL_BALANCED: {
      compression_level = Z_DEFAULT_COMPRESSION;
    } break;

    case MS_COMPRESS_LEVEL_BEST: {
      compression_level = 9;
    } break;
  }

  if(
    deflateInit2(
      s,
      compression_level,
      Z_DEFLATED,
      window_bits,
      8,
      Z_DEFAULT_STRATEGY
    ) != Z_OK
  ) {
    return MS_RESULT_UNKNOWN;
  }

  return MS_RESULT_SUCCESS;
}

static ms_result init_decompression_zstream(z_stream * const s, ms_decompress_description const * const description) {
  int window_bits = get_window_bits(description->algorithm);

  memset(s, 0, sizeof(z_stream));

  s->next_in = (void*)description->input;
  s->avail_in = description->input_sz;

  s->next_out = description->output;
  s->avail_out = description->output_sz;

  s->zalloc = zlib_alloc;
  s->zfree = zlib_free;
  s->opaque = (voidpf)description->allocator;

  if(inflateInit2(s, window_bits) != Z_OK) {
    return MS_RESULT_UNKNOWN;
  }

  return MS_RESULT_SUCCESS;
}

ms_result ms_compress(ms_compress_description const * const description) {
  z_stream s;
  ms_result init_result;
  ms_result result = MS_RESULT_SUCCESS;
  int cres;

  if((init_result = init_compression_zstream(&s, description)) != MS_RESULT_SUCCESS) {
    THROW(init_result);
  }

  if((cres = deflate(&s, Z_FINISH)) != Z_STREAM_END) {
    THROW(
      (cres == Z_OK || cres == Z_BUF_ERROR)
      ? MS_RESULT_LENGTH
      : MS_RESULT_INVALID_ARGUMENT
    );
  }

  if(s.avail_in != 0) { // Not all input processed
    THROW(MS_RESULT_LENGTH);
  }

  *description->out_compressed_size = s.total_out;

on_error:
  if(init_result == MS_RESULT_SUCCESS) {
    inflateEnd(&s);
  }

  return result;
}

ms_result ms_decompress(ms_decompress_description const * const description) {
  z_stream s;
  ms_result init_result;
  ms_result result = MS_RESULT_SUCCESS;
  int cres;

  if((init_result = init_decompression_zstream(&s, description)) != MS_RESULT_SUCCESS) {
    THROW(init_result);
  }

  if((cres = inflate(&s, Z_FINISH)) != Z_STREAM_END) {
    THROW(
      (cres == Z_OK || cres == Z_BUF_ERROR)
      ? MS_RESULT_LENGTH
      : MS_RESULT_INVALID_ARGUMENT
    );
  }

  if(s.avail_in != 0) { // Not all input processed
    THROW(MS_RESULT_LENGTH);
  }

  *description->out_decompressed_size = s.total_out;

on_error:
  if(init_result == MS_RESULT_SUCCESS) {
    inflateEnd(&s);
  }

  return result;
}
