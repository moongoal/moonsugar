/**
 * @file
 *
 * File system access interface.
 */
#ifndef MS_FILE_H
#define MS_FILE_H

#include <moonsugar/api.h>

typedef struct ms_file ms_file;
typedef uint32_t ms_fmode_flags;

typedef enum {
  /**
   * Open the file for reading.
   */
  MS_FMODE_READ_BIT = 1,

  /**
   * Open the file for writing.
   */
  MS_FMODE_WRITE_BIT = 2,

  /**
   * Open the file for reading and writing.
   */
  MS_FMODE_READ_WRITE_BIT = MS_FMODE_READ_BIT | MS_FMODE_WRITE_BIT,

  /**
   * Open the file asynchronously.
   */
  MS_FMODE_ASYNC_BIT = 4,

  /**
   * The file will be accessed randomly.
   */
  MS_FMODE_RANDOM_BIT = 8,

  /**
   * Enable direct IO, disabling system caches.
   *
   * This flag is useful when streaming data,
   * no data is read twice from the file, but
   * requires all read and write buffers to
   * be aligned to the logical sector size
   * of the disk.
   *
   * Typically a buffer can be aligned to 4096
   * bytes without incurring in issues.
   */
  MS_FMODE_DIRECT_BIT = 16,

  /**
   * The file will be created if it doesn't exist.
   */
  MS_FMODE_CREATE_BIT = 32
} ms_fmode_flag_bits;

typedef enum {
  /**
   * The file position is absolute.
   */
  MS_FSEEK_ORIGIN,

  /**
   * The file position is relative to the end of the file.
   */
  MS_FSEEK_END,

  /**
   * The file position is relative to the current position.
   */
  MS_FSEEK_CUR
} ms_fseek_mode;

/**
 * Open a file.
 *
 * @param path The file path.
 * @param flags The file operation mode flags.
 * @param out_file Pointer to the output file pointer.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_RESULT_LIMIT if too many files are open
 */
MSUSERET ms_result MSAPI ms_fopen(char const * const path, ms_fmode_flags const flags, ms_file ** const out_file);

/**
 * Close an open file.
 *
 * @param f The file to close.
 */
void MSAPI ms_fclose(ms_file * const f);

/**
 * Read from a file.
 *
 * @param f The file.
 * @param size The number of bytes to read.
 * @param out The output buffer, with at least storage for `size` bytes.
 *
 * @return The result of the operation. The operation can succeed synchronously
 *  even when it's been asynchronously executed, due to caching mechanisms by
 *  the operating system.
 */
ms_result MSAPI ms_fread(ms_file * const f, uint32_t size, void * const out);

/**
 * Write to a file.
 *
 * @param f The file.
 * @param size The number of bytes to write.
 * @param buff The input buffer, with at least storage for `size` bytes.
 *
 * @return The result of the operation. The operation can succeed synchronously
 *  even when it's been asynchronously executed, due to caching mechanisms by
 *  the operating system.
 */
ms_result MSAPI ms_fwrite(ms_file * const f, uint32_t size, void const * const buff);

/**
 * Get the state of an asynchronous operation.
 *
 * @param f The file executing an asynchronous operation.
 *
 * @return
 *  - MS_RESULT_SUCCESS The operation has completed.
 *  - MS_RESULT_UNSUPPORTED The file was not opened for asynchronous operation.
 *  - MS_RESULT_SCHEDULED The operation is in progress.
 *  - Any other error code on error
 */
ms_result MSAPI ms_get_async_state(ms_file * const f);

/**
 * Get the number of bytes read or written by the last IO operation.
 * Only call this function immediately after verifying successful complation
 * of an IO operation, and only once per operation.
 *
 * @param f The file where the operation was submitted.
 *
 * @return The number of bytes read or written. The value returned by this function
 *  may be invalid if an IO operation is currently being executed on the input file,
 *  or has failed.
 */
uint32_t MSAPI ms_get_last_io_size(ms_file * const f);

/**
 * Set the current position in the file.
 *
 * @param f The file.
 * @param pos The position.
 * @param mode The mode of operation.
 *
 * @return The result of the operation.
 */
ms_result MSAPI ms_fseek(ms_file * const f, int64_t const pos, ms_fseek_mode const mode);

/**
 * Get the total size of a file, in bytes.
 *
 * @param f The file.
 * @param out_size The output file size.
 *
 * @return The result of the operation.
 */
ms_result MSAPI ms_get_size(ms_file * const f, uint64_t * const out_size);

/**
 * Get the size, in bytes, of the secondary storage block size
 * for the given file.
 *
 * @param f The file handle.
 * @param out_block_size Pointer to the memory that will receive the block size.
 *
 * @return The result of the operation.
 */
ms_result MSAPI ms_get_block_size(ms_file * const f, uint32_t * const out_block_size);

/**
 * Get the current file cursor position.
 *
 * @param f The file handle.
 * @param out_position Pointer to the memory that will receive the cursor position.
 *
 * @return The result of the operation.
 */
ms_result MSAPI ms_get_position(ms_file * const f, uint64_t * const out_position);

#ifdef MSAPI
  void ms_file_initialize(void);

  ms_result ms_file_backend_open(
    char const * const path,
    ms_file * const file,
    ms_fmode_flags const flags
  );

  void ms_file_backend_close(ms_file * const f);

  #ifdef _WIN32
    #include <Windows.h>

    struct ms_file {
      HANDLE hfile;
      OVERLAPPED overlapped;
    
      ms_fmode_flags flags;
    
      /**
       * Number of bytes read or written by the last IO operation.
       */
      DWORD last_bytes_rw;
    };
  #else
    #include <aio.h>

    struct ms_file {
      ssize_t last_io_bytes;
      int hfile;
      struct aiocb cb;
      ms_fmode_flags flags;
    };
  #endif // _WIN32
#endif // MSAPI

#endif // MS_FILE_H
