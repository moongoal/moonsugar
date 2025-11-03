#include <string.h>
#include <moonsugar/path.h>

ms_result ms_path_get_filename(char const * const s, char * const buff, unsigned buff_len) {
  char * const nulptr = &buff[buff_len - 1];
  unsigned filename_offset = 0;

  for(char const *p = s; *p != '\0'; ++p) {
    if(*p == MS_PATH_SEP) {
      filename_offset = p - s + 1;
    }
  }

  strncpy(buff, &s[filename_offset], buff_len);

  if(*nulptr != '\0') {
    return MS_RESULT_LENGTH;
  }

  return MS_RESULT_SUCCESS;
}

ms_result ms_path_get_parent(char const * const s, char * const buff, unsigned buff_len) {
  unsigned sep_offset = 0;
  unsigned offset;

  for(offset = 0; s[offset] != '\0'; ++offset) {
    if(s[offset] == MS_PATH_SEP) {
      sep_offset = offset;
    }

    if(offset == buff_len) {
      break;
    }

    buff[offset] = s[offset];
  }

  if(offset == buff_len) {
    return MS_RESULT_LENGTH;
  }

  // Relative path has no more available parents
  if(offset == 0) {
    buff[offset] = '\0';

    return MS_RESULT_SUCCESS;
  }

  // Windows style path and drive present
  if(sep_offset == 0 && buff_len > 1 && buff[1] == MS_PATH_WIN32_DRIVE_SEP) {
    if(buff_len < 3) {
      return MS_RESULT_LENGTH;
    }

    buff[2] = '\0';

    return MS_RESULT_SUCCESS;
  }

  buff[sep_offset] = '\0';

  return MS_RESULT_SUCCESS;
}

void ms_path_convert_slashes(char const * s, char * buff) {
  for(char c = *s; c != '\0'; ++s, ++buff, c = *s) {
    if(c == MS_PATH_WIN32_SEP) {
      c = MS_PATH_SEP;
    }

    *buff = c;
  }

  *buff = '\0';
}

ms_result ms_path_join(char const * const s1, char const * s2, char * const buff, unsigned const buff_len) {
  unsigned const s1_len = strlen(s1);
  unsigned s2_len = strlen(s2);
  unsigned buff_offset = 0;

  if(buff_len < s1_len + 1) {
    return MS_RESULT_LENGTH;
  }

  memcpy(buff, s1, sizeof(char) * s1_len);
  buff_offset = s1_len;

  if(buff_offset > 0 && buff[buff_offset - 1] != MS_PATH_SEP && s2_len > 0) {
    if(buff_offset + 1 == buff_len) {
      return MS_RESULT_LENGTH;
    }

    buff[buff_offset++] = MS_PATH_SEP;
  }

  char * const joint = buff + buff_offset;
  unsigned const joint_len = buff_len - buff_offset;

  if(*s2 == MS_PATH_SEP) {
    s2++;
    s2_len--;
  }
  
  if(joint_len < s2_len + 1) {
    return MS_RESULT_LENGTH;
  }

  memcpy(joint, s2, sizeof(char) * (s2_len + 1));

  return MS_RESULT_SUCCESS;
}
