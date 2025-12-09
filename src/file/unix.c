#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <moonsugar/util.h>
#include <moonsugar/file.h>

static MSUSERET ms_result convert_error(int error) {
  switch(error) {
    case EACCES:
    case EPERM:
      return MS_RESULT_ACCESS;

    case EINVAL:
      return MS_RESULT_INVALID_ARGUMENT;

    case EMFILE:
    case ENFILE:
    case EAGAIN:
      return MS_RESULT_RESOURCE_LIMIT;

    case ENOENT:
    case ENODEV:
      return MS_RESULT_NOT_FOUND;

    default:
      return MS_RESULT_UNKNOWN;
  }
}

static MSUSERET ms_result convert_errno() {
  return convert_error(errno);
}

ms_result ms_backend_open(
  char const * const path,
  ms_file * const file,
  ms_fmode_flags const flags
) {
  int fflags = 0;
  int fmode = 0;

  fflags |= (int)ms_choose(O_RDWR, 0, ms_test(flags, MS_FMODE_READ_WRITE_BIT));

  if(fflags == 0) {
    if(ms_test(flags, MS_FMODE_READ_BIT)) {
      fflags |= O_RDONLY;
    } else if(ms_test(flags, MS_FMODE_WRITE_BIT)) {
      fflags |= O_WRONLY;
    } else {
      return MS_RESULT_INVALID_ARGUMENT;
    }
  }

  fflags |= (int)ms_choose(O_ASYNC, 0, ms_test(flags, MS_FMODE_ASYNC_BIT));
  
  file->hfile = open(path, fflags, fmode);

  if(file->hfile != -1) {
    file->flags = flags;

    if(ms_test(flags, MS_FMODE_ASYNC_BIT)) {
      fcntl(file->hfile, F_SETFL, O_ASYNC);
    }

    if(ms_test(flags, MS_FMODE_DIRECT_BIT)) {
      #ifdef __APPLE__
        fcntl(file->hfile, F_NOCACHE, 1);
      #else
        fcntl(file->hfile, F_SETFL, O_DIRECT);
      #endif
    }

    file->cb = (struct aiocb) {
      file->hfile,
      0,
      NULL,
      0,
      0,
      {
        SIGEV_NONE,
        0,
        {0},
        NULL,
        NULL
        #ifdef __linux__
          , 0
        #endif
      },
      0
    };

    return MS_RESULT_SUCCESS;
  }

  return convert_errno();
}

void ms_backend_close(ms_file * const f) {
  close(f->hfile);
  f->hfile = -1;
}

ms_result ms_backend_read(ms_file * const f, uint32_t size, void * const out) {
  if(ms_test(f->flags, MS_FMODE_ASYNC_BIT)) {
    f->cb.aio_buf = out;
    f->cb.aio_nbytes = size;

    int const result = aio_read(&f->cb);

    if(result == 0) {
      return MS_RESULT_SUCCESS;
    }
  } else {
    f->last_io_bytes = read(f->hfile, out, size);

    if(f->last_io_bytes >= 0) {
      return MS_RESULT_SUCCESS;
    }
  }

  return convert_errno();
}

ms_result ms_backend_write(ms_file * const f, uint32_t size, void const * const buff) {
  if(ms_test(f->flags, MS_FMODE_ASYNC_BIT)) {
    f->cb.aio_buf = (void*)buff;
    f->cb.aio_nbytes = size;

    int const result = aio_write(&f->cb);

    if(result == 0) {
      return MS_RESULT_SUCCESS;
    }
  } else {
    f->last_io_bytes = write(f->hfile, buff, size);

    if(f->last_io_bytes >= 0) {
      return MS_RESULT_SUCCESS;
    }
  }

  return convert_errno();
}

ms_result ms_backend_get_async_state(ms_file * const f) {
  int const result = aio_error(&f->cb);

  switch(result) {
    case 0:
      return MS_RESULT_SUCCESS;

    case EINPROGRESS:
      return MS_RESULT_SCHEDULED;

    default:
      return convert_error(result);
  }
}

uint32_t ms_backend_get_last_io_size(ms_file * const f) {
  int const result = aio_return(&f->cb);

  f->cb.aio_offset += result;

  return (uint32_t)result;
}

ms_result ms_backend_seek(ms_file * const f, int64_t const pos, ms_fseek_mode const mode) {
  if(ms_test(f->flags, MS_FMODE_ASYNC_BIT)) {
    switch(mode) {
      case MS_FSEEK_ORIGIN: {
        f->cb.aio_offset = pos;
      } break;

      case MS_FSEEK_END: {
        uint64_t size;
        ms_result const result = ms_backend_get_size(f, &size);

        if(result != MS_RESULT_SUCCESS) {
          return result;
        }

        f->cb.aio_offset = size + pos;
      } break;

      case MS_FSEEK_CUR: {
        f->cb.aio_offset += pos;
      } break;
    }

    return MS_RESULT_SUCCESS;
  } else {
    int whence[] = {
      [MS_FSEEK_CUR] = SEEK_CUR,
      [MS_FSEEK_END] = SEEK_END,
      [MS_FSEEK_ORIGIN] = SEEK_SET
    };

    off_t const result = lseek(f->hfile, pos, whence[mode]);

    if(result != (off_t)-1) {
      return MS_RESULT_SUCCESS;
    }
  }

  return convert_errno();
}

ms_result ms_backend_get_size(ms_file * const f, uint64_t * const out_size) {
  struct stat s;
  int const result = fstat(f->hfile, &s);

  if(result == 0) {
    *out_size = s.st_size;
    return MS_RESULT_SUCCESS;
  }

  return convert_errno();
}


