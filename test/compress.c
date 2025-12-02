#include <stdlib.h>
#include <moondance/test.h>
#include <moonsugar/compress.h>

static void* allocate(void* p, size_t count, size_t alignment) {
  ((void)p);
  ((void)alignment);

  return malloc(count);
}

static void deallocate(void* p, void* ptr) {
  ((void)p);

  free(ptr);
}

static void* reallocate(void* p, void * ptr, size_t new_count) {
  ((void)p);

  return realloc(ptr, new_count);
}

static ms_allocator g_allocator = { allocate, deallocate, reallocate, NULL };

MD_CASE(compress_decompress__cycle) {
  char input[] = "some random text to compress";

  char compressed_data[1024];
  size_t compressed_sz;

  char decompressed_data[1024] = "hwosfhsihfoisdhfiohf0348hfwuoefw4ri2340-t0we4t0erhgr0ehgiu4h2ugbrjkbvneid";
  size_t decompressed_sz;

  ms_compress_description const cdesc = {
    &g_allocator,
    MS_COMPRESS_ALGORITHM_DEFLATE,
    MS_COMPRESS_LEVEL_BALANCED,
    sizeof(input),
    input,
    sizeof(compressed_data),
    compressed_data,
    &compressed_sz
  };

  ms_result const cresult = ms_compress(&cdesc);
  md_assert(cresult == MS_RESULT_SUCCESS);

  ms_decompress_description const ddesc = {
    &g_allocator,
    MS_COMPRESS_ALGORITHM_DEFLATE,
    compressed_sz,
    compressed_data,
    sizeof(decompressed_data),
    decompressed_data,
    &decompressed_sz
  };

  ms_result const dresult = ms_decompress(&ddesc);
  md_assert(dresult == MS_RESULT_SUCCESS);

  md_assert(strcmp(input, decompressed_data) == 0);
}

MD_CASE(compress__buffer_too_short) {
  char input[] = "some random text to compress";

  char compressed_data[1];
  size_t compressed_sz;

  ms_compress_description const cdesc = {
    &g_allocator,
    MS_COMPRESS_ALGORITHM_DEFLATE,
    MS_COMPRESS_LEVEL_BALANCED,
    sizeof(input),
    input,
    sizeof(compressed_data),
    compressed_data,
    &compressed_sz
  };

  ms_result const cresult = ms_compress(&cdesc);
  md_assert(cresult == MS_RESULT_LENGTH);
}

MD_CASE(decompress__buffer_too_short) {
  char input[] = "some random text to compress";

  char compressed_data[1024];
  size_t compressed_sz;

  char decompressed_data[1];
  size_t decompressed_sz;

  ms_compress_description const cdesc = {
    &g_allocator,
    MS_COMPRESS_ALGORITHM_DEFLATE,
    MS_COMPRESS_LEVEL_BALANCED,
    sizeof(input),
    input,
    sizeof(compressed_data),
    compressed_data,
    &compressed_sz
  };

  ms_result const cresult = ms_compress(&cdesc);
  md_assert(cresult == MS_RESULT_SUCCESS);

  ms_decompress_description const ddesc = {
    &g_allocator,
    MS_COMPRESS_ALGORITHM_DEFLATE,
    compressed_sz,
    compressed_data,
    sizeof(decompressed_data),
    decompressed_data,
    &decompressed_sz
  };

  ms_result const dresult = ms_decompress(&ddesc);
  md_assert(dresult == MS_RESULT_LENGTH);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  md_add(&suite, compress_decompress__cycle);
  md_add(&suite, compress__buffer_too_short);
  md_add(&suite, decompress__buffer_too_short);

  return md_run(argc, argv, &suite);
}
