#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef WIN32_MEAN_AND_LEAN
#undef VC_EXTRALEAN
#undef far
#undef near

#include "rin.h"

static void*
ReserveMemory(umm size, bool do_commit)
{
  void* memory = VirtualAlloc(0, size, MEM_RESERVE | (do_commit ? MEM_COMMIT : 0), PAGE_READWRITE);

  // TODO: Handle oom
  ASSERT(memory != 0);

  return memory;
}

static void
CommitMemory(void* base, umm size)
{
  void* result = VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE);

  // TODO: Handle oom
  ASSERT(result != 0);
}

static void
ReleaseMemory(void* reserve_base)
{
  VirtualFree(reserve_base, 0, MEM_RELEASE);
}

#include <stdio.h>

static bool
DEBUG__ParseAllFiles(Arena* file_arena, Arena* ast_arena, Arena* path_arena, u8* path, umm* files, umm* lines, umm* bytes)
{
  Arena_Marker marker = Arena_GetMarker(path_arena);
  u8* splice_point = (u8*)Arena_GetPointer(path_arena) - 1;

  Arena_Push(path_arena, sizeof("\\*")-1, _alignof(u8));
  Copy(splice_point, "\\*", sizeof("\\*"));

  WIN32_FIND_DATA find_data;
  HANDLE find_handle = FindFirstFileA(path, &find_data);

  Arena_PopToMarker(path_arena, marker);
  *((u8*)Arena_GetPointer(path_arena) - 1) = 0;

  bool succeeded = true;

  if (find_handle == INVALID_HANDLE_VALUE)
  {
    if (GetLastError() == ERROR_FILE_NOT_FOUND) succeeded = (succeeded && true);
    else                                        succeeded = false;
  }
  else
  {
    do
    {
      if (find_data.cFileName[0] != '.')
      {
        Arena_Marker marker = Arena_GetMarker(path_arena);
        u8* splice_point = (u8*)Arena_GetPointer(path_arena) - 1;

        u8* name     = find_data.cFileName;
        umm name_len = CString_Len(name);

        Arena_Push(path_arena, sizeof("\\")-1 + name_len, _alignof(u8));
        Copy(splice_point, "\\", sizeof("\\")-1);
        Copy(splice_point+1, name, name_len+1);

        if (find_data.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
        {
          u8* ext = name;
          for (u8* scan = name; *scan != 0; ++scan)
          {
            if (*scan == '.') ext = scan+1;
          }

          if (CString_Match(ext, "rin"))
          {
            HANDLE file_handle = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            if (file_handle == INVALID_HANDLE_VALUE) succeeded = false;
            else
            {
              LARGE_INTEGER file_size;
              if (!GetFileSizeEx(file_handle, &file_size) || file_size.QuadPart > U32_MAX) succeeded = false;
              else
              {
                u8* file_contents = Arena_Push(file_arena, file_size.QuadPart + 1, _alignof(u8));

                u32 bytes_read = 0;
                if (!ReadFile(file_handle, file_contents, (u32)file_size.QuadPart, &bytes_read, 0) || bytes_read != file_size.QuadPart) succeeded = false;
                else
                {
                  file_contents[file_size.QuadPart] = 0;

                  Ident_Table* ident_table = IdentTable_Create(1ULL << 32);

                  AST* ast = 0;
                  u64 debug_lines = 0;
                  if (!Parser_ParseFile(ast_arena, ident_table, file_contents, &ast, &debug_lines))
                  {
                    succeeded = false;
                    __debugbreak();
                  }
                  else
                  {
                    *files += 1;
                    *lines += debug_lines;
                    *bytes += file_size.QuadPart;
                  }

                  IdentTable_Destroy(&ident_table);
                }
              }

              CloseHandle(file_handle);
            }
          }
        }
        else if (find_data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
        {
          if (!DEBUG__ParseAllFiles(file_arena, ast_arena, path_arena, path, files, lines, bytes))
          {
            succeeded = false;
          }
        }

        Arena_PopToMarker(path_arena, marker);
        *((u8*)Arena_GetPointer(path_arena) - 1) = 0;
      }
    } while (succeeded && FindNextFileA(find_handle, &find_data));

    if (GetLastError() == ERROR_NO_MORE_FILES) succeeded = (succeeded && true);
    else                                       succeeded = false;
  }

  FindClose(find_handle);

  return succeeded;
}

static void
DEBUG_ParseAllFiles(u8* root_path)
{
  bool succeeded = false;
  umm lines = 0;
  umm files = 0;
  umm bytes = 0;

  LARGE_INTEGER start_ticks;
  QueryPerformanceCounter(&start_ticks);

  Arena* file_arena   = Arena_Create(1ULL << 32);
  Arena* ast_arena    = Arena_Create(1ULL << 34);
  Arena* path_arena   = Arena_Create(1ULL << 34);
  
  umm path_len = CString_Len(root_path);
  u8* path = Arena_Push(path_arena, path_len + 1, _alignof(u8));
  Copy(path, root_path, path_len + 1);

  if (path[path_len-1] != '\\' && path[path_len-1] != '/' && DEBUG__ParseAllFiles(file_arena, ast_arena, path_arena, path, &files, &lines, &bytes))
  {
    succeeded = true;
  }

  LARGE_INTEGER end_ticks;
  QueryPerformanceCounter(&end_ticks);

  if (!succeeded) fprintf(stderr, "Failed\n");
  else
  {
    LARGE_INTEGER perf_freq;
    QueryPerformanceFrequency(&perf_freq);

    u64 elapsed_ticks = end_ticks.QuadPart - start_ticks.QuadPart;
    f64 elapsed_ms    = 1000 * (f64)elapsed_ticks/perf_freq.QuadPart;

    printf("Succeeded\n");
    printf("parsed: %llu files, %llu lines\n", files, lines);
    printf("time: %f ms\n", elapsed_ms);
    printf("speed: %.3f Kloc/s, %.3f MB/s\n", lines/elapsed_ms, bytes/(elapsed_ms*((f64)(1U << 20)/1000)));
  }
}

int
main(int argc, char** argv)
{
  if (argc == 3 && CString_Match(argv[1], "parse_all")) DEBUG_ParseAllFiles(argv[2]);

  return 0;
}
