#include <moonsugar/util.h>
#include <moonsugar/file.h>

static void setup_overlapped(ms_file * const f) {
  f->overlapped.Internal = 0;
  f->overlapped.InternalHigh = 0;
  f->overlapped.Pointer = 0;
}

static ms_result convert_error(DWORD const error) {
  switch(error) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
      return MS_RESULT_NOT_FOUND;

    case ERROR_ACCESS_DENIED:
      return MS_RESULT_ACCESS;

    case ERROR_NOT_ENOUGH_MEMORY:
      return MS_RESULT_MEMORY;

    case ERROR_INSUFFICIENT_BUFFER:
      return MS_RESULT_LENGTH;

    case ERROR_HANDLE_EOF:
      return MS_RESULT_EOF;

    default:
      return MS_RESULT_UNKNOWN;
  }
}

ms_result ms_file_backend_open(
  char const * const path,
  ms_file * const file,
  ms_fmode_flags const flags
) {
  DWORD desired_access = 0;
  DWORD share_mode = 0;
  DWORD attributes_and_flags = FILE_ATTRIBUTE_NORMAL;

  desired_access |= ms_choose(GENERIC_READ, 0, ms_test(flags, MS_FMODE_READ_BIT));
  desired_access |= ms_choose(GENERIC_WRITE, 0, ms_test(flags, MS_FMODE_WRITE_BIT));

  if(desired_access == 0) { 
    return MS_RESULT_INVALID_ARGUMENT;
  }

  share_mode |= ms_choose(FILE_SHARE_READ, 0, !ms_test(flags, MS_FMODE_WRITE_BIT));

  attributes_and_flags |= ms_choose(FILE_FLAG_OVERLAPPED, 0, ms_test(flags, MS_FMODE_ASYNC_BIT));
  attributes_and_flags |= ms_choose(FILE_FLAG_RANDOM_ACCESS, 0, ms_test(flags, MS_FMODE_RANDOM_BIT));
  attributes_and_flags |= ms_choose(FILE_FLAG_NO_BUFFERING, 0, ms_test(flags, MS_FMODE_DIRECT_BIT));

  DWORD const disposition = ms_choose(CREATE_ALWAYS, OPEN_EXISTING, ms_test(flags, MS_FMODE_CREATE_BIT));

  file->hfile = CreateFileA(
    path,
    desired_access,
    share_mode,
    NULL,
    disposition,
    attributes_and_flags,
    NULL
  );

  if(file->hfile != INVALID_HANDLE_VALUE) {
    return MS_RESULT_SUCCESS;
  }

  file->flags = flags;

  memset(&file->overlapped, 0, sizeof(OVERLAPPED));

  return convert_error(GetLastError());
}

void ms_file_backend_close(ms_file * const f) {
  CloseHandle(f->hfile);
  f->hfile = NULL;
}

ms_result ms_fread(ms_file * const f, uint32_t size, void * const out) {
  if(ms_test(f->flags, MS_FMODE_ASYNC_BIT)) {
    setup_overlapped(f);
  }

  BOOL const result = ReadFile(
    f->hfile,
    out,
    size,
    ms_test(f->flags, MS_FMODE_ASYNC_BIT) ? NULL : &f->last_bytes_rw,
    &f->overlapped
  );

  if(result) {
    return MS_RESULT_SUCCESS;
  }

  DWORD const read_error = GetLastError();

  if(read_error == ERROR_IO_PENDING) {
    return MS_RESULT_SCHEDULED;
  }

  return convert_error(read_error);
}

ms_result ms_get_async_state(ms_file * const f) {
  if(!ms_test(f->flags, MS_FMODE_ASYNC_BIT)) {
    return MS_RESULT_UNSUPPORTED;
  }

  BOOL const result = GetOverlappedResult(
    f->hfile,
    &f->overlapped,
    &f->last_bytes_rw,
    FALSE
  );

  if(result) {
    return MS_RESULT_SUCCESS;
  }

  DWORD const gor_error = GetLastError();

  if(gor_error == ERROR_IO_INCOMPLETE) {
    return MS_RESULT_SCHEDULED;
  }

  return convert_error(gor_error);
}

uint32_t ms_get_last_io_size(ms_file * const f) {
  return f->last_bytes_rw;
}

ms_result ms_fseek(ms_file * const f, int64_t const pos, ms_fseek_mode const mode) {
  if(ms_test(f->flags, MS_FMODE_ASYNC_BIT)) {
    switch(mode) {
      case MS_FSEEK_ORIGIN: {
        f->overlapped.Offset = pos & UINT32_MAX;
        f->overlapped.OffsetHigh = pos >> 32;
      } break;

      case MS_FSEEK_CUR: {
        uint64_t const new_pos = (
          pos
          + (uint64_t)f->overlapped.Offset
          + ((uint64_t)f->overlapped.OffsetHigh << 32)
        );

        f->overlapped.Offset = new_pos & UINT32_MAX;
        f->overlapped.OffsetHigh = new_pos >> 32;
      } break;

      case MS_FSEEK_END: {
        LARGE_INTEGER size;

        if(!GetFileSizeEx(f->hfile, &size)) {
          return convert_error(GetLastError());
        }

        uint64_t const new_pos = (
          size.QuadPart
          + (uint64_t)f->overlapped.Offset
          + ((uint64_t)f->overlapped.OffsetHigh << 32)
        );

        f->overlapped.Offset = new_pos & UINT32_MAX;
        f->overlapped.OffsetHigh = new_pos >> 32;
      } break;
    }
  } else {
    DWORD const mode_map[] = {
      [MS_FSEEK_ORIGIN] = FILE_BEGIN,
      [MS_FSEEK_END] = FILE_END,
      [MS_FSEEK_CUR] = FILE_CURRENT
    };

    LONG high = pos >> 32;
    DWORD const result = SetFilePointer(f->hfile, pos & UINT32_MAX, &high, mode_map[mode]);

    if(result == INVALID_SET_FILE_POINTER) {
      DWORD const sfp_error = GetLastError();

      return convert_error(sfp_error);
    }
  }

  return MS_RESULT_SUCCESS;
}

ms_result ms_fwrite(ms_file * const f, uint32_t size, void const * const buff) {
  if(ms_test(f->flags, MS_FMODE_ASYNC_BIT)) {
    setup_overlapped(f);
  }

  BOOL const result = WriteFile(
    f->hfile,
    buff,
    size,
    ms_test(f->flags, MS_FMODE_ASYNC_BIT) ? NULL : &f->last_bytes_rw,
    &f->overlapped
  );

  if(result) {
    return MS_RESULT_SUCCESS;
  }

  DWORD const read_error = GetLastError();

  if(read_error == ERROR_IO_PENDING) {
    return MS_RESULT_SCHEDULED;
  }

  return convert_error(read_error);
}

ms_result ms_get_size(ms_file * const f, uint64_t * const out_size) {
  LARGE_INTEGER size;

  BOOL const result = GetFileSizeEx(f->hfile, &size);

  if(result == TRUE) {
    *out_size = size.LowPart | ((uint64_t)size.HighPart << 32);

    return MS_RESULT_SUCCESS;
  }

  return convert_error(GetLastError());
}

ms_result ms_get_block_size(ms_file * const f, uint32_t * const out_block_size) {
  FILE_STORAGE_INFO sinfo;

  if(GetFileInformationByHandleEx(f->hfile, FileStorageInfo, &sinfo, sizeof(sinfo))) {
    *out_block_size = sinfo.PhysicalBytesPerSectorForPerformance;

    return MS_RESULT_SUCCESS;
  }

  return MS_RESULT_UNKNOWN;
}

ms_result ms_get_position(ms_file * const f, uint64_t * const out_position) {
  if(ms_test(f->flags, MS_FMODE_ASYNC_BIT)) {
    *out_position = (uint64_t)f->overlapped.Offset + ((uint64_t)f->overlapped.OffsetHigh << 32);

    return MS_RESULT_SUCCESS;
  }

  LARGE_INTEGER set_pos = { .QuadPart = 0 };
  LARGE_INTEGER new_pos;
  BOOL const result = SetFilePointerEx(f->hfile, set_pos, &new_pos, FILE_CURRENT);
  
  if(result != 0) {
    *out_position = new_pos.QuadPart;

    return MS_RESULT_SUCCESS;
  }

  return MS_RESULT_UNKNOWN;
}

